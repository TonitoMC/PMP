#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <pthread.h>
#include <unistd.h> // Para usleep
#include <SDL2/SDL_ttf.h>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int WALL_THICKNESS = 10;
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;
const int BALL_SIZE = 15;
const int PADDLE_SPEED = 11;
const int BALL_SPEED = 13;
const int WINNING_SCORE = 11;

// Definición de FRAME_DELAY
const int FRAME_DELAY = 1000 / 60; // Para 60 FPS

// Colores (RGBA)
SDL_Color wallColor = {255, 255, 255, 255};  // Blanco
SDL_Color paddleColor = {0, 255, 0, 255};    // Verde para las raquetas
SDL_Color ballColor = {255, 255, 0, 255};    // Amarillo para la pelota
SDL_Color backgroundColor = {0, 0, 0, 255};  // Negro para el fondo
SDL_Color netColor = {255, 255, 255, 255};   // Blanco para la red divisoria
SDL_Color scoreColor = {255, 255, 255, 255}; // Blanco para el texto del puntaje

// Variables de puntuación
int player1Score = 0;
int player2Score = 0;

// Estructuras para los hilos de jugadores y la pelota
struct Paddle {
    SDL_Rect rect;
    int speed;
};

struct Ball {
    SDL_Rect rect;
    int velX, velY;
};

// Variables globales para el juego
Paddle player1Paddle, player2Paddle;
Ball ball;
pthread_mutex_t gameMutex;
bool quit = false;

// Funciones para inicializar y limpiar SDL
bool init(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Pong Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! IMG_Error: " << IMG_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    // Cargar la fuente
    font = TTF_OpenFont("res/arial.ttf", 28);
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    return true;
}

bool isMouseOverButton(SDL_Rect& button, int mouseX, int mouseY) {
    return mouseX > button.x && mouseX < button.x + button.w && mouseY > button.y && mouseY < button.y + button.h;
}

SDL_Texture* loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Texture* newTexture = nullptr;

    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (!loadedSurface) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }

    newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (!newTexture) {
        std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
    }

    SDL_FreeSurface(loadedSurface);

    return newTexture;
}

void closeSDL(SDL_Window* window, SDL_Renderer* renderer, TTF_Font* font) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

// Renderizado
void renderWalls(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, wallColor.r, wallColor.g, wallColor.b, wallColor.a);
    SDL_Rect topWall = {0, 0, SCREEN_WIDTH, WALL_THICKNESS}; // Pared superior
    SDL_Rect bottomWall = {0, SCREEN_HEIGHT - WALL_THICKNESS, SCREEN_WIDTH, WALL_THICKNESS}; // Pared inferior
    SDL_RenderFillRect(renderer, &topWall);
    SDL_RenderFillRect(renderer, &bottomWall);
}

void renderPaddle(SDL_Renderer* renderer, Paddle& paddle) {
    SDL_SetRenderDrawColor(renderer, paddleColor.r, paddleColor.g, paddleColor.b, paddleColor.a);
    SDL_RenderFillRect(renderer, &paddle.rect);
}

void renderBall(SDL_Renderer* renderer, Ball& ball) {
    SDL_SetRenderDrawColor(renderer, ballColor.r, ballColor.g, ballColor.b, ballColor.a);
    SDL_RenderFillRect(renderer, &ball.rect);
}

void renderNet(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, netColor.r, netColor.g, netColor.b, netColor.a);
    for (int y = 0; y < SCREEN_HEIGHT; y += 40) {
        SDL_Rect netSegment = {SCREEN_WIDTH / 2 - 1, y, 2, 20}; // Segmentos de la red
        SDL_RenderFillRect(renderer, &netSegment);
    }
}

