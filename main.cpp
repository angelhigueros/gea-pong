#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <algorithm>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

struct PositionComponent {
    float x, y;
};

struct VelocityComponent {
    float velX, velY;
};

struct PaddleComponent {
    float width, height;
};

struct BallComponent {
    float radius;
};

struct BlockComponent {
    float width, height;
    bool destroyed = false;
    SDL_Color color;
};

enum EntityType { PLAYER, BALL, BLOCK };

struct Entity {
    EntityType type;
    PositionComponent position;
    VelocityComponent velocity;
    PaddleComponent* paddle = nullptr;
    BallComponent* ball = nullptr;
    BlockComponent* block = nullptr;
};

std::vector<Entity> entities;
bool running = true;

Entity& createPaddle(float x, float y) {
    Entity paddle;
    paddle.type = PLAYER;
    paddle.position = { x, y };
    paddle.paddle = new PaddleComponent{ 100.0f, 20.0f };
    entities.push_back(paddle);
    return entities.back();
}

Entity& createBall(float x, float y, float velX, float velY) {
    Entity ball;
    ball.type = BALL;
    ball.position = { x, y };
    ball.velocity = { velX, velY };
    ball.ball = new BallComponent{ 10.0f };
    entities.push_back(ball);
    return entities.back();
}

Entity& createBlock(float x, float y, float w, float h, SDL_Color color) {
    Entity block;
    block.type = BLOCK;
    block.position = { x, y };
    block.block = new BlockComponent{w, h, false, color}; 
    entities.push_back(block);
    return entities.back();
}

// Inicializar bloques con colores diferentes por fila
void initBlocks() {
    int rows = 5;
    int cols = 10;
    float blockWidth = WINDOW_WIDTH / cols;
    float blockHeight = 30;

    SDL_Color colors[] = {
        {255, 0, 0, 255},    
        {255, 165, 0, 255},  
        {255, 255, 0, 255},  
        {0, 255, 0, 255},   
        {0, 0, 255, 255}     
    };
    
    for (int row = 0; row < rows; ++row) {
        SDL_Color color = colors[row % 5];  
        for (int col = 0; col < cols; ++col) {
            createBlock(col * blockWidth, row * blockHeight, blockWidth - 5, blockHeight - 5, color);
        }
    }
}

void handleEvents(SDL_Event& event) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
    }
}

void updatePaddle(Entity& paddle, float deltaTime) {
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_LEFT]) {
        paddle.position.x -= 400.0f * deltaTime;
    }
    if (state[SDL_SCANCODE_RIGHT]) {
        paddle.position.x += 400.0f * deltaTime;
    }
    paddle.position.x = std::max(0.0f, std::min(WINDOW_WIDTH - paddle.paddle->width, paddle.position.x));
}

void updateBall(Entity& ball, float deltaTime) {
    // Ajuste de velocidad base para el movimiento
    float speedMultiplier = 400.0f;

    ball.position.x += ball.velocity.velX * speedMultiplier * deltaTime;
    ball.position.y += ball.velocity.velY * speedMultiplier * deltaTime;

    // Colisiones con los bordes de la ventana
    if (ball.position.x <= 0 || ball.position.x >= WINDOW_WIDTH - ball.ball->radius * 2) {
        ball.velocity.velX = -ball.velocity.velX * 1.05f;
    }
    if (ball.position.y <= 0) {
        ball.velocity.velY = -ball.velocity.velY * 1.05f; 
    }

    // Game Over si la pelota toca la parte inferior de la ventana
    if (ball.position.y >= WINDOW_HEIGHT - ball.ball->radius * 2) {
        running = false;
        std::cout << "Game Over: La pelota tocó la parte inferior" << std::endl;
    }
}

// Manejo de colisiones
void handleCollisions() {
    for (auto& entity : entities) {
        if (entity.type == BALL) {
            for (auto& target : entities) {
                if (target.type == PLAYER) {
                    if (entity.position.x + entity.ball->radius * 2 >= target.position.x &&
                        entity.position.x <= target.position.x + target.paddle->width &&
                        entity.position.y + entity.ball->radius * 2 >= target.position.y &&
                        entity.position.y <= target.position.y + target.paddle->height) {
                        entity.velocity.velY = -entity.velocity.velY * 1.05f;
                    }
                }
            }
            
            for (auto& block : entities) {
                if (block.type == BLOCK && !block.block->destroyed) {
                    if (entity.position.x + entity.ball->radius * 2 >= block.position.x &&
                        entity.position.x <= block.position.x + block.block->width &&
                        entity.position.y + entity.ball->radius * 2 >= block.position.y &&
                        entity.position.y <= block.position.y + block.block->height) {
                        block.block->destroyed = true;
                        entity.velocity.velY = -entity.velocity.velY * 1.05f;
                    }
                }
            }
        }
    }
}

// Renderizado de la escena
void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (const auto& entity : entities) {
        if (entity.type == PLAYER) {
            SDL_Rect paddleRect = {static_cast<int>(entity.position.x), static_cast<int>(entity.position.y), 
                                   static_cast<int>(entity.paddle->width), static_cast<int>(entity.paddle->height)};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &paddleRect);
        } else if (entity.type == BALL) {
            SDL_Rect ballRect = {static_cast<int>(entity.position.x), static_cast<int>(entity.position.y), 
                                 static_cast<int>(entity.ball->radius * 2), static_cast<int>(entity.ball->radius * 2)};
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &ballRect);
        } else if (entity.type == BLOCK && !entity.block->destroyed) {
            SDL_Rect blockRect = {static_cast<int>(entity.position.x), static_cast<int>(entity.position.y), 
                                  static_cast<int>(entity.block->width), static_cast<int>(entity.block->height)};
            SDL_SetRenderDrawColor(renderer, entity.block->color.r, entity.block->color.g, entity.block->color.b, entity.block->color.a);
            SDL_RenderFillRect(renderer, &blockRect);
        }
    }
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error inicializando SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Breakout ECS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Crear paddle y pelota con velocidad inicial
    createPaddle(WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT - 30);
    createBall(WINDOW_WIDTH / 2 - 10, WINDOW_HEIGHT / 2 - 10, 0.2f, 0.2f);
    initBlocks();

    Uint32 lastTime = SDL_GetTicks();
    SDL_Event event;

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        handleEvents(event);

        for (auto& entity : entities) {
            if (entity.type == PLAYER) {
                updatePaddle(entity, deltaTime);
            } else if (entity.type == BALL) {
                updateBall(entity, deltaTime);
            }
        }

        handleCollisions();
        render(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
