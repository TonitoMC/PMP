#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <pthread.h>
#include <atomic>
#include <unistd.h>
#include <semaphore.h>

// Constantes de juego / variables globales
const int SCREEN_WIDTH = 800;   // Ancho de la pantalla
const int SCREEN_HEIGHT = 600;  // ALtura de la pantalla
const int WALL_THICKNESS = 10;  // Ancho de las paredes
const int PADDLE_HEIGHT = 100;  // Altura de las paletas
const int BALL_SIZE = 15;       // Tamano de la pelota
const int BALL_SPEED = 4;       // Velocidad de la pelota
const int PADDLE_WIDTH = 20;    // Ancho de paleta
int paddleSpeed = 5;            // Velocidad de paleta


// Enumeración para diferentes estados de juego
enum GameState {
    MAIN_MENU,      // Menu Principal
    SINGLE_PLAYER,  // Jugador vs Computadora
    TWO_PLAYER,     // Jugador vs Jugador
    SIMULATION,     // Computadora vs Computadora
    GAME_OVER,      // Juego terminado
    SETTINGS,       // Configuracion
    QUIT            // Salir
};

// Variable global para almacenar el estado de juego
std::atomic<GameState> currentState(MAIN_MENU);

// Variables globales de recursos de SDL
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* mainMenuTexture = nullptr;

// Rectángulos para botones del menú
SDL_Rect singlePlayerButton = { 175, 210, 450, 75 };
SDL_Rect twoPlayerButton = { 125, 310, 560, 75 };
SDL_Rect simulationButton = { 125, 400, 560, 75 };
SDL_Rect settingsButton = { 660, 45, 95, 95 };

// Variables para sincronización de hilos
pthread_mutex_t gameMutex;          // Mutex
pthread_cond_t paddleUpdateCond;    // Variable condicional para actualizar paletas
pthread_cond_t renderCond;          // Variable condicional para avisar al renderer
sem_t ballUpdateSem;                // Semaforo para actualizar pelota

// Estructura para representar una paleta
struct Paddle {
    SDL_Rect rect;      // El rectangulo para dibujar en SDL
    bool isMovingUp;    // Representa el estado de la paleta en un momento dado (cuando se lee el estado del teclado)
    bool isMovingDown;  // Representa el estado de la paleta en un  momento dado (cuando se lee el estado del teclado)
};

// Estructura para representar la pelota
struct Ball {
    SDL_Rect rect;      // El rectangulo para dibujar en SDL
    int velX, velY;     // Representa la velocidad en cada coordenada de la pelota
};

// Estructura para almacenar informacion sobre la pelota, dentro del hilo que controla la pelota se modifican puntajes
struct BallData {
    Ball* ball;         // La pelota que se mueve
    int* player1Score;  // Puntaje del jugador 1
    int* player2Score;  // Puntaje del jugador 2
};

// Pelota global, se necesita acceder desde diferentes hilos
Ball global_ball = {{SCREEN_WIDTH / 2 - BALL_SIZE / 2, SCREEN_HEIGHT / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE}, BALL_SPEED, BALL_SPEED};


// Variable para controlar la finalización del juego
bool quit = false;

// Paleta Izquierda
SDL_Rect leftPaddleRect = { 50, SCREEN_HEIGHT / 2 - 50, 20, 100 }; // Rectangulo de SDL
Paddle leftPaddle = {leftPaddleRect, false, false}; // Estructura para almacenar la paleta y su estado de movimiento

// Paleta Derecha
SDL_Rect rightPaddleRect = { SCREEN_WIDTH - 70, SCREEN_HEIGHT / 2 - 50, 20, 100 };  // Rectangulo de SDL
Paddle rightPaddle = {rightPaddleRect, false, false};   // Estructura para almacenar la paleta y su estado de movimiento