void renderScore(SDL_Renderer* renderer, TTF_Font* font, int player1Score, int player2Score) {
    SDL_Surface* surface;
    SDL_Texture* texture;
    SDL_Color textColor = {255, 255, 255, 255};  // Blanco

    // Renderizar el puntaje del jugador 1
    std::string player1ScoreText = std::to_string(player1Score);
    surface = TTF_RenderText_Solid(font, player1ScoreText.c_str(), textColor);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect player1ScoreRect = {SCREEN_WIDTH / 4 - surface->w / 2, 50, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &player1ScoreRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // Renderizar el puntaje del jugador 2
    std::string player2ScoreText = std::to_string(player2Score);
    surface = TTF_RenderText_Solid(font, player2ScoreText.c_str(), textColor);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect player2ScoreRect = {3 * SCREEN_WIDTH / 4 - surface->w / 2, 50, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &player2ScoreRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Funciones para mover los jugadores y la pelota
void* movePlayer(void* paddleData) {
    Paddle* paddle = (Paddle*) paddleData;
    while (!quit) {
        pthread_mutex_lock(&gameMutex);

        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

        if (paddle == &player1Paddle) {
            if (currentKeyStates[SDL_SCANCODE_W] && paddle->rect.y > WALL_THICKNESS) {
                paddle->rect.y -= paddle->speed;
            }
            if (currentKeyStates[SDL_SCANCODE_S] && paddle->rect.y + PADDLE_HEIGHT < SCREEN_HEIGHT - WALL_THICKNESS) {
                paddle->rect.y += paddle->speed;
            }
        } else if (paddle == &player2Paddle) {
            if (currentKeyStates[SDL_SCANCODE_UP] && paddle->rect.y > WALL_THICKNESS) {
                paddle->rect.y -= paddle->speed;
            }
            if (currentKeyStates[SDL_SCANCODE_DOWN] && paddle->rect.y + PADDLE_HEIGHT < SCREEN_HEIGHT - WALL_THICKNESS) {
                paddle->rect.y += paddle->speed;
            }
        }

        pthread_mutex_unlock(&gameMutex);
        usleep(16000); // Pausa para reducir la velocidad de ejecución (aproximadamente 60 FPS)
    }

    return nullptr;
}

void* moveBall(void* ballData) {
    Ball* ball = (Ball*) ballData;

    while (!quit) {
        pthread_mutex_lock(&gameMutex);

        ball->rect.x += ball->velX;
        ball->rect.y += ball->velY;

        if (ball->rect.y <= WALL_THICKNESS || ball->rect.y + BALL_SIZE >= SCREEN_HEIGHT - WALL_THICKNESS) {
            ball->velY = -ball->velY;
        }

        // Rebote en el jugador 1
        if (ball->rect.x <= player1Paddle.rect.x + PADDLE_WIDTH &&
            ball->rect.y + BALL_SIZE >= player1Paddle.rect.y &&
            ball->rect.y <= player1Paddle.rect.y + PADDLE_HEIGHT) {
            ball->velX = -ball->velX;
        }

        // Rebote en el jugador 2
        if (ball->rect.x + BALL_SIZE >= player2Paddle.rect.x &&
            ball->rect.y + BALL_SIZE >= player2Paddle.rect.y &&
            ball->rect.y <= player2Paddle.rect.y + PADDLE_HEIGHT) {
            ball->velX = -ball->velX;
        }

        // Manejar la puntuación
        if (ball->rect.x < 0) {
            player2Score++;
            // Reiniciar la pelota
            ball->rect.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
            ball->rect.y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
            ball->velX = BALL_SPEED;
            ball->velY = BALL_SPEED;
        }

        if (ball->rect.x > SCREEN_WIDTH) {
            player1Score++;
            // Reiniciar la pelota
            ball->rect.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
            ball->rect.y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
            ball->velX = -BALL_SPEED;
            ball->velY = BALL_SPEED;
        }

        pthread_mutex_unlock(&gameMutex);
        usleep(16000); // Pausa pequeña para evitar un uso excesivo de CPU (aproximadamente 60 FPS)
    }

    return nullptr;
}

bool checkWinCondition(int player1Score, int player2Score) {
    // Gana el primero que alcance 11 puntos con una diferencia de al menos 2
    if (player1Score >= WINNING_SCORE && (player1Score - player2Score) >= 2) {
        return true; // Jugador 1 gana
    }
    if (player2Score >= WINNING_SCORE && (player2Score - player1Score) >= 2) {
        return true; // Jugador 2 gana
    }
    return false; // Nadie ha ganado todavía
}

// Funciones del juego para diferentes modos
void game_player_computer(SDL_Renderer* renderer, TTF_Font* font) {
    player1Score = 0;
    player2Score = 0;
    quit = false;

    player1Paddle = {{50, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT}, PADDLE_SPEED};
    player2Paddle = {{SCREEN_WIDTH - 50 - PADDLE_WIDTH, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT}, PADDLE_SPEED};
    ball = {{SCREEN_WIDTH / 2 - BALL_SIZE / 2, SCREEN_HEIGHT / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE}, BALL_SPEED, BALL_SPEED};

    pthread_mutex_init(&gameMutex, NULL);

    pthread_t player1Thread, ballThread;
    pthread_create(&player1Thread, NULL, movePlayer, &player1Paddle);
    pthread_create(&ballThread, NULL, moveBall, &ball);

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks(); // Comienzo del fotograma

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        pthread_mutex_lock(&gameMutex);
        // Movimiento automático de la raqueta del jugador 2 (computadora)
        if (ball.rect.y < player2Paddle.rect.y && player2Paddle.rect.y > WALL_THICKNESS) {
            player2Paddle.rect.y -= player2Paddle.speed;
        }
        if (ball.rect.y > player2Paddle.rect.y + PADDLE_HEIGHT && player2Paddle.rect.y + PADDLE_HEIGHT < SCREEN_HEIGHT - WALL_THICKNESS) {
            player2Paddle.rect.y += player2Paddle.speed;
        }
        pthread_mutex_unlock(&gameMutex);

        // Renderizado
        SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        SDL_RenderClear(renderer);

        renderNet(renderer);
        renderWalls(renderer);
        renderPaddle(renderer, player1Paddle);
        renderPaddle(renderer, player2Paddle);
        renderBall(renderer, ball);

        // Renderizar el puntaje
        renderScore(renderer, font, player1Score, player2Score);

        SDL_RenderPresent(renderer);

        // Limitar la velocidad del fotograma
        Uint32 frameTime = SDL_GetTicks() - frameStart; 
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime); 
        }

        // Verificar condición de victoria
        if (checkWinCondition(player1Score, player2Score)) {
            quit = true;
        }
    }

    pthread_mutex_destroy(&gameMutex);
    pthread_join(player1Thread, NULL);
    pthread_join(ballThread, NULL);
}

