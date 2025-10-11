// main.c - Base de Jogo Estilo Cuphead (CORREÇÃO FINAL da Hitbox e Disparos)

#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Funções de Vetores (salvaguarda)
#ifndef VECTOR2_FUNCTIONS_DEFINED
#define VECTOR2_FUNCTIONS_DEFINED
static float Vector2Distance(Vector2 v1, Vector2 v2) { return sqrtf((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y)); }
static Vector2 Vector2Subtract(Vector2 v1, Vector2 v2) { return (Vector2){ v1.x - v2.x, v1.y - v2.y }; }
static Vector2 Vector2Add(Vector2 v1, Vector2 v2) { return (Vector2){ v1.x + v2.x, v1.y + v2.y }; }
static Vector2 Vector2Scale(Vector2 v, float scale) { return (Vector2){ v.x * scale, v.y * scale }; }
static Vector2 Vector2Normalize(Vector2 v) { float length = sqrtf(v.x*v.x + v.y*v.y); if (length > 0) { float ilength = 1.0f/length; v.x *= ilength; v.y *= ilength; } return v; }
#endif

// --- DEFINIÇÕES GLOBAIS ---
#define PLAYER_HEALTH 10
#define PLAYER_HITBOX_WIDTH 30.0f
#define PLAYER_HITBOX_HEIGHT 60.0f
#define BOSS_HEALTH 400 
#define BOSS_RADIUS 80.0f
#define MAX_BULLETS 250
#define PLAYER_SCALE 0.15f

// --- ESTRUTURAS DE DADOS ---
typedef struct Player {
    Rectangle hitbox; int health;
    float dashCooldown;
    bool hasTripleShot; float tripleShotTimer;
    int shootCooldown;
    bool isDashing;
    int dashTimer;
    Vector2 dashDirection;
    bool isInvulnerable;
    int invulnerabilityTimer;
    int superMeter;
    bool isParryInvulnerable;
} Player;

typedef struct Boss { Vector2 position; int health; float radius; Color color; float speedY; int phase; } Boss;
typedef struct PowerUp { Vector2 position; bool active; float radius; Color color; } PowerUp;
typedef enum BulletType { NORMAL, SINE_WAVE, HOMING } BulletType;

typedef struct Bullet {
    Vector2 position; Vector2 speed; bool active; Color color; float radius;
    BulletType type; float originY; float waveAngle;
    int lifeTimer;
    bool isParryable;
} Bullet;

typedef enum BossAttackPattern { AIMED_SHOT, SPREAD_SHOT, BURST_SHOT, SINE_WAVE_SHOT, SPIRAL_SHOT, HOMING_SHOT } BossAttackPattern;
#define TOTAL_ATTACK_PATTERNS 6

