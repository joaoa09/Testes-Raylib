#include "raylib.h"
#include <stdbool.h>

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

// ESTRUTURA DO EASTER EGG - CORRIGIDA E FINALIZADA
typedef struct {
    Vector2 position;      // Posição do texto na tela
    Rectangle bounds;      // Área de colisão
    const char* text;      // Texto a ser exibido
    bool isHovered;        // Se o coração do jogador está sobre
    Color normalColor;     // Cor invisível (fundo)
    Color hoverColor;      // Cor ao passar o coração (amarelo)
} EasterEgg;

// ==================== CONSTANTES ====================
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define HEART_SIZE 20.0f
#define HEART_SPEED 300.0f

typedef enum {
    MENU_JOGAR = 0,
    MENU_OPCOES,
    MENU_CREDITOS,
    MENU_COUNT
} MenuState;

// ==================== FUNÇÕES ====================
void DrawHeart(Heart heart, Color color) { // Adicionado parâmetro de cor
    Vector2 center = {heart.position.x, heart.position.y};

    DrawCircle(center.x - heart.size/4, center.y - heart.size/4, 
               heart.size/2, color); // Usa a cor passada
    DrawCircle(center.x + heart.size/4, center.y - heart.size/4, 
               heart.size/2, color); // Usa a cor passada

    Vector2 v1 = {center.x - heart.size/2, center.y - heart.size/4};
    Vector2 v2 = {center.x + heart.size/2, center.y - heart.size/4};
    Vector2 v3 = {center.x, center.y + heart.size/2};
    DrawTriangle(v1, v2, v3, color); // Usa a cor passada

    DrawRectangle(center.x - heart.size/2, center.y - heart.size/4,
                  heart.size, heart.size/2, color); // Usa a cor passada
}

// Função para verificar colisão entre coração do jogador e uma área
bool CheckHeartCollision(Heart heart, Rectangle bounds) {
    return CheckCollisionCircleRec(heart.position, HEART_SIZE/2, bounds);
}

// Função para atualizar movimento do coração do jogador
void UpdateHeart(Heart* heart, float deltaTime) {
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        heart->position.y -= heart->speed * deltaTime;
    }
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        heart->position.y += heart->speed * deltaTime;
    }
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        heart->position.x -= heart->speed * deltaTime;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        heart->position.x += heart->speed * deltaTime;
    }

    if (heart->position.x < HEART_SIZE) heart->position.x = HEART_SIZE;
    if (heart->position.x > SCREEN_WIDTH - HEART_SIZE) 
        heart->position.x = SCREEN_WIDTH - HEART_SIZE;
    if (heart->position.y < HEART_SIZE) heart->position.y = HEART_SIZE;
    if (heart->position.y > SCREEN_HEIGHT - HEART_SIZE) 
        heart->position.y = SCREEN_HEIGHT - HEART_SIZE;
}

