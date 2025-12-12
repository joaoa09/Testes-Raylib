#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

// ==================== ENUM PARA TELAS DO JOGO ====================
typedef enum {
    SCREEN_MENU = 0,
    SCREEN_GAME,
    SCREEN_OPTIONS,
    SCREEN_CREDITS,
    SCREEN_EXIT
} GameScreen;

// ==================== DECLARAÇÃO DA FUNÇÃO DO JOGO ====================
// Essa função está no seu outro arquivo (o do jogo). 
// O compilador precisa saber que ela existe.
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

// ==================== FUNÇÕES AUXILIARES ====================
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
    InitWindow(MENU_SCREEN_WIDTH, MENU_SCREEN_HEIGHT, "PITale - Menu");
    SetTargetFPS(60);

    // --- INICIALIZAÇÃO DE ÁUDIO ---
    InitAudioDevice();
    SetMasterVolume(1.0f);

    // Carregamento seguro do som (tenta duas pastas)
    fxSelect = LoadSound("../resources/select.wav");
    if (fxSelect.frameCount == 0) {
        fxSelect = LoadSound("resources/select.wav");
    }

    Font font = GetFontDefault();
    int fontSize = 40;
    float fontSpacing = 2.0f;

    GameScreen currentScreen = SCREEN_MENU;

    // O coração começa um pouco mais baixo para facilitar a navegação
    Heart playerHeart = {
        .position = {MENU_SCREEN_WIDTH / 2.0f, MENU_SCREEN_HEIGHT / 2.0f + 20},
        .size = HEART_SIZE,
        .speed = HEART_SPEED
    };

    MenuOption options[MENU_COUNT];

    // --- CONFIGURAÇÃO DO LAYOUT EM "PIRÂMIDE" ---
    
    // 1. JOGAR: No centro, um pouco acima do meio da tela
    options[MENU_JOGAR].text = "Jogar"; 
    options[MENU_JOGAR].isHovered = false;
    options[MENU_JOGAR].position = (Vector2){MENU_SCREEN_WIDTH / 2.0f, MENU_SCREEN_HEIGHT / 2.0f - 40};

    // 2. OPÇÕES: No lado ESQUERDO, abaixo do meio
    options[MENU_OPCOES].text = "Opcoes"; 
    options[MENU_OPCOES].isHovered = false;
    // Posição X = 25% da tela (esquerda)
    options[MENU_OPCOES].position = (Vector2){MENU_SCREEN_WIDTH * 0.25f, MENU_SCREEN_HEIGHT / 2.0f + 60};

    // 3. CRÉDITOS: No lado DIREITO, abaixo do meio
    options[MENU_CREDITOS].text = "Creditos"; 
    options[MENU_CREDITOS].isHovered = false;
    // Posição X = 75% da tela (direita)
    options[MENU_CREDITOS].position = (Vector2){MENU_SCREEN_WIDTH * 0.75f, MENU_SCREEN_HEIGHT / 2.0f + 60};

    // Gera as caixas de colisão (Hitboxes) automaticamente
    for (int i = 0; i < MENU_COUNT; i++) {
        Vector2 textSize = MeasureTextEx(font, options[i].text, fontSize, fontSpacing);
        options[i].bounds = (Rectangle){
            options[i].position.x - textSize.x / 2.0f - 20, // Margem X
            options[i].position.y - textSize.y / 2.0f - 10, // Margem Y
            textSize.x + 40,
            textSize.y + 20
        };
    }

    // Configuração do Easter Egg
    EasterEgg easterEgg;
    easterEgg.text = "sem saida";
    easterEgg.normalColor = Fade(BLACK, 0.0f); // Invisível
    easterEgg.hoverColor = YELLOW;
    easterEgg.isHovered = false;
    int easterEggFontSize = 24;
    Vector2 easterEggTextSize = MeasureTextEx(font, easterEgg.text, easterEggFontSize, fontSpacing);
    float marginX = 40; float marginY = 40;
    easterEgg.position = (Vector2){MENU_SCREEN_WIDTH - easterEggTextSize.x - marginX, MENU_SCREEN_HEIGHT - easterEggTextSize.y - marginY};
    float padding = 15;
    easterEgg.bounds = (Rectangle){
        easterEgg.position.x - padding, easterEgg.position.y - padding,
        easterEggTextSize.x + (padding * 2), easterEggTextSize.y + (padding * 2)
    };

    int selectedOption = -1;

    // LOOP PRINCIPAL
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_F11)) ToggleFullscreen();

        switch (currentScreen) {
            case SCREEN_MENU: {
                UpdateHeart(&playerHeart, deltaTime);

                // Colisão do Coração com os Botões
                for (int i = 0; i < MENU_COUNT; i++) {
                    options[i].isHovered = CheckHeartCollision(playerHeart, options[i].bounds);
                    if (options[i].isHovered && IsKeyPressed(KEY_Z)) {
                        PlaySound(fxSelect);
                        selectedOption = i;
                    }
                }
                easterEgg.isHovered = CheckHeartCollision(playerHeart, easterEgg.bounds);

                // Ação ao selecionar
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

                // --- TÍTULO: PITale ---
                const char* title = "PITale";
                int titleSize = 70;
                Vector2 titleMeasure = MeasureTextEx(font, title, titleSize, fontSpacing);
                DrawTextEx(font, title, (Vector2){MENU_SCREEN_WIDTH / 2.0f - titleMeasure.x / 2.0f, 80}, titleSize, fontSpacing, WHITE);

                // Desenha os botões
                for (int i = 0; i < MENU_COUNT; i++) {
                    Color textColor = options[i].isHovered ? YELLOW : WHITE;
                    Vector2 textSize = MeasureTextEx(font, options[i].text, fontSize, fontSpacing);
                    DrawTextEx(font, options[i].text, (Vector2){options[i].position.x - textSize.x / 2.0f, options[i].position.y - textSize.y / 2.0f}, fontSize, fontSpacing, textColor);
                    
                    // Retângulo amarelo ao redor se selecionado
                    if (options[i].isHovered) DrawRectangleLinesEx(options[i].bounds, 2, YELLOW);
                }

                // Easter Egg
                Color easterEggColor = easterEgg.isHovered ? easterEgg.hoverColor : easterEgg.normalColor;
                if(easterEgg.isHovered) DrawTextEx(font, easterEgg.text, easterEgg.position, easterEggFontSize, fontSpacing, easterEggColor);

                // Desenha o Jogador
                DrawHeart(playerHeart, RED);

                // --- RODAPÉ COM INSTRUÇÕES ---
                const char* instructions = "[WASD/SETAS] Mover   [Z] Confirmar   [X] Voltar";
                int instSize = 18;
                int instWidth = MeasureText(instructions, instSize);
                DrawText(instructions, MENU_SCREEN_WIDTH/2 - instWidth/2, MENU_SCREEN_HEIGHT - 30, instSize, GRAY);

                EndDrawing();
                break;
            }
            case SCREEN_GAME: {
                RunUndertaleGame(&currentScreen);
                
                // Retornando do jogo para o menu
                if (currentScreen == SCREEN_MENU) {
                    if (IsWindowFullscreen()) ToggleFullscreen();
                    SetWindowSize(MENU_SCREEN_WIDTH, MENU_SCREEN_HEIGHT);
                    SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - MENU_SCREEN_WIDTH/2, GetMonitorHeight(GetCurrentMonitor())/2 - MENU_SCREEN_HEIGHT/2);
                }
                break;
            }
            case SCREEN_OPTIONS: {
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("OPCOES", MENU_SCREEN_WIDTH/2 - MeasureText("OPCOES", 60)/2, 150, 60, WHITE);
                DrawText("- Não tem nada aqui. -", MENU_SCREEN_WIDTH/2 - MeasureText("- Não tem nada aqui. -", 30)/2, 300, 30, GRAY);
                
                DrawText("[X] VOLTAR", MENU_SCREEN_WIDTH/2 - MeasureText("[X] VOLTAR", 20)/2, 500, 20, YELLOW);
                EndDrawing();
                if (IsKeyPressed(KEY_X)) {
                    PlaySound(fxSelect);
                    currentScreen = SCREEN_MENU;
                }
                break;
            }
            case SCREEN_CREDITS: {
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("CREDITOS", MENU_SCREEN_WIDTH/2 - MeasureText("CREDITOS", 60)/2, 100, 60, WHITE);
                
                const char* nomes = "Joao Augusto\n\nCauan Felipe\n\nVinicius Oliveira\n\nLuis Henrique\n\nVinicius";
                Vector2 nomesSize = MeasureTextEx(font, nomes, 30, 2);
                DrawTextEx(font, nomes, (Vector2){MENU_SCREEN_WIDTH/2 - nomesSize.x/2, 200}, 30, 2, WHITE);
                
                DrawText("[X] VOLTAR", MENU_SCREEN_WIDTH/2 - MeasureText("[X] VOLTAR", 20)/2, 550, 20, YELLOW);
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