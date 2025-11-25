#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

//----------------------------------------------------------------------------------
// Definições e Constantes
//----------------------------------------------------------------------------------
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

#define PLAYER_SPEED 5.0f
#define PLAYER_INITIAL_HP 20

#define ARENA_WIDTH 300
#define ARENA_HEIGHT 300

#define MAX_STARS 200
#define MAX_LIGHTNINGS 15
#define MAX_PARTICLES 500

#define MUSIC_FILE_PATH "hopes_and_dreams.ogg"

//----------------------------------------------------------------------------------
// Estruturas de Dados (Structs)
//----------------------------------------------------------------------------------
typedef struct Player {
    Vector2 position;
    int hp;
    float invincibilityTimer;
    bool isDamaged;
} Player;

typedef struct Star {
    Vector2 position;
    Vector2 speed;
    float radius;
    Color color;
    float rotation;
    bool active;
} Star;

typedef struct Lightning {
    Rectangle warningRect;
    Rectangle strikeRect;
    float timer;
    int state;
    bool active;
    Color color;
} Lightning;

typedef struct Particle {
    Vector2 position;
    Vector2 speed;
    Color color;
    float radius;
    float lifetime;
    bool active;
} Particle;

typedef enum GameState { TITLE, GAMEPLAY, GAME_OVER } GameState;
typedef enum AttackType { NONE, STAR_BLAZING, SHOCKER_BREAKER, CHAOS_SABER, HYPER_GONER } AttackType;

//----------------------------------------------------------------------------------
// Variáveis Globais
//----------------------------------------------------------------------------------
static GameState gameState = TITLE;
static Player player = { 0 };
static Rectangle arena = { 0 };

static Texture2D texHeart;
static Texture2D texStar;
static Texture2D texLightningWarning;
static Texture2D texLightningStrike;
static Texture2D texChaosSaber;
static Texture2D texHyperGonerMouth;
static Texture2D texHyperGonerCharge;
static Texture2D texHyperGonerBeam;
static Texture2D texBackgroundStar;

static Star stars[MAX_STARS] = { 0 };
static Lightning lightnings[MAX_LIGHTNINGS] = { 0 };
static Rectangle chaosSabers[2] = { 0 };
static Particle particles[MAX_PARTICLES] = { 0 };

static Vector2 gonerPosition;
static float gonerChargeTimer;
static float gonerBeamTimer;
static bool gonerActive;
static bool gonerCharging;
static bool gonerBeaming;
static float gonerRotation;

static AttackType currentAttack = NONE;
static float attackTimer = 0.0f;
static float attackDuration = 10.0f;
static int saberPhase = 0;

static float screenShakeIntensity = 0.0f;
static float screenFlashTimer = 0.0f;
static float backgroundStarRotation = 0.0f;

static Music gameMusic;

//----------------------------------------------------------------------------------
// Declaração de Funções
//----------------------------------------------------------------------------------
void InitGame(void);
void UpdateGame(void);
void DrawGame(void);
void UpdateDrawFrame(void);

Texture2D CreateHeartTexture(void);
Texture2D CreateStarTexture(void);
Texture2D CreateLightningWarningTexture(void);
Texture2D CreateLightningStrikeTexture(void);
Texture2D CreateChaosSaberTexture(void);
Texture2D CreateHyperGonerMouthTexture(void);
Texture2D CreateHyperGonerChargeTexture(void);
Texture2D CreateHyperGonerBeamTexture(void);
Texture2D CreateBackgroundStarTexture(void);

void EmitParticle(Vector2 pos, Vector2 speed, Color color, float radius, float lifetime);
void UpdateParticles(void);
void DrawParticles(void);

void InitAttack(AttackType newAttack);
void UpdateStarBlazing(void);
void UpdateShockerBreaker(void);
void UpdateChaosSaber(void);
void UpdateHyperGoner(void);
void DrawStarBlazing(void);
void DrawShockerBreaker(void);
void DrawChaosSaber(void);
void DrawHyperGoner(void);