// Función para manejar una paleta controlada por un jugador
void* playerPaddleFunc(void* arg) {
    Paddle* paddle = (Paddle*)arg;
    
    //TODO actualizar a otra variable
    while (!quit) {x
        /**
         * Espera a variable condicional señalizada desde el hilo de lógica de juego. De
         * esta manera se actualiza correctamente el estado de la paleta (moviéndose hacia
         * arriba / abajo) y se mantiene un "tickrate" consistente con los demás elementos.
         */
        pthread_mutex_lock(&gameMutex);
        pthread_cond_wait(&paddleUpdateCond, &gameMutex);
        pthread_mutex_unlock(&gameMutex);

        // Verifica el estado de la paleta y realiza los movimientos respectivos
        if (paddle->isMovingUp && paddle->rect.y > WALL_THICKNESS) {
            paddle->rect.y -= paddleSpeed;
        }
        if (paddle->isMovingDown && paddle->rect.y + PADDLE_HEIGHT < SCREEN_HEIGHT - WALL_THICKNESS) {
            paddle->rect.y += paddleSpeed;
        }
        // Postea el semaforo de actualizacion de la pelota
        sem_post(&ballUpdateSem);
    }
    return NULL;
}

// Funcion para manejar una paleta controlada por IA
void* computerPaddleFunc(void* arg) {
    Paddle* paddle = (Paddle*)arg;
    while (!quit) {
        /**
         * Espera a variable condicional señalizada desde el hilo de lógica de juego. De
         * esta manera se actualiza correctamente el estado de la paleta (moviéndose hacia
         * arriba / abajo) y se mantiene un "tickrate" consistente con los demás elementos.
         */        
        pthread_mutex_lock(&gameMutex);
        pthread_cond_wait(&paddleUpdateCond, &gameMutex);
        pthread_mutex_unlock(&gameMutex);

        // Verifica el estado de la paleta y la posicion de la pelota, se mueve directamente
        // hacia la altura de la pelota respecto a la paleta
        if (global_ball.rect.y < paddle->rect.y && paddle->rect.y > WALL_THICKNESS){
            paddle->rect.y -= paddleSpeed;
}
        if (global_ball.rect.y > paddle->rect.y + PADDLE_HEIGHT && paddle->rect.y + PADDLE_HEIGHT < SCREEN_HEIGHT - WALL_THICKNESS){
            paddle->rect.y += paddleSpeed;
        }
        // Postea el semaforo de actualizacion de la pelota
        // TODO revisar si se cambia a barrera
        sem_post(&ballUpdateSem);
    }
    return NULL;
}

// Funcion para manejar los movimientos de la pelota
void* ballFunc(void* arg) {
    BallData* ballData = (BallData*)arg;
    Ball* ball = ballData->ball;
    int* player1Score = ballData->player1Score;
    int* player2Score = ballData->player2Score;
    while (!quit) {

        // Actualizacion de Posicion, esto se puede realizar de manera independiente de los demas hilos
        ball->rect.x += ball->velX;
        ball->rect.y += ball->velY;

        // Verificacion de colisiones con paredes
        if (ball -> rect.y <= WALL_THICKNESS || ball-> rect.y + BALL_SIZE >= SCREEN_HEIGHT - WALL_THICKNESS) {
            ball->velY = -ball->velY;
        }

        //TODO Espera a que actualicen paletas antes de verificar colisiones con jugador
        //TODO Calculos
        /**
         * Espera a que ambas paletas actualicen su posicion por medio de un semaforo.
         * Esto es necesario para evitar que se calculen colisiones con posiciones de
         * paleta viejas.
         */
        sem_wait(&ballUpdateSem);
        sem_wait(&ballUpdateSem);

        // Verifica rebotes con la paleta izquierda
        if (ball->rect.x <= leftPaddle.rect.x + PADDLE_WIDTH &&
            ball->rect.y + BALL_SIZE >= leftPaddle.rect.y &&
            ball->rect.y <= leftPaddle.rect.y + PADDLE_HEIGHT) {
            ball->velX = -ball->velX;
        }

        // Verifica rebotes con la paleta derecha
        if (ball->rect.x + BALL_SIZE >= rightPaddle.rect.x &&
            ball->rect.y + BALL_SIZE >= rightPaddle.rect.y &&
            ball->rect.y <= rightPaddle.rect.y + PADDLE_HEIGHT) {
            ball->velX = -ball->velX;
        }  

        // Manejo de la puntuacion
        if (ball->rect.x < 0) {
            // Reiniciar la pelota
            ball->rect.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
            ball->rect.y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
            ball->velX = BALL_SPEED;
            ball->velY = BALL_SPEED;
            (*player1Score)++;
        }

        if (ball->rect.x > SCREEN_WIDTH) {
            // Reiniciar la pelota
            ball->rect.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
            ball->rect.y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
            ball->velX = -BALL_SPEED;
            ball->velY = BALL_SPEED;
            (*player2Score)++;
        }
        /**
         * Desbloquea el Mutex de juego y señaliza a el hilo de renderizado
         * que se encuentra actualizado el estado de juego para que lo muestre
         */
        pthread_mutex_unlock(&gameMutex);
        pthread_cond_signal(&renderCond);
    }
    return NULL;
}