void game_pvp(SDL_Renderer* renderer, TTF_Font* font) {
    player1Score = 0;
    player2Score = 0;
    quit = false;

    player1Paddle = {{50, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT}, PADDLE_SPEED};
    player2Paddle = {{SCREEN_WIDTH - 50 - PADDLE_WIDTH, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT}, PADDLE_SPEED};
    ball = {{SCREEN_WIDTH / 2 - BALL_SIZE / 2, SCREEN_HEIGHT / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE}, BALL_SPEED, BALL_SPEED};

    pthread_mutex_init(&gameMutex, NULL);

    pthread_t player1Thread, player2Thread, ballThread;
    pthread_create(&player1Thread, NULL, movePlayer, &player1Paddle);
    pthread_create(&player2Thread, NULL, movePlayer, &player2Paddle);
    pthread_create(&ballThread, NULL, moveBall, &ball);

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks(); // Comienzo del fotograma

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Renderizado
        SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        SDL_RenderClear(renderer);

        renderNet(renderer);
        renderWalls(renderer);
        renderPaddle(renderer, player1Paddle);
        renderPaddle(renderer, player2Paddle);
        renderBall(renderer, ball);

        // Renderizar el puntaje
        renderScore(renderer, font, player1Score, player2Score);

        SDL_RenderPresent(renderer);

        // Limitar la velocidad del fotograma
        Uint32 frameTime = SDL_GetTicks() - frameStart; 
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime); 
        }

        // Verificar condición de victoria
        if (checkWinCondition(player1Score, player2Score)) {
            quit = true;
        }
    }

    pthread_mutex_destroy(&gameMutex);
    pthread_join(player1Thread, NULL);
    pthread_join(player2Thread, NULL);
    pthread_join(ballThread, NULL);
}