//----------------------------------------------------------------------------------
// Função Principal
//----------------------------------------------------------------------------------
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Undertale-like Boss Fight - FINAL FORM");
    InitAudioDevice();
    srand(time(NULL));

    gameMusic = LoadMusicStream(MUSIC_FILE_PATH);
    SetMusicVolume(gameMusic, 0.6f);
    PlayMusicStream(gameMusic);

    texHeart = CreateHeartTexture();
    texStar = CreateStarTexture();
    texLightningWarning = CreateLightningWarningTexture();
    texLightningStrike = CreateLightningStrikeTexture();
    texChaosSaber = CreateChaosSaberTexture();
    texHyperGonerMouth = CreateHyperGonerMouthTexture();
    texHyperGonerCharge = CreateHyperGonerChargeTexture();
    texHyperGonerBeam = CreateHyperGonerBeamTexture();
    texBackgroundStar = CreateBackgroundStarTexture();

    InitGame();

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(gameMusic);
        UpdateDrawFrame();
    }

    UnloadTexture(texHeart);
    UnloadTexture(texStar);
    UnloadTexture(texLightningWarning);
    UnloadTexture(texLightningStrike);
    UnloadTexture(texChaosSaber);
    UnloadTexture(texHyperGonerMouth);
    UnloadTexture(texHyperGonerCharge);
    UnloadTexture(texHyperGonerBeam);
    UnloadTexture(texBackgroundStar);

    UnloadMusicStream(gameMusic);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

//----------------------------------------------------------------------------------
// Funções do Jogo
//----------------------------------------------------------------------------------

void InitGame(void)
{
    arena.x = (SCREEN_WIDTH - ARENA_WIDTH) / 2;
    arena.y = (SCREEN_HEIGHT - ARENA_HEIGHT) / 2 + 50;
    arena.width = ARENA_WIDTH;
    arena.height = ARENA_HEIGHT;

    player.position = (Vector2){ arena.x + arena.width / 2, arena.y + arena.height / 2 };
    player.hp = PLAYER_INITIAL_HP;
    player.invincibilityTimer = 0;
    player.isDamaged = false;

    for (int i = 0; i < MAX_STARS; i++) stars[i].active = false;
    for (int i = 0; i < MAX_LIGHTNINGS; i++) lightnings[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;

    gonerActive = false;
    gonerCharging = false;
    gonerBeaming = false;
    gonerChargeTimer = 0;
    gonerBeamTimer = 0;

    InitAttack(STAR_BLAZING);
    attackDuration = 10.0f;
}

void UpdateGame(void)
{
    switch(gameState)
    {
        case TITLE:
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                gameState = GAMEPLAY;
                InitGame();
                if (!IsMusicStreamPlaying(gameMusic)) PlayMusicStream(gameMusic);
            }
        } break;
        case GAMEPLAY:
        {
            attackTimer += GetFrameTime();
            if (attackTimer >= attackDuration)
            {
                int nextAttackNum = GetRandomValue(1, 4);
                InitAttack((AttackType)nextAttackNum);
            }

            if (IsKeyDown(KEY_LEFT)) player.position.x -= PLAYER_SPEED;
            if (IsKeyDown(KEY_RIGHT)) player.position.x += PLAYER_SPEED;
            if (IsKeyDown(KEY_UP)) player.position.y -= PLAYER_SPEED;
            if (IsKeyDown(KEY_DOWN)) player.position.y += PLAYER_SPEED;

            if (player.position.x < arena.x) player.position.x = arena.x;
            if (player.position.x > arena.x + arena.width) player.position.x = arena.x + arena.width;
            if (player.position.y < arena.y) player.position.y = arena.y;
            if (player.position.y > arena.y + arena.height) player.position.y = arena.y + arena.height;

            if (player.invincibilityTimer > 0) player.invincibilityTimer -= GetFrameTime();
            else player.isDamaged = false;

            switch(currentAttack)
            {
                case STAR_BLAZING: UpdateStarBlazing(); break;
                case SHOCKER_BREAKER: UpdateShockerBreaker(); break;
                case CHAOS_SABER: UpdateChaosSaber(); break;
                case HYPER_GONER: UpdateHyperGoner(); break;
                default: break;
            }

            UpdateParticles();

            if (screenShakeIntensity > 0)
            {
                screenShakeIntensity -= GetFrameTime() * 10;
                if (screenShakeIntensity < 0) screenShakeIntensity = 0;
            }

            if (screenFlashTimer > 0) screenFlashTimer -= GetFrameTime();

            backgroundStarRotation += 10 * GetFrameTime();

            if (player.hp <= 0)
            {
                gameState = GAME_OVER;
                StopMusicStream(gameMusic);
            }

        } break;
        case GAME_OVER:
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                gameState = TITLE;
            }
        } break;
    }
}