// Función de lógica del juego
void* logicThreadFunc(void* arg) {
    /**
     * Al ser creado inicializa los 3 hilos hijos que necesita, siendo
     * 2 paletas y 1 pelota. El juego inicializa con un puntaje de 0-0.
     * El programa esta diseñado para que siempre se manejen estos hilos 
     * y únicamente se les pasen diferentes rutinas como parámetro.
     */
    pthread_t leftPaddleThread, rightPaddleThread, ballThread;
    int player1Score = 0;
    int player2Score = 0;

    BallData ballData = BallData{&global_ball, &player1Score, &player2Score};

    // Si la modalidad de juego es de un unico jugador, crea un hilo de jugador
    // a la izquierda y una paleta de computadora a la derecha
    if (currentState == SINGLE_PLAYER) {
        pthread_create(&leftPaddleThread, NULL, playerPaddleFunc, &leftPaddle);
        pthread_create(&rightPaddleThread, NULL, computerPaddleFunc, &rightPaddle);
        pthread_create(&ballThread, NULL, ballFunc, &ballData);
    }
    // Si la modalidad de juego es de dos jugadores, crea un hilo de jugador
    // a la izquierda y uno a la derecha
    else if (currentState == TWO_PLAYER) {
        pthread_create(&leftPaddleThread, NULL, playerPaddleFunc, &leftPaddle);
        pthread_create(&rightPaddleThread, NULL, playerPaddleFunc, &rightPaddle);
        pthread_create(&ballThread, NULL, ballFunc, &ballData);
    }
    // Si la modalidad de juego es de simulacion, crea un hilo de computadora
    // a la izquierda y uno a la derecha
    else if (currentState == SIMULATION){
        pthread_create(&leftPaddleThread, NULL, computerPaddleFunc, &leftPaddle);
        pthread_create(&rightPaddleThread, NULL, computerPaddleFunc, &rightPaddle);
        pthread_create(&ballThread, NULL, ballFunc, &ballData);
    }

    /**
     * Entra a un loop donde obtiene el estado del teclado para verificar que teclas
     * estan siendo presionadas en un momento dado.
     */
    while (currentState != QUIT) {
        // Actualizar el estado de las teclas
        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        if (player1Score >= 2){
            quit = true;
            break;
        }

        pthread_mutex_lock(&gameMutex);

        // Actualizar el movimiento de las paletas basado en la entrada
        leftPaddle.isMovingUp = currentKeyStates[SDL_SCANCODE_W];
        leftPaddle.isMovingDown = currentKeyStates[SDL_SCANCODE_S];
        rightPaddle.isMovingUp = currentKeyStates[SDL_SCANCODE_UP];
        rightPaddle.isMovingDown = currentKeyStates[SDL_SCANCODE_DOWN];

        // Señalar que las paletas deben actualizarse
        pthread_cond_broadcast(&paddleUpdateCond);
        pthread_mutex_unlock(&gameMutex);

        usleep(4000);
    }
    printf("AAAAAAAAAA");
    sem_post(&ballUpdateSem);
    sem_post(&ballUpdateSem);
    pthread_join(leftPaddleThread, NULL);
    pthread_join(rightPaddleThread, NULL);
    pthread_join(ballThread, NULL);
    currentState = GAME_OVER;

    return nullptr;
}

bool checkWinCondition(int player1Score, int player2Score){
    if (player1Score >= 3){
        return true;
    }
    return false;
}

