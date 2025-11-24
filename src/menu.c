// menu.c
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h> // Para logs

// ==================== ENUM PARA TELAS DO JOGO ====================
typedef enum {
    SCREEN_MENU = 0,
    SCREEN_GAME,
    SCREEN_OPTIONS,
    SCREEN_CREDITS,
    SCREEN_EXIT
} GameScreen;

// ==================== DECLARAÇÃO DA FUNÇÃO DO JOGO ====================
void RunUndertaleGame(GameScreen *currentScreen);

// ==================== ESTRUTURAS ====================
typedef struct {
    Vector2 position;
    float size;
    float speed;
} Heart;

typedef struct {
    Vector2 position;
    Rectangle bounds;
    const char* text;
    bool isHovered;
} MenuOption;

typedef struct {
    Vector2 position;      
    Rectangle bounds;      
    const char* text;      
    bool isHovered;        
    Color normalColor;     
    Color hoverColor;      
} EasterEgg;

// ==================== CONSTANTES ====================
#define MENU_SCREEN_WIDTH 800   
#define MENU_SCREEN_HEIGHT 600  

#define GAME_SCREEN_WIDTH 1920
#define GAME_SCREEN_HEIGHT 1080

#define HEART_SIZE 20.0f
#define HEART_SPEED 300.0f

typedef enum {
    MENU_JOGAR = 0,
    MENU_OPCOES,
    MENU_CREDITOS,
    MENU_COUNT
} MenuState;

// ==================== VARIÁVEIS GLOBAIS ====================
static Sound fxSelect; 

// ==================== FUNÇÕES ====================
void DrawHeart(Heart heart, Color color) { 
    Vector2 center = {heart.position.x, heart.position.y};
    DrawCircle(center.x - heart.size/4, center.y - heart.size/4, heart.size/2, color);
    DrawCircle(center.x + heart.size/4, center.y - heart.size/4, heart.size/2, color);
    Vector2 v1 = {center.x - heart.size/2, center.y - heart.size/4};
    Vector2 v2 = {center.x + heart.size/2, center.y - heart.size/4};
    Vector2 v3 = {center.x, center.y + heart.size/2};
    DrawTriangle(v1, v2, v3, color);
    DrawRectangle(center.x - heart.size/2, center.y - heart.size/4, heart.size, heart.size/2, color);
}

bool CheckHeartCollision(Heart heart, Rectangle bounds) {
    return CheckCollisionCircleRec(heart.position, HEART_SIZE/2, bounds);
}

void UpdateHeart(Heart* heart, float deltaTime) {
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) heart->position.y -= heart->speed * deltaTime;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) heart->position.y += heart->speed * deltaTime;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) heart->position.x -= heart->speed * deltaTime;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) heart->position.x += heart->speed * deltaTime;

    if (heart->position.x < HEART_SIZE) heart->position.x = HEART_SIZE;
    if (heart->position.x > MENU_SCREEN_WIDTH - HEART_SIZE) heart->position.x = MENU_SCREEN_WIDTH - HEART_SIZE;
    if (heart->position.y < HEART_SIZE) heart->position.y = HEART_SIZE;
    if (heart->position.y > MENU_SCREEN_HEIGHT - HEART_SIZE) heart->position.y = MENU_SCREEN_HEIGHT - HEART_SIZE;
}