void DrawGame(void)
{
    BeginDrawing();
    
    Vector2 shakeOffset = {0};
    if (screenShakeIntensity > 0)
    {
        shakeOffset.x = GetRandomValue(-screenShakeIntensity, screenShakeIntensity);
        shakeOffset.y = GetRandomValue(-screenShakeIntensity, screenShakeIntensity);
    }
    
    BeginMode2D((Camera2D){0,0, (Vector2){shakeOffset.x, shakeOffset.y}, 0, 1.0f});

    ClearBackground(BLACK);

    if (gameState != GAME_OVER) {
        DrawTexturePro(texBackgroundStar, (Rectangle){0,0, (float)texBackgroundStar.width, (float)texBackgroundStar.height},
                       (Rectangle){SCREEN_WIDTH/2, SCREEN_HEIGHT/2, (float)texBackgroundStar.width * 2, (float)texBackgroundStar.height * 2},
                       (Vector2){(float)texBackgroundStar.width, (float)texBackgroundStar.height},
                       backgroundStarRotation, Fade(WHITE, 0.2f));
    }

    switch(gameState)
    {
        case TITLE:
        {
            DrawText("ASPIRING DEITY", SCREEN_WIDTH/2 - MeasureText("ASPIRING DEITY", 40)/2, SCREEN_HEIGHT/2 - 80, 40, WHITE);
            DrawText("Pressione ENTER para começar", SCREEN_WIDTH/2 - MeasureText("Pressione ENTER para começar", 20)/2, SCREEN_HEIGHT/2, 20, GRAY);
        } break;
        case GAMEPLAY:
        {
            DrawRectangleLinesEx(arena, 3, WHITE);

            switch(currentAttack)
            {
                case STAR_BLAZING: DrawStarBlazing(); break;
                case SHOCKER_BREAKER: DrawShockerBreaker(); break;
                case CHAOS_SABER: DrawChaosSaber(); break;
                case HYPER_GONER: DrawHyperGoner(); break;
                default: break;
            }
            
            Color playerColor = RED;
            if (player.invincibilityTimer > 0)
            {
                if (fmod(player.invincibilityTimer, 0.2f) > 0.1f) playerColor = BLANK;
                else playerColor = RED;
            } else if (player.isDamaged) {
                 playerColor = BLANK;
            }

            DrawTexturePro(texHeart, (Rectangle){0,0, (float)texHeart.width, (float)texHeart.height},
                           (Rectangle){player.position.x, player.position.y, (float)texHeart.width, (float)texHeart.height},
                           (Vector2){(float)texHeart.width/2, (float)texHeart.height/2},
                           0, playerColor);

            DrawParticles();

            DrawText(TextFormat("HP: %02i/%02i", player.hp, PLAYER_INITIAL_HP), 20, 20, 30, WHITE);
            DrawText("SOBREVIVA!", SCREEN_WIDTH/2 - MeasureText("SOBREVIVA!", 20)/2, 20, 20, YELLOW);
            
            const char *attackName = "";
            switch(currentAttack) {
                case STAR_BLAZING: attackName = "Star Blazing"; break;
                case SHOCKER_BREAKER: attackName = "Shocker Breaker"; break;
                case CHAOS_SABER: attackName = "Chaos Saber"; break;
                case HYPER_GONER: attackName = "Hyper Goner"; break;
                default: break;
            }
            DrawText(attackName, SCREEN_WIDTH - MeasureText(attackName, 20) - 20, 20, 20, RAYWHITE);
        } break;
        case GAME_OVER:
        {
            DrawText("GAME OVER", SCREEN_WIDTH/2 - MeasureText("GAME OVER", 60)/2, SCREEN_HEIGHT/2 - 60, 60, RED);
            DrawText("Mas se recusou.", SCREEN_WIDTH/2 - MeasureText("Mas se recusou.", 20)/2, SCREEN_HEIGHT/2 + 20, 20, WHITE);
            DrawText("Pressione ENTER para tentar novamente.", SCREEN_WIDTH/2 - MeasureText("Pressione ENTER para tentar novamente.", 20)/2, SCREEN_HEIGHT - 50, 20, GRAY);
        } break;
    }

    if (screenFlashTimer > 0)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(WHITE, screenFlashTimer / 0.1f));
    }

    EndMode2D();
    EndDrawing();
}