int main(void)
{
    // --- 1. INICIALIZAÇÃO ---
    const int screenWidth = 1280;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Batalha Contra o Chefe - Versão Estável!");
    srand(time(NULL));

    Texture2D background = LoadTexture("background.png");
    Texture2D playerTexture = LoadTexture("personagem.png");

    Player player = { 0 };
    player.hitbox.x = 100;
    player.hitbox.y = (float)screenHeight / 2 - (PLAYER_HITBOX_HEIGHT / 2);
    player.hitbox.width = PLAYER_HITBOX_WIDTH;
    player.hitbox.height = PLAYER_HITBOX_HEIGHT;
    player.health = PLAYER_HEALTH;
    player.dashCooldown = 0.0f;
    player.hasTripleShot = false;
    player.tripleShotTimer = 0.0f;
    player.shootCooldown = 0;
    player.isDashing = false;
    player.dashTimer = 0;
    player.dashDirection = (Vector2){0,0};
    player.isInvulnerable = false;
    player.invulnerabilityTimer = 0;
    player.superMeter = 0;
    player.isParryInvulnerable = false;

    Boss boss = { 0 };
    boss.position = (Vector2){ screenWidth - BOSS_RADIUS - 20, (float)screenHeight / 2 };
    boss.health = BOSS_HEALTH;
    boss.radius = BOSS_RADIUS;
    boss.color = RED;
    boss.speedY = 2.0f;
    boss.phase = 1;

    PowerUp tripleShotPowerUp = { 0 };
    tripleShotPowerUp.active = false;
    tripleShotPowerUp.radius = 15.0f;
    tripleShotPowerUp.color = GOLD;
    float powerUpSpawnTimer = 600;

    Bullet playerBullets[MAX_BULLETS] = { 0 };
    Bullet bossBullets[MAX_BULLETS] = { 0 };
    
    bool superAttackActive = false;
    int superAttackTimer = 0;
    Rectangle superBeam = { 0 };
    
    BossAttackPattern currentPattern = AIMED_SHOT;
    int patternTimer = 0; int attackTimer = 0; bool bossIsAttacking = false;
    int shotsFired = 0; float spiralAngle = 0.0f;
    
    bool gameOver = false; bool victory = false;
    SetTargetFPS(60);

    // --- 2. O LOOP PRINCIPAL ---
    while (!WindowShouldClose())
    {
        // --- 2.1. INPUT ---
        if (!gameOver && !victory)
        {
            if (!player.isDashing)
            {
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.hitbox.x += 5.0f;
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.hitbox.x -= 5.0f;
                if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) player.hitbox.y += 5.0f;
                if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) player.hitbox.y -= 5.0f;
            }

            if (IsKeyPressed(KEY_LEFT_SHIFT) && player.dashCooldown <= 0)
            {
                player.isDashing = true;
                player.dashTimer = 15;
                player.dashCooldown = 120;
                player.isParryInvulnerable = false;
                player.dashDirection = (Vector2){ 0, 0 };
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.dashDirection.x += 1;
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.dashDirection.x -= 1;
                if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) player.dashDirection.y += 1;
                if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) player.dashDirection.y -= 1;
                if (player.dashDirection.x == 0 && player.dashDirection.y == 0) player.dashDirection.x = 1;
            }

            if ((IsKeyDown(KEY_SPACE)) && (player.shootCooldown <= 0))
            {
                player.shootCooldown = 10;
                Vector2 playerCenter = { player.hitbox.x + player.hitbox.width / 2, player.hitbox.y + player.hitbox.height / 2 };

                if (player.hasTripleShot)
                {
                    player.shootCooldown = 15;
                    for (int j = 0; j < 3; j++) {
                        for (int i = 0; i < MAX_BULLETS; i++) if (!playerBullets[i].active) {
                            playerBullets[i].active = true;
                            playerBullets[i].position = playerCenter;
                            playerBullets[i].color = GOLD;
                            playerBullets[i].radius = 8.0f;
                            playerBullets[i].type = NORMAL;
                            float angleOffset = (j == 1) ? -0.2f : (j == 2) ? 0.2f : 0.0f;
                            playerBullets[i].speed.x = cosf(angleOffset) * 12.0f;
                            playerBullets[i].speed.y = sinf(angleOffset) * 12.0f;
                            break;
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < MAX_BULLETS; i++) if (!playerBullets[i].active) {
                        playerBullets[i].active = true;
                        playerBullets[i].position = playerCenter;
                        playerBullets[i].speed = (Vector2){ 12.0f, 0 };
                        playerBullets[i].color = SKYBLUE;
                        playerBullets[i].radius = 10.0f;
                        playerBullets[i].type = NORMAL;
                        break;
                    }
                }
            }
            
            if (IsKeyPressed(KEY_E) && player.superMeter == 4 && !superAttackActive)
            {
                superAttackActive = true;
                superAttackTimer = 90;
                player.superMeter = 0;
            }
        }

        // --- 2.2. UPDATE ---
        if (!gameOver && !victory)
        {
            if (player.isDashing)
            {
                player.dashTimer--;
                float dashSpeed = 20.0f;
                player.hitbox.x += player.dashDirection.x * dashSpeed;
                player.hitbox.y += player.dashDirection.y * dashSpeed;
                if (player.dashTimer <= 0) player.isDashing = false;
            }
            
            if (player.dashCooldown > 0) player.dashCooldown--;
            if (player.shootCooldown > 0) player.shootCooldown--;
            if (player.tripleShotTimer > 0) player.tripleShotTimer--; else player.hasTripleShot = false;

            if (player.isInvulnerable)
            {
                player.invulnerabilityTimer--;
                if (player.invulnerabilityTimer <= 0)
                {
                    player.isInvulnerable = false;
                }
            }
            
            if (superAttackActive)
            {
                Vector2 playerCenter = { player.hitbox.x + player.hitbox.width / 2, player.hitbox.y + player.hitbox.height / 2 };
                superAttackTimer--;
                superBeam = (Rectangle){ playerCenter.x, playerCenter.y - 15, (float)screenWidth, 30 };
                if (CheckCollisionCircleRec(boss.position, boss.radius, superBeam))
                {
                    if (superAttackTimer % 3 == 0) boss.health--;
                }
                if (superAttackTimer <= 0) superAttackActive = false;
            }

            boss.position.y += boss.speedY;
            if (boss.position.y - boss.radius <= 0 || boss.position.y + boss.radius >= screenHeight) boss.speedY *= -1;
            
            if (boss.health <= (BOSS_HEALTH / 2) && boss.phase == 1){ boss.phase = 2; boss.color = PURPLE; boss.speedY *= 1.5f; }

            if (!tripleShotPowerUp.active){ powerUpSpawnTimer--; if (powerUpSpawnTimer <= 0){ tripleShotPowerUp.active = true; tripleShotPowerUp.position = (Vector2){ rand() % (screenWidth / 2), rand() % screenHeight }; powerUpSpawnTimer = 1200;}}
            else { if (CheckCollisionCircleRec(tripleShotPowerUp.position, tripleShotPowerUp.radius, player.hitbox)){ tripleShotPowerUp.active = false; player.hasTripleShot = true; player.tripleShotTimer = 480;}}
            
            for (int i = 0; i < MAX_BULLETS; i++) if (playerBullets[i].active) {
                playerBullets[i].position.x += playerBullets[i].speed.x;
                playerBullets[i].position.y += playerBullets[i].speed.y;
                if (playerBullets[i].position.x > screenWidth) playerBullets[i].active = false;
                if (CheckCollisionCircles(playerBullets[i].position, playerBullets[i].radius, boss.position, boss.radius)) {
                    boss.health--;
                    playerBullets[i].active = false;
                }
            }
            
            if (player.hitbox.x < 0) player.hitbox.x = 0;
            if (player.hitbox.x + player.hitbox.width > screenWidth) player.hitbox.x = screenWidth - player.hitbox.width;
            if (player.hitbox.y < 0) player.hitbox.y = 0;
            if (player.hitbox.y + player.hitbox.height > screenHeight) player.hitbox.y = screenHeight - player.hitbox.height;

            if (CheckCollisionCircleRec(boss.position, boss.radius, player.hitbox)){
                Vector2 playerCenter = {player.hitbox.x + player.hitbox.width / 2, player.hitbox.y + player.hitbox.height / 2};
                float distance = Vector2Distance(playerCenter, boss.position);
                if (distance < (PLAYER_HITBOX_WIDTH / 2 + boss.radius)){ // Aproximação da colisão
                    float overlap = (PLAYER_HITBOX_WIDTH / 2 + boss.radius) - distance;
                    Vector2 normal = Vector2Normalize(Vector2Subtract(playerCenter, boss.position));
                    player.hitbox.x += normal.x * overlap;
                    player.hitbox.y += normal.y * overlap;
                }
                if (!player.isInvulnerable) {
                    player.health--;
                    player.isInvulnerable = true;
                    player.invulnerabilityTimer = 120;
                }
            }

            patternTimer++;
            if (patternTimer >= (boss.phase == 1 ? 240 : 150) && !bossIsAttacking){patternTimer = 0; BossAttackPattern oldPattern = currentPattern; do { if (boss.phase == 1) { currentPattern = rand() % 4; } else { currentPattern = (rand() % 3) + 3; } } while (currentPattern == oldPattern);}
            attackTimer++;
            if (!bossIsAttacking){
                switch (currentPattern){
                    case AIMED_SHOT: if (attackTimer >= 60) { attackTimer=0; for (int i=0; i<MAX_BULLETS; i++) if (!bossBullets[i].active) { bossBullets[i].active=true; bossBullets[i].position=boss.position; bossBullets[i].color=ORANGE; bossBullets[i].radius=12.0f; bossBullets[i].type=NORMAL; bossBullets[i].isParryable = false; float angle=atan2f(player.hitbox.y - boss.position.y, player.hitbox.x - boss.position.x); bossBullets[i].speed.x=cosf(angle)*7.0f; bossBullets[i].speed.y=sinf(angle)*7.0f; break; } } break;
                    case SPREAD_SHOT: if (attackTimer >= 80) { attackTimer=0; int n=3; for (int i=0; i<MAX_BULLETS && n>0; i++) if (!bossBullets[i].active) { bossBullets[i].active=true; bossBullets[i].position=boss.position; bossBullets[i].color=RAYWHITE; bossBullets[i].radius=8.0f; bossBullets[i].type=NORMAL; bossBullets[i].isParryable = false; float o=(n==2)?-0.4f:(n==1)?0.4f:0.0f; float a=PI+o; bossBullets[i].speed.x=cosf(a)*6.0f; bossBullets[i].speed.y=sinf(a)*6.0f; n--; } } break;
                    case BURST_SHOT: case SINE_WAVE_SHOT: case SPIRAL_SHOT: case HOMING_SHOT: if (attackTimer >= 90) { attackTimer=0; bossIsAttacking=true; shotsFired=0; } break;
                }
            } else {
                attackTimer++;
                switch (currentPattern){
                    case BURST_SHOT: if (attackTimer % 8 == 0 && shotsFired < 4) { for (int i=0; i<MAX_BULLETS; i++) if (!bossBullets[i].active) { bossBullets[i].active=true; bossBullets[i].position=boss.position; bossBullets[i].color=YELLOW; bossBullets[i].radius=10.0f; bossBullets[i].type=NORMAL; bossBullets[i].isParryable = false; float a=atan2f(player.hitbox.y - boss.position.y, player.hitbox.x - boss.position.x); bossBullets[i].speed.x=cosf(a)*8.0f; bossBullets[i].speed.y=sinf(a)*8.0f; shotsFired++; break; } } if (shotsFired>=4) bossIsAttacking=false; break;
                    case SINE_WAVE_SHOT: if (attackTimer % 5 == 0 && shotsFired < 30) { for (int i=0; i<MAX_BULLETS; i++) if (!bossBullets[i].active) { bossBullets[i].active=true; bossBullets[i].position=boss.position; bossBullets[i].color=LIME; bossBullets[i].radius=7.0f; bossBullets[i].type=SINE_WAVE; bossBullets[i].isParryable = false; bossBullets[i].speed=(Vector2){-5.0f,0}; bossBullets[i].originY=boss.position.y; bossBullets[i].waveAngle=shotsFired*0.5f; shotsFired++; break; } } if (shotsFired>=30) bossIsAttacking=false; break;
                    case SPIRAL_SHOT: if (attackTimer % 4 == 0 && shotsFired < 60) { for (int i=0; i<MAX_BULLETS; i++) if (!bossBullets[i].active) { bossBullets[i].active=true; bossBullets[i].position=boss.position; bossBullets[i].color=VIOLET; bossBullets[i].radius=6.0f; bossBullets[i].type=NORMAL; bossBullets[i].isParryable = false; spiralAngle+=0.4f; bossBullets[i].speed.x=cosf(spiralAngle)*5.0f; bossBullets[i].speed.y=sinf(spiralAngle)*5.0f; shotsFired++; break; } } if (shotsFired>=60) bossIsAttacking=false; break;
                    case HOMING_SHOT: if (shotsFired < 1) { for (int i=0; i<MAX_BULLETS; i++) if (!bossBullets[i].active) { bossBullets[i].active=true; bossBullets[i].position=boss.position; bossBullets[i].color=PINK; bossBullets[i].radius=15.0f; bossBullets[i].type=HOMING; bossBullets[i].lifeTimer = 0; bossBullets[i].isParryable = true; shotsFired++; break;}} if (shotsFired >= 1) bossIsAttacking = false; break;
                }
            }

            for (int i=0; i<MAX_BULLETS; i++) if (bossBullets[i].active) {
                if (bossBullets[i].type==SINE_WAVE) { bossBullets[i].position.x+=bossBullets[i].speed.x; bossBullets[i].waveAngle+=0.1f; bossBullets[i].position.y=bossBullets[i].originY+sinf(bossBullets[i].waveAngle)*80.0f; }
                else if (bossBullets[i].type==HOMING) { bossBullets[i].lifeTimer++; if (bossBullets[i].lifeTimer < 360) { Vector2 playerCenter = {player.hitbox.x + player.hitbox.width/2, player.hitbox.y + player.hitbox.height/2}; float a=atan2f(playerCenter.y-bossBullets[i].position.y,playerCenter.x-bossBullets[i].position.x); float s=3.5f; bossBullets[i].speed.x=cosf(a)*s; bossBullets[i].speed.y=sinf(a)*s; } bossBullets[i].position.x+=bossBullets[i].speed.x; bossBullets[i].position.y+=bossBullets[i].speed.y; }
                else { bossBullets[i].position.x+=bossBullets[i].speed.x; bossBullets[i].position.y+=bossBullets[i].speed.y; }
                if (bossBullets[i].position.x < -50 || bossBullets[i].position.x > screenWidth+50 || bossBullets[i].position.y < -50 || bossBullets[i].position.y > screenHeight+50) bossBullets[i].active = false;
                if (CheckCollisionCircleRec(bossBullets[i].position, bossBullets[i].radius, player.hitbox)) { if (bossBullets[i].isParryable && player.isDashing) { bossBullets[i].active = false; player.dashCooldown = 0; if (player.superMeter < 4) player.superMeter++; player.isParryInvulnerable = true; } else if (player.isDashing) { if (!player.isParryInvulnerable && !player.isInvulnerable) { player.health--; bossBullets[i].active = false; player.isInvulnerable = true; player.invulnerabilityTimer = 120; } } else if (!player.isInvulnerable){ player.health--; bossBullets[i].active = false; player.isInvulnerable = true; player.invulnerabilityTimer = 120;}}}
            if (boss.health<=0) victory=true; if (player.health<=0) gameOver=true;
        }

        // --- 2.3. DRAW ---
        BeginDrawing();
            ClearBackground(DARKGRAY);
            DrawTexturePro(background, (Rectangle){ 0, 0, (float)background.width, (float)background.height }, (Rectangle){ 0, 0, (float)screenWidth, (float)screenHeight }, (Vector2){ 0, 0 }, 0.0f, WHITE);
            
            if (!player.isInvulnerable || (player.invulnerabilityTimer % 10 < 5))
            {
                 // Descomente a linha abaixo para ver a hitbox retangular de depuração
                 // DrawRectangleRec(player.hitbox, Fade(GREEN, 0.4f)); 
                 if (player.isDashing) DrawRectangleLinesEx(player.hitbox, 3.0f, Fade(WHITE, 0.5f));

                 Vector2 playerCenter = {player.hitbox.x + player.hitbox.width/2, player.hitbox.y + player.hitbox.height/2};
                 Vector2 playerTextureDrawPos = { playerCenter.x - (playerTexture.width * PLAYER_SCALE / 2), playerCenter.y - (playerTexture.height * PLAYER_SCALE / 2) };
                 DrawTextureEx(playerTexture, playerTextureDrawPos, 0.0f, PLAYER_SCALE, WHITE);
            }
            
            DrawCircleV(boss.position, boss.radius, boss.color);
            if (tripleShotPowerUp.active) { DrawCircleV(tripleShotPowerUp.position, tripleShotPowerUp.radius, tripleShotPowerUp.color); DrawCircle(tripleShotPowerUp.position.x, tripleShotPowerUp.position.y, tripleShotPowerUp.radius+5, Fade(GOLD, 0.3f)); }
            for (int i=0; i<MAX_BULLETS; i++) { if (playerBullets[i].active) DrawCircleV(playerBullets[i].position, playerBullets[i].radius, playerBullets[i].color); if (bossBullets[i].active) DrawCircleV(bossBullets[i].position, bossBullets[i].radius, bossBullets[i].color); }
            if (superAttackActive){ DrawRectangleRec(superBeam, Fade(YELLOW, 0.5f)); DrawRectangleRec((Rectangle){superBeam.x, superBeam.y + 5, superBeam.width, superBeam.height - 10}, Fade(ORANGE, 0.8f));}
            DrawText("JOGADOR", 10, 10, 20, WHITE); DrawRectangle(10, 40, player.health*20, 20, GREEN);
            const float bossBarWidth = 300.0f; const float bossBarX = screenWidth - 10 - bossBarWidth; float currentBossHealthWidth = ((float)boss.health / BOSS_HEALTH) * bossBarWidth; if (currentBossHealthWidth < 0) currentBossHealthWidth = 0; int textWidth = MeasureText("CHEFE", 20); float textX = bossBarX + (bossBarWidth / 2) - (textWidth / 2);
            DrawText("CHEFE", textX, 10, 20, WHITE); DrawRectangle(bossBarX, 40, currentBossHealthWidth, 20, MAROON);
            DrawText("DASH (SHIFT)", 10, screenHeight-60, 20, WHITE); DrawRectangle(10, screenHeight-30, 150, 20, DARKGRAY); if (player.dashCooldown<=0) DrawRectangle(10, screenHeight-30, 150, 20, SKYBLUE); else DrawRectangle(10, screenHeight-30, 150-(player.dashCooldown/120*150), 20, SKYBLUE);
            Color superTextColor = (player.superMeter < 4) ? WHITE : GOLD;
            DrawText("SUPER (E)", 200, screenHeight-60, 20, superTextColor);
            DrawText(TextFormat("%i/4", player.superMeter), 200, screenHeight-30, 20, superTextColor);
            if (player.hasTripleShot){ float remainingSeconds = player.tripleShotTimer / 60.0f; DrawText(TextFormat("TIRO TRIPLO: %.1fs", remainingSeconds), 400, screenHeight - 60, 20, GOLD); DrawRectangle(400, screenHeight - 30, (player.tripleShotTimer / 480.0f) * 200, 20, GOLD);}
            if (victory) DrawText("VOCÊ VENCEU!", screenWidth/2-MeasureText("VOCÊ VENCEU!", 60)/2, screenHeight/2-30, 60, GOLD);
            if (gameOver) DrawText("GAME OVER", screenWidth/2-MeasureText("GAME OVER", 60)/2, screenHeight/2-30, 60, GRAY);
        EndDrawing();
    }

    UnloadTexture(background);
    UnloadTexture(playerTexture);
    CloseWindow();
    return 0;
}