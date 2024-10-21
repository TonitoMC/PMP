#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <pthread.h>
#include <atomic>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Enumeration for different states/screens in the game
enum GameState {
    MAIN_MENU,
    SINGLE_PLAYER,
    SETTINGS,
    QUIT
};

// Shared variable for game state
std::atomic<GameState> currentState(MAIN_MENU);

// Global variables for SDL resources
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* mainMenuTexture = nullptr;

// Button rectangles
SDL_Rect singlePlayerButton = { 175, 210, 450, 75 };
SDL_Rect twoPlayerButton = { 125, 310, 560, 75 };
SDL_Rect simulationButton = { 125, 400, 560, 75 };
SDL_Rect settingsButton = { 660, 45, 95, 95 };

// Paddles and gameplay properties for Pong
SDL_Rect playerPaddle = { 50, SCREEN_HEIGHT / 2 - 50, 20, 100 }; // Left paddle
SDL_Rect aiPaddle = { SCREEN_WIDTH - 70, SCREEN_HEIGHT / 2 - 50, 20, 100 }; // Right paddle
int paddleSpeed = 5; // Paddle movement speed
int playerPaddleVel = 0; // Player paddle velocity

// Function for the rendering thread
void* renderThreadFunc(void* arg) {
    while (currentState != QUIT) {
        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render based on the current state
        if (currentState == MAIN_MENU) {
            if (mainMenuTexture) {
                SDL_RenderCopy(renderer, mainMenuTexture, NULL, NULL);
            }

            // Render all menu buttons
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Green color for buttons
            SDL_RenderFillRect(renderer, &singlePlayerButton);
            SDL_RenderFillRect(renderer, &twoPlayerButton);
            SDL_RenderFillRect(renderer, &simulationButton);
            SDL_RenderFillRect(renderer, &settingsButton);
        } else if (currentState == SINGLE_PLAYER) {
            // Render the paddles
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color for paddles
            SDL_RenderFillRect(renderer, &playerPaddle);
            SDL_RenderFillRect(renderer, &aiPaddle);
        }

        // Present the updated screen
        SDL_RenderPresent(renderer);

        // Delay to control frame rate (~60 FPS)
        SDL_Delay(16);
    }

    return nullptr;
}

// Function to update the logic of the Pong game
void* logicThreadFunc(void* arg) {
    while (currentState != QUIT) {
        // Only update paddle positions if in SINGLE_PLAYER state
        if (currentState == SINGLE_PLAYER) {
            // Move player paddle
            playerPaddle.y += playerPaddleVel;

            // Keep the player paddle within screen bounds
            if (playerPaddle.y < 0) playerPaddle.y = 0;
            if (playerPaddle.y > SCREEN_HEIGHT - playerPaddle.h) playerPaddle.y = SCREEN_HEIGHT - playerPaddle.h;
        }

        // Delay to control frame rate (~60 FPS)
        SDL_Delay(16);
    }

    return nullptr;
}

// Check if mouse is inside a rectangle (button)
bool isInside(int x, int y, SDL_Rect buttonRect) {
    return (x >= buttonRect.x) && (x < buttonRect.x + buttonRect.w) && (y >= buttonRect.y) && (y < buttonRect.y + buttonRect.h);
}

// Initialization function
bool init(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("SDL2 Pong Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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

// Cleanup function
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

    mainMenuTexture = loadTexture("res/mainmenu.png", renderer);
    if (!mainMenuTexture) {
        close(window, renderer);
        return -1;
    }

    // Create rendering thread
    pthread_t renderThread;
    pthread_create(&renderThread, nullptr, renderThreadFunc, nullptr);

    // Create logic thread
    pthread_t logicThread;
    pthread_create(&logicThread, nullptr, logicThreadFunc, nullptr);

    // Main loop for handling input and state transitions
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        // Poll events for input handling
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                currentState = QUIT;
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);

                // Check for button clicks in MAIN_MENU state
                if (currentState == MAIN_MENU) {
                    if (isInside(x, y, singlePlayerButton)) {
                        std::cout << "Single Player button clicked!" << std::endl;
                        currentState = SINGLE_PLAYER; // Change state to single player
                    } else if (isInside(x, y, twoPlayerButton)) {
                        std::cout << "Two Player button clicked!" << std::endl;
                        // Handle state change or actions
                    } else if (isInside(x, y, simulationButton)) {
                        std::cout << "Simulation button clicked!" << std::endl;
                        // Handle state change or actions
                    } else if (isInside(x, y, settingsButton)) {
                        std::cout << "Settings button clicked!" << std::endl;
                        currentState = SETTINGS; // Change state to settings
                    }
                }
            } else if (e.type == SDL_KEYDOWN) {
                // Handle paddle movement for single player
                if (currentState == SINGLE_PLAYER) {
                    if (e.key.keysym.sym == SDLK_w) {
                        playerPaddleVel = -paddleSpeed; // Move up
                    } else if (e.key.keysym.sym == SDLK_s) {
                        playerPaddleVel = paddleSpeed; // Move down
                    }
                }
            } else if (e.type == SDL_KEYUP) {
                // Stop paddle movement when key is released
                if (currentState == SINGLE_PLAYER) {
                    if (e.key.keysym.sym == SDLK_w || e.key.keysym.sym == SDLK_s) {
                        playerPaddleVel = 0;
                    }
                }
            }
        }

        // Delay to control frame rate (~60 FPS)
        SDL_Delay(16);
    }

    // Clean up
    pthread_join(renderThread, nullptr);
    pthread_join(logicThread, nullptr);
    SDL_DestroyTexture(mainMenuTexture);
    close(window, renderer);

    return 0;
<<<<<<< Updated upstream
}
=======
}






>>>>>>> Stashed changes