void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}

//----------------------------------------------------------------------------------
// Funções de Criação de Texturas
//----------------------------------------------------------------------------------

Texture2D CreateHeartTexture(void) {
    Image heart = GenImageColor(16, 16, BLANK);
    int heartShape[16][16] = {
        {0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0},{0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0},
        {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},{0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},{0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
        {0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0},{0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
        {0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0},{0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0},
        {0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0},{0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0}
    };
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (heartShape[y][x] == 1) ImageDrawPixel(&heart, x, y, RED);
        }
    }
    Texture2D texture = LoadTextureFromImage(heart);
    UnloadImage(heart);
    return texture;
}

Texture2D CreateStarTexture(void) {
    Image star = GenImageColor(24, 24, BLANK);
    int cx = 12, cy = 12;
    int outerR = 11, innerR = 5;
    Vector2 points[10];
    for (int i = 0; i < 5; i++) {
        points[i * 2].x = cx + outerR * sinf(DEG2RAD * (i * 72));
        points[i * 2].y = cy - outerR * cosf(DEG2RAD * (i * 72));
        points[i * 2 + 1].x = cx + innerR * sinf(DEG2RAD * (i * 72 + 36));
        points[i * 2 + 1].y = cy - innerR * cosf(DEG2RAD * (i * 72 + 36));
    }
    for (int i = 0; i < 5; i++) {
        ImageDrawTriangle(&star, points[i*2], points[(i*2 + 9) % 10], (Vector2){cx,cy}, GOLD);
        ImageDrawTriangle(&star, points[i*2], points[(i*2 + 1) % 10], (Vector2){cx,cy}, GOLD);
    }
    ImageDrawTriangle(&star, points[1], points[3], points[5], YELLOW);
    ImageDrawTriangle(&star, points[5], points[7], points[1], YELLOW);
    ImageDrawTriangle(&star, points[7], points[9], points[1], YELLOW);
    Texture2D texture = LoadTextureFromImage(star);
    UnloadImage(star);
    return texture;
}

Texture2D CreateLightningWarningTexture(void) {
    Image img = GenImageColor(20, ARENA_HEIGHT, BLANK);
    ImageDrawRectangle(&img, 0, 0, 20, ARENA_HEIGHT, Fade(YELLOW, 0.5f));
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

Texture2D CreateLightningStrikeTexture(void) {
    Image img = GenImageColor(20, ARENA_HEIGHT, BLANK);
    ImageDrawRectangle(&img, 0, 0, 20, ARENA_HEIGHT, WHITE);
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

// ***** FUNÇÃO CORRIGIDA DEFINITIVAMENTE *****
Texture2D CreateChaosSaberTexture(void) {
    Image saber = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT / 2, BLANK);
    
    Color startColor = {0, 121, 241, 150};
    Color endColor = {41, 178, 255, 150};

    for (int y = 0; y < saber.height; y++) {
        float amount = (float)y / (float)saber.height;
        // Interpolação linear manual para cada componente de cor
        unsigned char r = (unsigned char)(startColor.r + (endColor.r - startColor.r) * amount);
        unsigned char g = (unsigned char)(startColor.g + (endColor.g - startColor.g) * amount);
        unsigned char b = (unsigned char)(startColor.b + (endColor.b - startColor.b) * amount);
        unsigned char a = (unsigned char)(startColor.a + (endColor.a - startColor.a) * amount);
        Color rowColor = { r, g, b, a };

        for (int x = 0; x < saber.width; x++) {
            ImageDrawPixel(&saber, x, y, rowColor);
        }
    }
    
    // O fio da lâmina continua o mesmo, pois ImageDrawRectangle é básico
    ImageDrawRectangle(&saber, 0, saber.height / 2 - 5, saber.width, 10, Fade(WHITE, 0.7f));
    
    Texture2D texture = LoadTextureFromImage(saber);
    UnloadImage(saber);
    return texture;
}

Texture2D CreateHyperGonerMouthTexture(void) {
    Image img = GenImageColor(200, 100, BLANK);
    int cx = 100; int cy = 50;
    int rx_outer = 100; int ry_outer = 50;
    int rx_inner = 80; int ry_inner = 40;
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            if (((float)(x-cx)*(x-cx))/(rx_outer*rx_outer) + ((float)(y-cy)*(y-cy))/(ry_outer*ry_outer) <= 1) {
                ImageDrawPixel(&img, x, y, GRAY);
            }
        }
    }
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            if (((float)(x-cx)*(x-cx))/(rx_inner*rx_inner) + ((float)(y-cy)*(y-cy))/(ry_inner*ry_inner) <= 1) {
                ImageDrawPixel(&img, x, y, BLACK);
            }
        }
    }
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