// ==================== MAIN ====================
int main(void) {
    InitWindow(MENU_SCREEN_WIDTH, MENU_SCREEN_HEIGHT, "Menu - Undertale Style");
    SetTargetFPS(60);

    // --- INICIALIZAÇÃO DE ÁUDIO BLINDADA ---
    InitAudioDevice(); 
    SetMasterVolume(1.0f); // Garante volume máximo

    // Tenta carregar o som de dois lugares diferentes para evitar erros
    fxSelect = LoadSound("../resources/select.wav"); 
    if (fxSelect.frameCount == 0) {
        fxSelect = LoadSound("resources/select.wav");
    }
    // ---------------------------------------

    Font font = GetFontDefault();
    int fontSize = 40;
    float fontSpacing = 2.0f;

    GameScreen currentScreen = SCREEN_MENU;

    Heart playerHeart = {
        .position = {MENU_SCREEN_WIDTH / 2.0f, MENU_SCREEN_HEIGHT / 2.0f},
        .size = HEART_SIZE,
        .speed = HEART_SPEED
    };

    MenuOption options[MENU_COUNT];
    options[MENU_JOGAR].text = "Jogar"; options[MENU_JOGAR].isHovered = false;
    options[MENU_JOGAR].position = (Vector2){MENU_SCREEN_WIDTH / 2.0f, MENU_SCREEN_HEIGHT / 2.0f - 50};
    
    options[MENU_OPCOES].text = "Opcoes"; options[MENU_OPCOES].isHovered = false;
    options[MENU_OPCOES].position = (Vector2){MENU_SCREEN_WIDTH / 3.0f, MENU_SCREEN_HEIGHT / 2.0f + 80};
    
    options[MENU_CREDITOS].text = "Creditos"; options[MENU_CREDITOS].isHovered = false;
    options[MENU_CREDITOS].position = (Vector2){MENU_SCREEN_WIDTH * 2.0f / 3.0f, MENU_SCREEN_HEIGHT / 2.0f + 80};

    for (int i = 0; i < MENU_COUNT; i++) {
        Vector2 textSize = MeasureTextEx(font, options[i].text, fontSize, fontSpacing);
        options[i].bounds = (Rectangle){
            options[i].position.x - textSize.x / 2.0f - 20,
            options[i].position.y - textSize.y / 2.0f - 10,
            textSize.x + 40,
            textSize.y + 20
        };
    }

    EasterEgg easterEgg;
    easterEgg.text = "sem saida";
    easterEgg.normalColor = BLACK; 
    easterEgg.hoverColor = YELLOW; 
    easterEgg.isHovered = false;
    
    int easterEggFontSize = 24;
    Vector2 easterEggTextSize = MeasureTextEx(font, easterEgg.text, easterEggFontSize, fontSpacing);
    float marginX = 80; float marginY = 80;
    easterEgg.position = (Vector2){MENU_SCREEN_WIDTH - easterEggTextSize.x - marginX, MENU_SCREEN_HEIGHT - easterEggTextSize.y - marginY};
    float padding = 15;
    easterEgg.bounds = (Rectangle){
        easterEgg.position.x - padding, easterEgg.position.y - padding,
        easterEggTextSize.x + (padding * 2), easterEggTextSize.y + (padding * 2)
    };

    int selectedOption = -1;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

        switch (currentScreen) {
            case SCREEN_MENU: {
                UpdateHeart(&playerHeart, deltaTime);
                for (int i = 0; i < MENU_COUNT; i++) {
                    options[i].isHovered = CheckHeartCollision(playerHeart, options[i].bounds);
                    if (options[i].isHovered && IsKeyPressed(KEY_Z)) {
                        PlaySound(fxSelect); 
                        selectedOption = i;
                    }
                }
                easterEgg.isHovered = CheckHeartCollision(playerHeart, easterEgg.bounds);

                if (selectedOption != -1) {
                    switch(selectedOption) {
                        case MENU_JOGAR:
                            currentScreen = SCREEN_GAME; 
                            SetWindowSize(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
                            SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - GAME_SCREEN_WIDTH/2, GetMonitorHeight(GetCurrentMonitor())/2 - GAME_SCREEN_HEIGHT/2);
                            ToggleFullscreen();
                            break;
                        case MENU_OPCOES: currentScreen = SCREEN_OPTIONS; break;
                        case MENU_CREDITOS: currentScreen = SCREEN_CREDITS; break;
                    }
                    selectedOption = -1; 
                }

                BeginDrawing();
                ClearBackground(BLACK);
                const char* title = "MEU JOGO";
                int titleSize = 70;
                Vector2 titleMeasure = MeasureTextEx(font, title, titleSize, fontSpacing);
                DrawTextEx(font, title, (Vector2){MENU_SCREEN_WIDTH / 2.0f - titleMeasure.x / 2.0f, 80}, titleSize, fontSpacing, WHITE);

                for (int i = 0; i < MENU_COUNT; i++) {
                    Color textColor = options[i].isHovered ? YELLOW : WHITE;
                    Vector2 textSize = MeasureTextEx(font, options[i].text, fontSize, fontSpacing);
                    DrawTextEx(font, options[i].text, (Vector2){options[i].position.x - textSize.x / 2.0f, options[i].position.y - textSize.y / 2.0f}, fontSize, fontSpacing, textColor);
                    if (options[i].isHovered) DrawRectangleLinesEx(options[i].bounds, 2, YELLOW);
                }

                Color easterEggColor = easterEgg.isHovered ? easterEgg.hoverColor : easterEgg.normalColor;
                DrawTextEx(font, easterEgg.text, easterEgg.position, easterEggFontSize, fontSpacing, easterEggColor);
                DrawHeart(playerHeart, RED); 
                DrawTextEx(font, "WASD/Setas = Mover | Z = Selecionar | F11 = Fullscreen", (Vector2){10, MENU_SCREEN_HEIGHT - 30}, 16, 1, GRAY);
                EndDrawing();
                break;
            }
            case SCREEN_GAME: {
                // Chama o loop do jogo (definido em undertale_jogo.c)
                RunUndertaleGame(&currentScreen);

                // Se voltou para o menu, restaura a janela
                if (currentScreen == SCREEN_MENU) { 
                    if (IsWindowFullscreen()) ToggleFullscreen();
                    SetWindowSize(MENU_SCREEN_WIDTH, MENU_SCREEN_HEIGHT);
                    SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - MENU_SCREEN_WIDTH/2, GetMonitorHeight(GetCurrentMonitor())/2 - MENU_SCREEN_HEIGHT/2);
                }
                break; 
            }
            case SCREEN_OPTIONS: {
                BeginDrawing();
                ClearBackground(BLUE);
                DrawText("OPÇÕES", 300, 250, 60, WHITE);
                DrawText("Pressione X para voltar", 250, 350, 30, WHITE);
                EndDrawing();
                if (IsKeyPressed(KEY_X)) {
                    PlaySound(fxSelect);
                    currentScreen = SCREEN_MENU;
                }
                break;
            }
            case SCREEN_CREDITS: {
                BeginDrawing();
                ClearBackground(GREEN);
                DrawText("CRÉDITOS", 280, 200, 60, WHITE);
                DrawText("Feito por João e o Inner AI", 220, 300, 30, WHITE);
                DrawText("Pressione X para voltar", 250, 450, 30, WHITE);
                EndDrawing();
                if (IsKeyPressed(KEY_X)) {
                    PlaySound(fxSelect);
                    currentScreen = SCREEN_MENU;
                }
                break;
            }
            case SCREEN_EXIT: {
                CloseWindow();
                break;
            }
        }
    }

    UnloadSound(fxSelect); 
    CloseAudioDevice(); 
    CloseWindow();
    return 0;
}