// Rutina del hilo de renderizado
void* renderThreadFunc(void* arg) {
    while (currentState != QUIT) {

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render basado en el estado del juego
        if (currentState == MAIN_MENU) {
            if (mainMenuTexture) {
                SDL_RenderCopy(renderer, mainMenuTexture, NULL, NULL);
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

        } else if (currentState == SINGLE_PLAYER || currentState == TWO_PLAYER || currentState == SIMULATION) {
            // Limpia la pantalla del renderer
            pthread_mutex_lock(&gameMutex);
            pthread_cond_wait(&renderCond, &gameMutex);
            pthread_mutex_unlock(&gameMutex);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Color blanco para las paletas
            SDL_RenderFillRect(renderer, &leftPaddle.rect);
            SDL_RenderFillRect(renderer, &rightPaddle.rect);
            SDL_RenderFillRect(renderer, &global_ball.rect);
        } else if (currentState == GAME_OVER){
            if (mainMenuTexture) {
                SDL_RenderCopy(renderer, mainMenuTexture, NULL, NULL);
            }
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        }

        // Actualizar el renderizado
        SDL_RenderPresent(renderer);

        // Control de framerate
        SDL_Delay(4);
    }

    return nullptr;
}

// Función para comprobar si el mouse está dentro de un botón
bool isInside(int x, int y, SDL_Rect buttonRect) {
    return (x >= buttonRect.x) && (x < buttonRect.x + buttonRect.w) && (y >= buttonRect.y) && (y < buttonRect.y + buttonRect.h);
}

// Función de inicialización de SDL
bool init(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "¡SDL no pudo inicializarse! Error SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("SDL2 Pong Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "¡No se pudo crear la ventana! Error SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "¡No se pudo crear el renderer! Error SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "¡SDL_image no pudo inicializarse! Error IMG: " << IMG_GetError() << std::endl;
        return false;
    }

    return true;
}

// Función para cargar texturas
SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());

    if (!loadedSurface) {
        std::cerr << "¡No se pudo cargar la imagen " << path << "! Error SDL_image: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (!newTexture) {
        std::cerr << "¡No se pudo crear la textura de " << path << "! Error SDL: " << SDL_GetError() << std::endl;
    }

    SDL_FreeSurface(loadedSurface);

    return newTexture;
}

// Función para cerrar SDL y limpiar recursos
void close(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!init(window, renderer)) {
        return -1;
    }
    sem_init(&ballUpdateSem, 0, 0);   // Start with 0 because ball should wait initially

    mainMenuTexture = loadTexture("res/mainmenu.png", renderer);
    if (!mainMenuTexture) {
        close(window, renderer);
        return -1;
    }

    // Inicializar el mutex y la variable de condición
    pthread_mutex_init(&gameMutex, NULL);
    pthread_cond_init(&paddleUpdateCond, NULL);

    // Crear el hilo de renderizado
    pthread_t renderThread, gameLogicThread;
    pthread_create(&renderThread, nullptr, renderThreadFunc, nullptr);

    bool gameActive = false;
    // Bucle principal para manejar la entrada y las transiciones de estado
    SDL_Event e;
    while (currentState != QUIT) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                currentState = QUIT;
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);

                // Comprobar clics de botones en el estado MAIN_MENU
                if (currentState == MAIN_MENU) {
                    if (gameActive){
                        pthread_join(gameLogicThread, NULL);
                    }
                    if (isInside(x, y, singlePlayerButton)) {
                        currentState = SINGLE_PLAYER;
                        pthread_create(&gameLogicThread, NULL, logicThreadFunc, NULL);
                    }
                    if (isInside(x, y, twoPlayerButton)) {
                        currentState = TWO_PLAYER;
                        pthread_create(&gameLogicThread, NULL, logicThreadFunc, NULL);
                    }
                    if (isInside(x, y, simulationButton)){
                        currentState = SIMULATION;
                        pthread_create(&gameLogicThread, NULL, logicThreadFunc, NULL);
                    }
                }
                if (currentState == GAME_OVER) {
                    pthread_join(gameLogicThread, NULL);
                    currentState = MAIN_MENU;
                }
            }
        }
        SDL_Delay(4);  // Aproximadamente 60 FPS
    }

    // Limpiar recursos
    pthread_mutex_destroy(&gameMutex);
    pthread_cond_destroy(&paddleUpdateCond);
    SDL_DestroyTexture(mainMenuTexture);
    close(window, renderer);

    return 0;
}