Texture2D CreateHyperGonerChargeTexture(void) {
    Image img = GenImageColor(100, 100, BLANK);
    for (int i = 0; i < 50; i++) {
        ImageDrawCircle(&img, 50, 50, i, Fade(BLUE, (float)i/50.0f));
    }
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

Texture2D CreateHyperGonerBeamTexture(void) {
    Image img = GenImageColor(SCREEN_WIDTH, 50, BLANK);
    // Usando o mesmo método de gradiente manual
    Color startColor = Fade(SKYBLUE, 0.5f);
    Color endColor = WHITE;
    for (int y = 0; y < img.height; y++) {
        float amount = (float)y / (float)img.height;
        unsigned char r = (unsigned char)(startColor.r + (endColor.r - startColor.r) * amount);
        unsigned char g = (unsigned char)(startColor.g + (endColor.g - startColor.g) * amount);
        unsigned char b = (unsigned char)(startColor.b + (endColor.b - startColor.b) * amount);
        unsigned char a = (unsigned char)(startColor.a + (endColor.a - startColor.a) * amount);
        Color rowColor = { r, g, b, a };
        for (int x = 0; x < img.width; x++) {
            ImageDrawPixel(&img, x, y, rowColor);
        }
    }
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

Texture2D CreateBackgroundStarTexture(void) {
    Image img = GenImageColor(100, 100, BLANK);
    ImageDrawCircle(&img, 50, 50, 45, Fade(BLUE, 0.1f));
    ImageDrawCircle(&img, 50, 50, 30, Fade(SKYBLUE, 0.15f));
    ImageDrawCircle(&img, 50, 50, 15, Fade(WHITE, 0.2f));
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

//----------------------------------------------------------------------------------
// Funções de Partículas
//----------------------------------------------------------------------------------

void EmitParticle(Vector2 pos, Vector2 speed, Color color, float radius, float lifetime) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].position = pos;
            particles[i].speed = speed;
            particles[i].color = color;
            particles[i].radius = radius;
            particles[i].lifetime = lifetime;
            particles[i].active = true;
            break;
        }
    }
}

void UpdateParticles(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].position.x += particles[i].speed.x * GetFrameTime();
            particles[i].position.y += particles[i].speed.y * GetFrameTime();
            particles[i].lifetime -= GetFrameTime();
            if (particles[i].lifetime <= 0) {
                particles[i].active = false;
            }
        }
    }
}

void DrawParticles(void) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            DrawCircleV(particles[i].position, particles[i].radius * (particles[i].lifetime / 1.0f), particles[i].color);
        }
    }
}

//----------------------------------------------------------------------------------
// Funções de Ataque
//----------------------------------------------------------------------------------