void game_cvc(SDL_Renderer* renderer, TTF_Font* font) {
    player1Score = 0;
    player2Score = 0;
    quit = false;

    player1Paddle = {{50, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT}, PADDLE_SPEED};
    player2Paddle = {{SCREEN_WIDTH - 50 - PADDLE_WIDTH, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT}, PADDLE_SPEED};
    ball = {{SCREEN_WIDTH / 2 - BALL_SIZE / 2, SCREEN_HEIGHT / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE}, BALL_SPEED, BALL_SPEED};

    pthread_mutex_init(&gameMutex, NULL);

    pthread_t ballThread;
    pthread_create(&ballThread, NULL, moveBall, &ball);

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks(); // Comienzo del fotograma

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        pthread_mutex_lock(&gameMutex);
        // Movimiento automático de las raquetas
        // Raqueta del jugador 1
        if (ball.rect.y < player1Paddle.rect.y && player1Paddle.rect.y > WALL_THICKNESS) {
            player1Paddle.rect.y -= player1Paddle.speed;
        }
        if (ball.rect.y > player1Paddle.rect.y + PADDLE_HEIGHT && player1Paddle.rect.y + PADDLE_HEIGHT < SCREEN_HEIGHT - WALL_THICKNESS) {
            player1Paddle.rect.y += player1Paddle.speed;
        }

        // Raqueta del jugador 2
        if (ball.rect.y < player2Paddle.rect.y && player2Paddle.rect.y > WALL_THICKNESS) {
            player2Paddle.rect.y -= player2Paddle.speed;
        }
        if (ball.rect.y > player2Paddle.rect.y + PADDLE_HEIGHT && player2Paddle.rect.y + PADDLE_HEIGHT < SCREEN_HEIGHT - WALL_THICKNESS) {
            player2Paddle.rect.y += player2Paddle.speed;
        }
        pthread_mutex_unlock(&gameMutex);

        // Renderizado
        SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        SDL_RenderClear(renderer);

        renderNet(renderer);
        renderWalls(renderer);
        renderPaddle(renderer, player1Paddle);
        renderPaddle(renderer, player2Paddle);
        renderBall(renderer, ball);

        // Renderizar el puntaje
        renderScore(renderer, font, player1Score, player2Score);

        SDL_RenderPresent(renderer);

        // Limitar la velocidad del fotograma
        Uint32 frameTime = SDL_GetTicks() - frameStart; 
        if (frameTime < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frameTime); 
        }

        // Verificar condición de victoria
        if (checkWinCondition(player1Score, player2Score)) {
            quit = true;
        }
    }

    pthread_mutex_destroy(&gameMutex);
    pthread_join(ballThread, NULL);
}

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;

    if (!init(window, renderer, font)) {
        return -1;
    }

    SDL_Texture* mainMenuTexture = loadTexture("res/mainmenu.png", renderer);
    if (!mainMenuTexture) {
        closeSDL(window, renderer, font);
        return -1;
    }

    // Definir las áreas de los botones del menú
    SDL_Rect onePlayerButton = {100, 200, 200, 50}; // Ajusta las coordenadas y tamaño según tu diseño
    SDL_Rect twoPlayersButton = {100, 300, 200, 50}; // Ajusta las coordenadas y tamaño según tu diseño
    SDL_Rect simulationButton = {100, 400, 200, 50}; // Ajusta las coordenadas y tamaño según tu diseño

    bool quitMenu = false;
    SDL_Event e;

    while (!quitMenu) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quitMenu = true;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (isMouseOverButton(onePlayerButton, mouseX, mouseY)) {
                    game_player_computer(renderer, font);  // Inicia el juego de 1 jugador contra la máquina
                }

                if (isMouseOverButton(twoPlayersButton, mouseX, mouseY)) {
                    game_pvp(renderer, font);  // Inicia el juego de 2 jugadores
                }

                if (isMouseOverButton(simulationButton, mouseX, mouseY)) {
                    game_cvc(renderer, font);  // Inicia el modo simulación
                }
            }
        }

        // Renderizar el menú principal
        SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, mainMenuTexture, NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(mainMenuTexture);
    closeSDL(window, renderer, font);

    return 0;
}