// ==================== MAIN ====================
int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Menu - Undertale Style");
    SetTargetFPS(60);

    Font font = GetFontDefault();
    int fontSize = 40;
    float fontSpacing = 2.0f;

    // Coração do jogador
    Heart playerHeart = { // Renomeado para playerHeart para evitar confusão com cursor
        .position = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f},
        .size = HEART_SIZE,
        .speed = HEART_SPEED
    };

    // Opções do menu
    MenuOption options[MENU_COUNT];

    options[MENU_JOGAR].text = "Jogar";
    options[MENU_JOGAR].isHovered = false;
    options[MENU_JOGAR].position.x = SCREEN_WIDTH / 2.0f;
    options[MENU_JOGAR].position.y = SCREEN_HEIGHT / 2.0f - 50;

    options[MENU_OPCOES].text = "Opcoes";
    options[MENU_OPCOES].isHovered = false;
    options[MENU_OPCOES].position.x = SCREEN_WIDTH / 3.0f;
    options[MENU_OPCOES].position.y = SCREEN_HEIGHT / 2.0f + 80;

    options[MENU_CREDITOS].text = "Creditos";
    options[MENU_CREDITOS].isHovered = false;
    options[MENU_CREDITOS].position.x = SCREEN_WIDTH * 2.0f / 3.0f;
    options[MENU_CREDITOS].position.y = SCREEN_HEIGHT / 2.0f + 80;

    // Calcula bounds das opções
    for (int i = 0; i < MENU_COUNT; i++) {
        Vector2 textSize = MeasureTextEx(font, options[i].text, fontSize, fontSpacing);

        options[i].bounds.x = options[i].position.x - textSize.x / 2.0f - 20;
        options[i].bounds.y = options[i].position.y - textSize.y / 2.0f - 10;
        options[i].bounds.width = textSize.x + 40;
        options[i].bounds.height = textSize.y + 20;
    }

    // ==================== EASTER EGG - INICIALIZAÇÃO ====================
    EasterEgg easterEgg;

    easterEgg.text = "sem saida";
    easterEgg.normalColor = BLACK;  // Invisível (mesma cor do fundo)
    easterEgg.hoverColor = YELLOW;  // Amarelo ao passar o coração
    easterEgg.isHovered = false;

    int easterEggFontSize = 24;
    Vector2 easterEggTextSize = MeasureTextEx(font, easterEgg.text, 
                                              easterEggFontSize, fontSpacing);

    // Posiciona no CANTO INFERIOR DIREITO
    float marginX = 80;  // Margem da direita
    float marginY = 80;  // Margem de baixo

    easterEgg.position.x = SCREEN_WIDTH - easterEggTextSize.x - marginX;
    easterEgg.position.y = SCREEN_HEIGHT - easterEggTextSize.y - marginY;

    // Área de colisão (maior que o texto para facilitar descoberta)
    float padding = 15;
    easterEgg.bounds.x = easterEgg.position.x - padding;
    easterEgg.bounds.y = easterEgg.position.y - padding;
    easterEgg.bounds.width = easterEggTextSize.x + (padding * 2);
    easterEgg.bounds.height = easterEggTextSize.y + (padding * 2);

    // Não precisamos esconder o cursor do sistema se não vamos usá-lo
    // HideCursor(); // Removido
    // =====================================================================

    int selectedOption = -1;

    // Loop principal
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_F11)) {
            ToggleFullscreen();
        }

        // Atualização do coração do jogador
        UpdateHeart(&playerHeart, deltaTime);

        // Colisões das opções
        for (int i = 0; i < MENU_COUNT; i++) {
            options[i].isHovered = CheckHeartCollision(playerHeart, options[i].bounds);

            if (options[i].isHovered && IsKeyPressed(KEY_Z)) {
                selectedOption = i;
            }
        }

        // ==================== EASTER EGG - LÓGICA ====================
        // Agora o easter egg é ativado pelo coração do jogador (playerHeart)
        easterEgg.isHovered = CheckHeartCollision(playerHeart, easterEgg.bounds);

        // Não há lógica de clique para o easter egg, apenas revela o texto
        // =============================================================

        if (selectedOption != -1) {
            switch(selectedOption) {
                case MENU_JOGAR:
                    TraceLog(LOG_INFO, "JOGAR selecionado!");
                    break;
                case MENU_OPCOES:
                    TraceLog(LOG_INFO, "OPÇÕES selecionado!");
                    break;
                case MENU_CREDITOS:
                    TraceLog(LOG_INFO, "CRÉDITOS selecionado!");
                    break;
            }
            selectedOption = -1;
        }

        // ==================== RENDERIZAÇÃO ====================
        BeginDrawing();
        ClearBackground(BLACK);

        // Título
        const char* title = "MEU JOGO";
        int titleSize = 70;
        Vector2 titleMeasure = MeasureTextEx(font, title, titleSize, fontSpacing);
        DrawTextEx(font, title, 
                   (Vector2){SCREEN_WIDTH / 2.0f - titleMeasure.x / 2.0f, 80}, 
                   titleSize, fontSpacing, WHITE);

        // Opções do menu
        for (int i = 0; i < MENU_COUNT; i++) {
            Color textColor = options[i].isHovered ? YELLOW : WHITE;
            Vector2 textSize = MeasureTextEx(font, options[i].text, fontSize, fontSpacing);
            Vector2 textPos = {
                options[i].position.x - textSize.x / 2.0f,
                options[i].position.y - textSize.y / 2.0f
            };

            DrawTextEx(font, options[i].text, textPos, fontSize, fontSpacing, textColor);

            // Debug: mostra a área de colisão das opções
            if (options[i].isHovered) {
                DrawRectangleLinesEx(options[i].bounds, 2, YELLOW);
            }
        }

        // ==================== DESENHA EASTER EGG ====================
        Color easterEggColor = easterEgg.isHovered ? 
                               easterEgg.hoverColor : easterEgg.normalColor;

        DrawTextEx(font, easterEgg.text, easterEgg.position, 
                   easterEggFontSize, fontSpacing, easterEggColor);

        // MODO DEBUG - Descomente a linha abaixo para ver onde está o easter egg
        // DrawRectangleLinesEx(easterEgg.bounds, 2, RED);
        // ============================================================

        // Coração do jogador (vermelho)
        DrawHeart(playerHeart, RED); // Sempre vermelho

        // Instruções
        DrawTextEx(font, "WASD/Setas = Mover | Z = Selecionar | F11 = Fullscreen", 
                   (Vector2){10, SCREEN_HEIGHT - 30}, 16, 1, GRAY);

        EndDrawing();
    }

    // ShowCursor(); // Não precisamos se não escondemos
    CloseWindow();

    return 0;
}