void InitAttack(AttackType newAttack)
{
    for (int i = 0; i < MAX_STARS; i++) stars[i].active = false;
    for (int i = 0; i < MAX_LIGHTNINGS; i++) lightnings[i].active = false;
    saberPhase = 0;
    gonerActive = false;
    gonerCharging = false;
    gonerBeaming = false;

    currentAttack = newAttack;
    attackTimer = 0.0f;
    attackDuration = 10.0f + GetRandomValue(-2, 2);

    if (currentAttack == HYPER_GONER) {
        gonerActive = true;
        gonerCharging = true;
        gonerChargeTimer = 3.0f;
        gonerBeamTimer = 0.5f;
        gonerPosition = (Vector2){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
        attackDuration = 5.0f;
    }
}

void UpdateStarBlazing(void)
{
    if (fmod(attackTimer, 0.1f) < GetFrameTime() * 1.5f) {
        for (int k = 0; k < 3; k++) {
            for (int i = 0; i < MAX_STARS; i++)
            {
                if (!stars[i].active)
                {
                    int side = GetRandomValue(0, 3);
                    if (side == 0) { stars[i].position = (Vector2){ GetRandomValue(0, SCREEN_WIDTH), -50 }; }
                    else if (side == 1) { stars[i].position = (Vector2){ GetRandomValue(0, SCREEN_WIDTH), SCREEN_HEIGHT + 50 }; }
                    else if (side == 2) { stars[i].position = (Vector2){ -50, GetRandomValue(0, SCREEN_HEIGHT) }; }
                    else { stars[i].position = (Vector2){ SCREEN_WIDTH + 50, GetRandomValue(0, SCREEN_HEIGHT) }; }
                    stars[i].radius = 12;

                    Vector2 target = (Vector2){ arena.x + arena.width / 2 + GetRandomValue(-50, 50), arena.y + arena.height / 2 + GetRandomValue(-50, 50) };
                    Vector2 direction = { target.x - stars[i].position.x, target.y - stars[i].position.y };
                    float length = sqrt(direction.x*direction.x + direction.y*direction.y);
                    if (length == 0) length = 1;
                    float speed = GetRandomValue(200, 400);
                    stars[i].speed = (Vector2){ (direction.x / length) * speed, (direction.y / length) * speed };
                    stars[i].color = (Color){ GetRandomValue(100, 255), GetRandomValue(100, 255), GetRandomValue(100, 255), 255 };
                    stars[i].rotation = GetRandomValue(0, 360);
                    stars[i].active = true;
                    break;
                }
            }
        }
    }

    Rectangle playerRect = { player.position.x - texHeart.width/2, player.position.y - texHeart.height/2, (float)texHeart.width, (float)texHeart.height };
    for (int i = 0; i < MAX_STARS; i++)
    {
        if (stars[i].active)
        {
            stars[i].position.x += stars[i].speed.x * GetFrameTime();
            stars[i].position.y += stars[i].speed.y * GetFrameTime();
            stars[i].rotation += 300 * GetFrameTime();
            
            if (CheckCollisionCircleRec(stars[i].position, stars[i].radius, playerRect) && player.invincibilityTimer <= 0)
            {
                player.hp--;
                player.invincibilityTimer = 1.0f;
                player.isDamaged = true;
                screenShakeIntensity = 5.0f;
            }
            if (stars[i].position.x < -100 || stars[i].position.x > SCREEN_WIDTH + 100 || stars[i].position.y < -100 || stars[i].position.y > SCREEN_HEIGHT + 100)
            {
                stars[i].active = false;
            }
        }
    }
}

void DrawStarBlazing(void)
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        if (stars[i].active)
        {
            DrawTexturePro(texStar, (Rectangle){0,0, (float)texStar.width, (float)texStar.height},
                           (Rectangle){stars[i].position.x, stars[i].position.y, stars[i].radius*2, stars[i].radius*2},
                           (Vector2){stars[i].radius, stars[i].radius},
                           stars[i].rotation, stars[i].color);
        }
    }
}

