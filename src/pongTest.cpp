#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int WALL_THICKNESS = 10;
const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;
const int BALL_SIZE = 15;
const int PADDLE_SPEED = 10;
const int BALL_SPEED = 5;
const int FPS = 60;
const int FRAME_DELAY = 1000 / FPS;  // Cada frame debe durar 16 ms (1000 ms / 60 FPS)

// Colores (RGBA)
SDL_Color wallColor = {255, 255, 255, 255};  // Blanco
SDL_Color paddleColor = {0, 255, 0, 255};    // Verde para las raquetas
SDL_Color ballColor = {255, 255, 0, 255};    // Amarillo para la pelota
SDL_Color backgroundColor = {0, 0, 0, 255};  // Negro para el fondo
SDL_Color netColor = {255, 255, 255, 255};   // Blanco para la red divisoria

// Áreas de las opciones del menú
SDL_Rect onePlayerButton = {SCREEN_WIDTH / 2 - 150, 200, 300, 50}; // Aproximado para "1 Jugador"
SDL_Rect twoPlayersButton = {SCREEN_WIDTH / 2 - 150, 300, 300, 50}; // Aproximado para "2 Jugadores"
SDL_Rect simulationButton = {SCREEN_WIDTH / 2 - 150, 400, 300, 50}; // Aproximado para "Simulación"

bool init(SDL_Window*& window, SDL_Renderer*& renderer) {
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

    return true;
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

// Función para verificar si se ha hecho clic en un botón
bool isMouseOverButton(SDL_Rect& button, int mouseX, int mouseY) {
    return mouseX > button.x && mouseX < button.x + button.w && mouseY > button.y && mouseY < button.y + button.h;
}

void renderTexture(SDL_Texture* texture, SDL_Renderer* renderer) {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void renderWalls(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, wallColor.r, wallColor.g, wallColor.b, wallColor.a);
    SDL_Rect topWall = {0, 0, SCREEN_WIDTH, WALL_THICKNESS}; // Pared superior
    SDL_Rect bottomWall = {0, SCREEN_HEIGHT - WALL_THICKNESS, SCREEN_WIDTH, WALL_THICKNESS}; // Pared inferior
    SDL_RenderFillRect(renderer, &topWall);
    SDL_RenderFillRect(renderer, &bottomWall);
}

void renderPaddle(SDL_Renderer* renderer, SDL_Rect& paddle) {
    SDL_SetRenderDrawColor(renderer, paddleColor.r, paddleColor.g, paddleColor.b, paddleColor.a);
    SDL_RenderFillRect(renderer, &paddle);
}

void renderBall(SDL_Renderer* renderer, SDL_Rect& ball) {
    SDL_SetRenderDrawColor(renderer, ballColor.r, ballColor.g, ballColor.b, ballColor.a);
    SDL_RenderFillRect(renderer, &ball);
}

void renderNet(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, netColor.r, netColor.g, netColor.b, netColor.a);
    for (int y = 0; y < SCREEN_HEIGHT; y += 40) {
        SDL_Rect netSegment = {SCREEN_WIDTH / 2 - 1, y, 2, 20}; // Segmentos de la red
        SDL_RenderFillRect(renderer, &netSegment);
    }
}

void moveBall(SDL_Rect& ball, int& ballVelX, int& ballVelY, SDL_Rect& player1Paddle, SDL_Rect& player2Paddle) {
    ball.x += ballVelX;
    ball.y += ballVelY;

    if (ball.y <= WALL_THICKNESS || ball.y + BALL_SIZE >= SCREEN_HEIGHT - WALL_THICKNESS) {
        ballVelY = -ballVelY; // Rebota verticalmente
    }

    if (ball.x <= player1Paddle.x + PADDLE_WIDTH && ball.y + BALL_SIZE >= player1Paddle.y && ball.y <= player1Paddle.y + PADDLE_HEIGHT) {
        ballVelX = -ballVelX;
    }

    if (ball.x + BALL_SIZE >= player2Paddle.x && ball.y + BALL_SIZE >= player2Paddle.y && ball.y <= player2Paddle.y + PADDLE_HEIGHT) {
        ballVelX = -ballVelX;
    }

    if (ball.x <= 0 || ball.x + BALL_SIZE >= SCREEN_WIDTH) {
        ball.x = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
        ball.y = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
        ballVelX = -ballVelX;
    }
}

void close(SDL_Window* window, SDL_Renderer* renderer) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void game(SDL_Renderer* renderer) {
    SDL_Rect player1Paddle = {30, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT};
    SDL_Rect player2Paddle = {SCREEN_WIDTH - 30 - PADDLE_WIDTH, SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT};

    SDL_Rect ball = {SCREEN_WIDTH / 2 - BALL_SIZE / 2, SCREEN_HEIGHT / 2 - BALL_SIZE / 2, BALL_SIZE, BALL_SIZE};
    int ballVelX = BALL_SPEED;
    int ballVelY = BALL_SPEED;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks(); // Tiempo al inicio del frame

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
        if (currentKeyStates[SDL_SCANCODE_W] && player1Paddle.y > WALL_THICKNESS) {
            player1Paddle.y -= PADDLE_SPEED;
        }
        if (currentKeyStates[SDL_SCANCODE_S] && player1Paddle.y + PADDLE_HEIGHT < SCREEN_HEIGHT - WALL_THICKNESS) {
            player1Paddle.y += PADDLE_SPEED;
        }

        if (ball.y < player2Paddle.y && player2Paddle.y > WALL_THICKNESS) {
            player2Paddle.y -= PADDLE_SPEED;
        }
        if (ball.y > player2Paddle.y + PADDLE_HEIGHT && player2Paddle.y + PADDLE_HEIGHT < SCREEN_HEIGHT - WALL_THICKNESS) {
            player2Paddle.y += PADDLE_SPEED;
        }

        moveBall(ball, ballVelX, ballVelY, player1Paddle, player2Paddle);

        SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        SDL_RenderClear(renderer);

        renderNet(renderer);
        renderWalls(renderer);
        renderPaddle(renderer, player1Paddle);
        renderPaddle(renderer, player2Paddle);
        renderBall(renderer, ball);

        SDL_RenderPresent(renderer);

        // Control de FPS
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (FRAME_DELAY > frameTime) {
            SDL_Delay(FRAME_DELAY - frameTime);  // Esperar el tiempo necesario para mantener los FPS
        }
    }
}

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!init(window, renderer)) {
        return -1;
    }

    SDL_Texture* mainMenuTexture = loadTexture("res/mainmenu.png", renderer);
    if (!mainMenuTexture) {
        close(window, renderer);
        return -1;
    }

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (isMouseOverButton(onePlayerButton, mouseX, mouseY)) {
                    game(renderer);  // Inicia el juego de 1 jugador contra la máquina
                    quit = true;
                }

                if (isMouseOverButton(twoPlayersButton, mouseX, mouseY)) {
                    std::cout << "Modo 2 Jugadores seleccionado" << std::endl;
                }

                if (isMouseOverButton(simulationButton, mouseX, mouseY)) {
                    std::cout << "Modo Simulación seleccionado" << std::endl;
                }
            }
        }
        renderTexture(mainMenuTexture, renderer);
    }

    SDL_DestroyTexture(mainMenuTexture);
    close(window, renderer);

    return 0;
}