void UpdateShockerBreaker(void)
{
    static float spawnTimer = 0.0f;
    spawnTimer += GetFrameTime();

    if (spawnTimer >= 0.5f) {
        for (int i = 0; i < MAX_LIGHTNINGS; i++)
        {
            if (!lightnings[i].active)
            {
                lightnings[i].active = true;
                lightnings[i].state = 1;
                lightnings[i].timer = 0.8f;
                lightnings[i].warningRect = (Rectangle){ arena.x + GetRandomValue(0, arena.width - 20), arena.y, 20, arena.height };
                lightnings[i].color = (Color){ GetRandomValue(0,255), GetRandomValue(0,255), 255, 255 };
                spawnTimer = 0.0f;
                break;
            }
        }
    }

    Rectangle playerRect = { player.position.x - texHeart.width/2, player.position.y - texHeart.height/2, (float)texHeart.width, (float)texHeart.height };
    for (int i = 0; i < MAX_LIGHTNINGS; i++)
    {
        if (lightnings[i].active)
        {
            lightnings[i].timer -= GetFrameTime();
            if (lightnings[i].timer <= 0)
            {
                if (lightnings[i].state == 1)
                {
                    lightnings[i].state = 2;
                    lightnings[i].timer = 0.2f;
                    lightnings[i].strikeRect = lightnings[i].warningRect;
                    screenShakeIntensity = 7.0f;
                }
                else if (lightnings[i].state == 2)
                {
                    lightnings[i].active = false;
                }
            }

            if (lightnings[i].state == 2 && CheckCollisionRecs(playerRect, lightnings[i].strikeRect) && player.invincibilityTimer <= 0)
            {
                player.hp--;
                player.invincibilityTimer = 1.0f;
                player.isDamaged = true;
                screenShakeIntensity = 10.0f;
            }
        }
    }
}

void DrawShockerBreaker(void)
{
    for (int i = 0; i < MAX_LIGHTNINGS; i++)
    {
        if (lightnings[i].active)
        {
            if (lightnings[i].state == 1)
            {
                DrawTexturePro(texLightningWarning, (Rectangle){0,0, (float)texLightningWarning.width, (float)texLightningWarning.height},
                               lightnings[i].warningRect, (Vector2){0,0}, 0, Fade(lightnings[i].color, lightnings[i].timer / 0.8f));
            }
            else if (lightnings[i].state == 2)
            {
                DrawTexturePro(texLightningStrike, (Rectangle){0,0, (float)texLightningStrike.width, (float)texLightningStrike.height},
                               lightnings[i].strikeRect, (Vector2){0,0}, 0, WHITE);
            }
        }
    }
}

void UpdateChaosSaber(void)
{
    float saberSpeed = 1000.0f;
    
    if (attackTimer > 0.8f && saberPhase == 0) {
        chaosSabers[0] = (Rectangle){ -SCREEN_WIDTH, arena.y - 50, SCREEN_WIDTH + arena.width, arena.height / 2 + 50 };
        chaosSabers[1] = (Rectangle){ SCREEN_WIDTH, arena.y + arena.height / 2, SCREEN_WIDTH + arena.width, arena.height / 2 + 50 };
        saberPhase = 1;
        screenFlashTimer = 0.1f;
    } else if (attackTimer > 3.0f && saberPhase == 1) {
        chaosSabers[0] = (Rectangle){ -SCREEN_WIDTH, arena.y + arena.height / 2, SCREEN_WIDTH + arena.width, arena.height / 2 + 50 };
        chaosSabers[1] = (Rectangle){ SCREEN_WIDTH, arena.y - 50, SCREEN_WIDTH + arena.width, arena.height / 2 + 50 };
        saberPhase = 2;
        screenFlashTimer = 0.1f;
    } else if (attackTimer > 5.0f && saberPhase == 2) {
        chaosSabers[0] = (Rectangle){ -SCREEN_WIDTH, arena.y + arena.height / 4, SCREEN_WIDTH*2, arena.height/2 };
        chaosSabers[1] = chaosSabers[0];
        saberPhase = 3;
        screenFlashTimer = 0.1f;
    }

    if (saberPhase == 1 || saberPhase == 2) {
        chaosSabers[0].x += saberSpeed * GetFrameTime();
        chaosSabers[1].x -= saberSpeed * GetFrameTime();
        if (chaosSabers[0].x > arena.x - arena.width) screenShakeIntensity = 10.0f;
    } else if (saberPhase == 3) {
        chaosSabers[0].x += saberSpeed * 1.5f * GetFrameTime();
        if (chaosSabers[0].x > 0) screenShakeIntensity = 10.0f;
    }
    
    Rectangle playerRect = { player.position.x - texHeart.width/2, player.position.y - texHeart.height/2, (float)texHeart.width, (float)texHeart.height };
    if (saberPhase >= 1 && saberPhase <= 3 && player.invincibilityTimer <= 0)
    {
        if (CheckCollisionRecs(playerRect, chaosSabers[0]) || (saberPhase != 3 && CheckCollisionRecs(playerRect, chaosSabers[1])))
        {
            player.hp--;
            player.invincibilityTimer = 1.0f;
            player.isDamaged = true;
            screenShakeIntensity = 15.0f;
        }
    }
}

void DrawChaosSaber(void)
{
    if (saberPhase >= 1 && saberPhase <= 3)
    {
        if(saberPhase == 1 || saberPhase == 2) {
            DrawTexturePro(texChaosSaber, (Rectangle){0,0, (float)texChaosSaber.width, (float)texChaosSaber.height},
                           chaosSabers[0], (Vector2){0,0}, 0, WHITE);
            DrawTexturePro(texChaosSaber, (Rectangle){0,0, (float)texChaosSaber.width, (float)texChaosSaber.height},
                           chaosSabers[1], (Vector2){0,0}, 0, WHITE);
        } else {
             DrawTexturePro(texChaosSaber, (Rectangle){0,0, (float)texChaosSaber.width, (float)texChaosSaber.height},
                           chaosSabers[0], (Vector2){0,0}, 0, WHITE);
        }
    }
}

void UpdateHyperGoner(void) {
    gonerRotation += 50 * GetFrameTime();

    if (gonerCharging) {
        gonerChargeTimer -= GetFrameTime();
        if (gonerChargeTimer <= 0) {
            gonerCharging = false;
            gonerBeaming = true;
            screenFlashTimer = 0.2f;
            screenShakeIntensity = 20.0f;
            player.invincibilityTimer = 0; 
        }
    }

    if (gonerBeaming) {
        gonerBeamTimer -= GetFrameTime();
        
        Rectangle playerRect = { player.position.x - texHeart.width/2, player.position.y - texHeart.height/2, (float)texHeart.width, (float)texHeart.height };
        Rectangle beamRect = { gonerPosition.x, gonerPosition.y - 25, SCREEN_WIDTH - gonerPosition.x, 50 };

        if (CheckCollisionRecs(playerRect, beamRect) && player.invincibilityTimer <= 0) {
            player.hp -= 5;
            player.invincibilityTimer = 1.0f;
            player.isDamaged = true;
            screenShakeIntensity = 30.0f;
        }

        if (gonerBeamTimer <= 0) {
            gonerBeaming = false;
            gonerActive = false;
        }
    }
}

void DrawHyperGoner(void) {
    if (gonerActive) {
        if (gonerCharging) {
            float scale = 1.0f + (1.0f - gonerChargeTimer / 3.0f) * 2.0f;
            DrawTexturePro(texHyperGonerCharge, (Rectangle){0,0, (float)texHyperGonerCharge.width, (float)texHyperGonerCharge.height},
                           (Rectangle){gonerPosition.x, gonerPosition.y, (float)texHyperGonerCharge.width * scale, (float)texHyperGonerCharge.height * scale},
                           (Vector2){(float)texHyperGonerCharge.width * scale/2, (float)texHyperGonerCharge.height * scale/2},
                           gonerRotation, Fade(WHITE, (1.0f - gonerChargeTimer / 3.0f) * 0.8f + 0.2f));
        }

        DrawTexturePro(texHyperGonerMouth, (Rectangle){0,0, (float)texHyperGonerMouth.width, (float)texHyperGonerMouth.height},
                       (Rectangle){gonerPosition.x, gonerPosition.y, (float)texHyperGonerMouth.width, (float)texHyperGonerMouth.height},
                       (Vector2){(float)texHyperGonerMouth.width/2, (float)texHyperGonerMouth.height/2},
                       gonerRotation, WHITE);

        if (gonerBeaming) {
            DrawTexturePro(texHyperGonerBeam, (Rectangle){0,0, (float)texHyperGonerBeam.width, (float)texHyperGonerBeam.height},
                           (Rectangle){gonerPosition.x, gonerPosition.y, SCREEN_WIDTH - gonerPosition.x, (float)texHyperGonerBeam.height},
                           (Vector2){0, (float)texHyperGonerBeam.height/2},
                           0, WHITE);
            DrawRectangle(gonerPosition.x, gonerPosition.y - 40, SCREEN_WIDTH - gonerPosition.x, 80, Fade(SKYBLUE, 0.2f));
        }
    }
}
