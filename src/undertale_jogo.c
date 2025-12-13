#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

// ============================================================================
// CONFIGURAÇÕES GERAIS
// ============================================================================
#define MENU_LARGURA 800
#define MENU_ALTURA 600
#define GAME_LARGURA 1920
#define GAME_ALTURA 1080

// Dificuldades (Fase 1)
#define VIDA_FACIL   50
#define VIDA_MEDIO   20
#define VIDA_DIFICIL 5

// Configurações Fase 1
#define QUESTOES_TOTAIS 1
#define ESTUDOS_POR_QUESTAO 1
#define TEMPO_TURNO_INIMIGO 10.0f
#define PROJETEIS_MAXIMOS 500
#define MAXIMO_PROJETEIS 8

// Configurações Fase 2 (Asriel)
#define PLAYER_SPEED 5.0f
#define ARENA_WIDTH 300
#define ARENA_HEIGHT 300
#define MAX_STARS 100
#define MAX_LIGHTNINGS 15
#define MAX_PARTICLES 500

// Menu
#define HEART_SIZE 20.0f
#define HEART_SPEED 300.0f


    float larguraTelaAtual;
    float alturaTelaAtual;

// ============================================================================
// ENUMS
// ============================================================================
typedef enum {
    SCREEN_MENU = 0,
    SCREEN_GAME,
    SCREEN_OPTIONS,
    SCREEN_CREDITS,
    SCREEN_EXIT
} GameScreen;

typedef enum {
    MENU_JOGAR = 0,
    MENU_OPCOES,
    MENU_CREDITOS,
    MENU_COUNT
} MenuState;

typedef enum {
    DIFICULDADE_FACIL = 0,
    DIFICULDADE_MEDIO,
    DIFICULDADE_DIFICIL
} DificuldadeJogo;

// Estados Fase 1
typedef enum { 
    TURNO_JOGADOR, 
    MENU_AGIR, 
    EXIBINDO_TEXTO, 
    TURNO_INIMIGO, 
    GAME_OVER_FASE1, 
    VITORIA_FASE1,
    TRANSICAO_FASE2
} EstadoJogoInterno;

// Estados Fase 2
typedef enum { 
    FASE2_GAMEPLAY, 
    FASE2_GAME_OVER, 
    FASE2_VITORIA 
} EstadoFase2;

typedef enum {
    PADRAO_NULO, 
    RAJADA_RAPIDA, 
    ATAQUE_LETRA_C, 
    ESPIRAL_PULSANTE, 
    CHUVA_ESTELAR_ANGULAR,
    ATAQUE_LANCA, 
    COMBO_CAOTICO, 
    VARREDURA_LASER, 
    ATAQUE_FINAL
} PadraoAtaque;

typedef enum { 
    PROJETIL_NORMAL, 
    ESTRELA_MAE, 
    FRAGMENTO_ESTELAR, 
    LANCA_AVISO, 
    LANCA_PROJETIL, 
    PROJETIL_RICOCHETE 
} TipoProjetil;

typedef enum { 
    NONE, 
    STAR_BLAZING, 
    SHOCKER_BREAKER, 
    CHAOS_SABER, 
    HYPER_GONER 
} AttackType;

// ============================================================================
// ESTRUTURAS
// ============================================================================
// Menu
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

// Fase 1
typedef struct { 
    Vector2 position; 
    int sanidade; 
    float temporizadorInvencibilidade; 
    float dashCooldown; 
    bool acabouDeDarDash; 
} Aluno;

typedef struct {
    Rectangle rect; 
    float rotation; 
    float timer; 
    int stage;
    Vector2 blasterPos;
} Laser;

typedef struct {
    Rectangle corpoVisual; 
    int estudosFeitos; 
    int questoesResolvidas; 
    bool podeResolverQuestao; 
    bool podeEntregarProva; 
    bool turnoFinalAtivo; 
    int turnosTotaisPassados;
    PadraoAtaque padraoAtual; 
    float temporizadorTurno; 
    float delayInicialTimer; 
    float anguloEspiral; 
    float temporizadorSubAtaque;
    int estagioAtaque; 
    int contadorAtaque; 
    Vector2 posAtaqueSalva;
    Vector2 posLetraC; 
    float rotacaoLetraC;
    Laser lasers[MAXIMO_PROJETEIS];
} Professor;

typedef struct { 
    Vector2 position; 
    Vector2 speed; 
    float radius; 
    float lifeTimer; 
    int ricochetes; 
    bool active; 
    Color color; 
    TipoProjetil type; 
} Projetil;

// Fase 2
typedef struct {
    Vector2 position;
    int hp;
    float invincibilityTimer;
    bool isDamaged;
} Player;

typedef struct {
    Vector2 position;
    Vector2 speed;
    float radius;
    Color color;
    float rotation;
    bool active;
} Star;

typedef struct {
    Rectangle warningRect;
    Rectangle strikeRect;
    float timer;
    int state;
    bool active;
    Color color;
} Lightning;

typedef struct {
    Vector2 position;
    Vector2 speed;
    Color color;
    float radius;
    float lifetime;
    bool active;
} Particle;

// ============================================================================
// VARIÁVEIS GLOBAIS
// ============================================================================
static DificuldadeJogo dificuldadeSelecionada = DIFICULDADE_MEDIO;
static Sound fxSelect;

// Fase 1
static EstadoJogoInterno estadoAtualInterno;
static Aluno aluno;
static Professor acm;
static Projetil projeteis[PROJETEIS_MAXIMOS];
static Rectangle caixaBatalha, caixaInterfaceInferior;
static char textoDialogo[256];
static bool acaoTerminaTurno;
static Rectangle botoesMenuGame[4];
static int menuSel = 0;
static const char *opcoesMenuAgir[] = { "Descricao", "Estudar", "Resolver Questao" };
static int menuAgirSel = 0;
static Sound fxHit, fxShoot, fxLaserCharge, fxLaserFire, fxMenuSelectGame, fxDamage;
static Texture2D texturaProfessor;
static Music musicBackground;   // Fase 1
static Music musicFase2;        // Fase 2
static Music musicFinal;        // Toca quando o jogador vence o jogo


// Fase 2
static EstadoFase2 estadoFase2;
static Player player;
static Rectangle arena;
static Texture2D texHeart;
static Texture2D texStar;
static Texture2D texLightningWarning;
static Texture2D texLightningStrike;
static Texture2D texChaosSaber;
static Texture2D texHyperGonerMouth;
static Texture2D texHyperGonerCharge;
static Texture2D texHyperGonerBeam;
static Texture2D texBackgroundStar;
static Star stars[MAX_STARS];
static Lightning lightnings[MAX_LIGHTNINGS];
static Rectangle chaosSabers[2];
static Particle particles[MAX_PARTICLES];
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
static Music musicFase2;

// Transição
static float transicaoTimer = 0.0f;
static float transicaoAlpha = 0.0f;

// ============================================================================
// PROTÓTIPOS
// ============================================================================
// Menu
void DrawHeart(Heart heart, Color color);
bool CheckHeartCollision(Heart heart, Rectangle bounds);
void UpdateHeart(Heart* heart, float deltaTime);

// Fase 1
void RunUndertaleGame(GameScreen *currentScreen);
static void IniciarJogoInterno(void);
static void MudarParaTurnoInimigoInterno(void);
static void AtualizarJogoInterno(void);
static void AtualizarTurnoJogadorInterna(void);
static void AtualizarMenuAgirInterna(void);
static void AtualizarExibindoTextoInterna(void);
static void AtualizarTurnoInimigoInterna(void);
static void DesenharJogoInterno(void);
static void DesenharInterfaceInterna(void);
static void DesenharTurnoJogadorInterna(void);
static void DesenharMenuAgirInterna(void);
static void DesenharCaixaDialogoInterna(void);
static void DesenharTurnoInimigoInterna(void);
static void AtivarProjetil(Vector2 pos, Vector2 vel, float radius, Color color, TipoProjetil type, float life);
static void ExecutarAtaqueChefe(float modificadorVelocidade);
static void CriarExplosaoEstelar(Vector2 pos, float modVel);

// Fase 2
void IniciarFase2(void);
void AtualizarFase2(void);
void DesenharFase2(void);
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

// Helpers
static Sound CarregarSomSeguro(const char* nomeArquivo);
static Music CarregarMusicaSegura(const char* nomeArquivo);

// ============================================================================
// MAIN
// ============================================================================
int main(void) {
    InitWindow(MENU_LARGURA, MENU_ALTURA, "PITale - Complete Edition");
    SetTargetFPS(60);
    InitAudioDevice();
    SetMasterVolume(1.0f);
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();

    fxSelect = CarregarSomSeguro("select.wav");

    Font font = GetFontDefault();
    GameScreen currentScreen = SCREEN_MENU;

    Heart playerHeart = {
        .position = {MENU_LARGURA / 2.0f, MENU_ALTURA / 2.0f + 20},
        .size = HEART_SIZE,
        .speed = HEART_SPEED
    };

    MenuOption options[MENU_COUNT];
    options[MENU_JOGAR].text = "Jogar"; 
    options[MENU_JOGAR].position = (Vector2){MENU_LARGURA / 2.0f, MENU_ALTURA / 2.0f - 40};
    options[MENU_OPCOES].text = "Opcoes"; 
    options[MENU_OPCOES].position = (Vector2){MENU_LARGURA * 0.25f, MENU_ALTURA / 2.0f + 60};
    options[MENU_CREDITOS].text = "Creditos"; 
    options[MENU_CREDITOS].position = (Vector2){MENU_LARGURA * 0.75f, MENU_ALTURA / 2.0f + 60};

    for (int i = 0; i < MENU_COUNT; i++) {
        options[i].isHovered = false;
        Vector2 textSize = MeasureTextEx(font, options[i].text, 40, 2);
        options[i].bounds = (Rectangle){
            options[i].position.x - textSize.x / 2.0f - 20,
            options[i].position.y - textSize.y / 2.0f - 10,
            textSize.x + 40, textSize.y + 20
        };
    }

    EasterEgg easterEgg;
    easterEgg.text = "sem saida";
    easterEgg.normalColor = Fade(BLACK, 0.0f);
    easterEgg.hoverColor = YELLOW;
    easterEgg.isHovered = false;
    Vector2 eeSize = MeasureTextEx(font, easterEgg.text, 24, 2);
    easterEgg.position = (Vector2){MENU_LARGURA - eeSize.x - 40, MENU_ALTURA - eeSize.y - 40};
    easterEgg.bounds = (Rectangle){easterEgg.position.x - 10, easterEgg.position.y - 10, eeSize.x + 20, eeSize.y + 20};

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
                            SetWindowSize(GAME_LARGURA, GAME_ALTURA);
                            SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - GAME_LARGURA/2, GetMonitorHeight(GetCurrentMonitor())/2 - GAME_ALTURA/2);
                            ToggleFullscreen();
                            break;
                        case MENU_OPCOES: currentScreen = SCREEN_OPTIONS; break;
                        case MENU_CREDITOS: currentScreen = SCREEN_CREDITS; break;
                    }
                    selectedOption = -1;
                }

                BeginDrawing();
                ClearBackground(BLACK);
                const char* title = "PITale";
                Vector2 titleSize = MeasureTextEx(font, title, 70, 2);
                DrawTextEx(font, title, (Vector2){MENU_LARGURA/2 - titleSize.x/2, 80}, 70, 2, WHITE);

                for (int i = 0; i < MENU_COUNT; i++) {
                    Color color = options[i].isHovered ? YELLOW : WHITE;
                    Vector2 txtSz = MeasureTextEx(font, options[i].text, 40, 2);
                    DrawTextEx(font, options[i].text, (Vector2){options[i].position.x - txtSz.x/2, options[i].position.y - txtSz.y/2}, 40, 2, color);
                    if (options[i].isHovered) DrawRectangleLinesEx(options[i].bounds, 2, YELLOW);
                }

                if(easterEgg.isHovered) DrawTextEx(font, easterEgg.text, easterEgg.position, 24, 2, easterEgg.hoverColor);
                DrawHeart(playerHeart, RED);
                DrawText("[WASD/Setas] Mover  [Z] Confirmar  [X] Voltar", MENU_LARGURA/2 - MeasureText("[WASD/Setas] Mover  [Z] Confirmar  [X] Voltar", 18)/2, MENU_ALTURA - 30, 18, GRAY);
                EndDrawing();
                break;
            }

            case SCREEN_GAME: {
                RunUndertaleGame(&currentScreen);
                if (currentScreen == SCREEN_MENU) {
                    if (IsWindowFullscreen()) ToggleFullscreen();
                    SetWindowSize(MENU_LARGURA, MENU_ALTURA);
                    SetWindowPosition(GetMonitorWidth(GetCurrentMonitor())/2 - MENU_LARGURA/2, GetMonitorHeight(GetCurrentMonitor())/2 - MENU_ALTURA/2);
                }
                break;
            }

            case SCREEN_OPTIONS: {
                if (IsKeyPressed(KEY_RIGHT)) {
                    dificuldadeSelecionada++;
                    if (dificuldadeSelecionada > DIFICULDADE_DIFICIL) dificuldadeSelecionada = DIFICULDADE_FACIL;
                    PlaySound(fxSelect);
                }
                if (IsKeyPressed(KEY_LEFT)) {
                    dificuldadeSelecionada--;
                    if (dificuldadeSelecionada < DIFICULDADE_FACIL) dificuldadeSelecionada = DIFICULDADE_DIFICIL;
                    PlaySound(fxSelect);
                }

                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("OPCOES", MENU_LARGURA/2 - MeasureText("OPCOES", 60)/2, 100, 60, WHITE);
                DrawText("SELECIONE A DIFICULDADE:", MENU_LARGURA/2 - MeasureText("SELECIONE A DIFICULDADE:", 30)/2, 250, 30, GRAY);

                const char* nomeDif;
                Color corDif;
                if (dificuldadeSelecionada == DIFICULDADE_FACIL) { nomeDif = "< FACIL >"; corDif = GREEN; }
                else if (dificuldadeSelecionada == DIFICULDADE_MEDIO) { nomeDif = "< MEDIO >"; corDif = YELLOW; }
                else { nomeDif = "< DIFICIL >"; corDif = RED; }

                DrawText(nomeDif, MENU_LARGURA/2 - MeasureText(nomeDif, 50)/2, 320, 50, corDif);
                DrawText("Use as SETAS para mudar", MENU_LARGURA/2 - MeasureText("Use as SETAS para mudar", 20)/2, 400, 20, DARKGRAY);
                DrawText("[X] VOLTAR", MENU_LARGURA/2 - MeasureText("[X] VOLTAR", 20)/2, 500, 20, YELLOW);
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
                DrawText("CREDITOS", MENU_LARGURA/2 - MeasureText("CREDITOS", 60)/2, 80, 60, WHITE);
                const char* nomes = "Joao Augusto\n\nCauan Felipe\n\nVinicius Oliveira\n\nLuis Henrique\n\nVinicius";
                Vector2 nomesSize = MeasureTextEx(font, nomes, 30, 2);
                DrawTextEx(font, nomes, (Vector2){MENU_LARGURA/2 - nomesSize.x/2, 180}, 30, 2, WHITE);
                DrawText("[X] VOLTAR", MENU_LARGURA/2 - MeasureText("[X] VOLTAR", 20)/2, 550, 20, YELLOW);
                EndDrawing();

                if (IsKeyPressed(KEY_X)) {
                    PlaySound(fxSelect);
                    currentScreen = SCREEN_MENU;
                }
                break;
            }

            case SCREEN_EXIT:
                CloseWindow();
                break;
        }
    }

    UnloadSound(fxSelect);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

// ============================================================================
// FUNÇÕES DO MENU
// ============================================================================
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
    if (heart->position.x > MENU_LARGURA - HEART_SIZE) heart->position.x = MENU_LARGURA - HEART_SIZE;
    if (heart->position.y < HEART_SIZE) heart->position.y = HEART_SIZE;
    if (heart->position.y > MENU_ALTURA - HEART_SIZE) heart->position.y = MENU_ALTURA - HEART_SIZE;
}

// ============================================================================
// FASE 1 - PROFESSOR ACM
// ============================================================================
void RunUndertaleGame(GameScreen *currentScreen) {
    TraceLog(LOG_INFO, "DEBUG: RunUndertaleGame iniciada.");
    SetTargetFPS(60);
    SetRandomSeed(time(NULL));
    if (!IsAudioDeviceReady()) InitAudioDevice();

    fxHit = CarregarSomSeguro("hit.wav");
    fxShoot = CarregarSomSeguro("shoot.wav");
    fxLaserCharge = CarregarSomSeguro("laser_charge.wav");
    fxLaserFire = CarregarSomSeguro("laser_fire.wav");
    fxMenuSelectGame = CarregarSomSeguro("select.wav");
    fxDamage = CarregarSomSeguro("damage_sound.wav");
    musicBackground = CarregarMusicaSegura("Megalovania.mp3");

    if (musicBackground.frameCount > 0) {
        PlayMusicStream(musicBackground);
        SetMusicVolume(musicBackground, 0.7f);
    }

    texturaProfessor = LoadTexture("../resources/professor.png");
    if (texturaProfessor.id == 0) texturaProfessor = LoadTexture("resources/professor.png");

    IniciarJogoInterno();

    while (*currentScreen == SCREEN_GAME && !WindowShouldClose()) {
        UpdateMusicStream(musicBackground);
        AtualizarJogoInterno();
        DesenharJogoInterno();

        if (estadoAtualInterno == GAME_OVER_FASE1) {
            if (IsKeyPressed(KEY_Z)) {
                PlaySound(fxMenuSelectGame);
                *currentScreen = SCREEN_MENU;
                UnloadSound(fxHit); UnloadSound(fxShoot); 
                UnloadSound(fxLaserCharge); UnloadSound(fxLaserFire); 
                UnloadSound(fxMenuSelectGame); UnloadSound(fxDamage);
                UnloadTexture(texturaProfessor);
                UnloadMusicStream(musicBackground);
                return;
            }
        }

        if (estadoAtualInterno == TRANSICAO_FASE2) {
            transicaoTimer += GetFrameTime();
            transicaoAlpha += GetFrameTime() * 0.5f;
            if (transicaoAlpha > 1.0f) transicaoAlpha = 1.0f;

            if (transicaoTimer > 3.0f) {
                // Inicia Fase 2
                UnloadMusicStream(musicBackground);
                IniciarFase2();

                // Loop Fase 2
                while(*currentScreen == SCREEN_GAME && !WindowShouldClose()){
                if(musicFase2.frameCount > 0) UpdateMusicStream(musicFase2);
                if(musicFinal.frameCount > 0) UpdateMusicStream(musicFinal);

                AtualizarFase2();
                DesenharFase2();

                    if (estadoFase2 == FASE2_GAME_OVER || estadoFase2 == FASE2_VITORIA) {
                        if (IsKeyPressed(KEY_Z)) {
                            PlaySound(fxMenuSelectGame);
                            *currentScreen = SCREEN_MENU;
                            // Cleanup Fase 2
                            UnloadTexture(texHeart);
                            UnloadTexture(texStar);
                            UnloadTexture(texLightningWarning);
                            UnloadTexture(texLightningStrike);
                            UnloadTexture(texChaosSaber);
                            UnloadTexture(texHyperGonerMouth);
                            UnloadTexture(texHyperGonerCharge);
                            UnloadTexture(texHyperGonerBeam);
                            UnloadTexture(texBackgroundStar);

                            if(musicFase2.frameCount > 0) UnloadMusicStream(musicFase2);
                            if(musicFinal.frameCount > 0) UnloadMusicStream(musicFinal);

                            // Cleanup Fase 1
                            UnloadSound(fxHit); UnloadSound(fxShoot); 
                            UnloadSound(fxLaserCharge); UnloadSound(fxLaserFire); 
                            UnloadSound(fxMenuSelectGame); UnloadSound(fxDamage);
                            UnloadTexture(texturaProfessor);
                            return;
                        }
                    }
                }
                return;
            }
        }
    }

    if (WindowShouldClose()) {
        *currentScreen = SCREEN_EXIT;
        UnloadSound(fxHit); UnloadSound(fxShoot); 
        UnloadSound(fxLaserCharge); UnloadSound(fxLaserFire); 
        UnloadSound(fxMenuSelectGame); UnloadSound(fxDamage);
        UnloadTexture(texturaProfessor);
        UnloadMusicStream(musicBackground);
    }
}

static void IniciarJogoInterno(void) {
    estadoAtualInterno = EXIBINDO_TEXTO; 
    acaoTerminaTurno = false; 
    menuSel = 0; 
    menuAgirSel = 0;
    strcpy(textoDialogo, "* Professor ACM encara voce. A prova final definira seu futuro.");

    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();

    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;

    caixaBatalha = (Rectangle){
        larguraTelaAtual/2.0f - (800.0f * scaleX)/2.0f, 
        320.0f * scaleY, 
        800.0f * scaleX, 
        400.0f * scaleY
    };
    caixaInterfaceInferior = (Rectangle){
        larguraTelaAtual/2.0f - (1520.0f * scaleX)/2.0f, 
        750.0f * scaleY, 
        1520.0f * scaleX, 
        300.0f * scaleY
    };

    int vidaInicial = VIDA_MEDIO;
    if (dificuldadeSelecionada == DIFICULDADE_FACIL) vidaInicial = VIDA_FACIL;
    if (dificuldadeSelecionada == DIFICULDADE_DIFICIL) vidaInicial = VIDA_DIFICIL;

    aluno = (Aluno){
        (Vector2){caixaBatalha.x + caixaBatalha.width/2.0f, caixaBatalha.y + caixaBatalha.height/2.0f},
        vidaInicial, 
        0, 
        0, 
        false
    };

    acm = (Professor){
        (Rectangle){larguraTelaAtual/2.0f - (100.0f * scaleX), 100.0f * scaleY, 200.0f * scaleX, 200.0f * scaleY},
        0, 0, false, false, false, 0,
        PADRAO_NULO, TEMPO_TURNO_INIMIGO, 0, 0, 0, 0, 0,
        (Vector2){-1000,-1000}, 0
    };

    float btnW = 220.0f * scaleX; 
    float btnH = 70.0f * scaleY; 
    float sp = 40.0f * scaleX;
    float paddingX = 40.0f * scaleX; 
    float paddingY = (caixaInterfaceInferior.height - (btnH*2+sp))/2;
    float sX = caixaInterfaceInferior.x + caixaInterfaceInferior.width - (btnW*2+sp) - paddingX;
    float sY = caixaInterfaceInferior.y + paddingY;

    botoesMenuGame[0] = (Rectangle){sX, sY, btnW, btnH};
    botoesMenuGame[1] = (Rectangle){sX + btnW + sp, sY, btnW, btnH};
    botoesMenuGame[2] = (Rectangle){sX, sY + btnH + sp, btnW, btnH};
    botoesMenuGame[3] = (Rectangle){sX + btnW + sp, sY + btnH + sp, btnW, btnH};

    for(int i=0; i<PROJETEIS_MAXIMOS; i++) projeteis[i].active = false;
    for(int i=0; i<MAXIMO_PROJETEIS; i++) acm.lasers[i] = (Laser){0};
}

static void MudarParaTurnoInimigoInterno(void) {
    estadoAtualInterno = TURNO_INIMIGO;
    acm.temporizadorTurno = TEMPO_TURNO_INIMIGO;
    acm.delayInicialTimer = 1.5f;
    acm.anguloEspiral = 0; 
    acm.temporizadorSubAtaque = 0;
    acm.estagioAtaque = 0; 
    acm.contadorAtaque = 0;

    for(int i=0; i<PROJETEIS_MAXIMOS; i++) projeteis[i].active = false;
    for(int i=0; i<MAXIMO_PROJETEIS; i++) acm.lasers[i] = (Laser){0};

    int r = rand() % 100;
    if (acm.turnoFinalAtivo) acm.padraoAtual = ATAQUE_FINAL;
    else if (r < 20) acm.padraoAtual = RAJADA_RAPIDA;
    else if (r < 40) acm.padraoAtual = ATAQUE_LETRA_C;
    else if (r < 60) acm.padraoAtual = ESPIRAL_PULSANTE;
    else if (r < 75) acm.padraoAtual = CHUVA_ESTELAR_ANGULAR;
    else if (r < 90) acm.padraoAtual = ATAQUE_LANCA;
    else acm.padraoAtual = COMBO_CAOTICO;
}

static void AtualizarJogoInterno(void) {
    switch (estadoAtualInterno) {
        case TURNO_JOGADOR: AtualizarTurnoJogadorInterna(); break;
        case MENU_AGIR: AtualizarMenuAgirInterna(); break;
        case EXIBINDO_TEXTO: AtualizarExibindoTextoInterna(); break;
        case TURNO_INIMIGO: AtualizarTurnoInimigoInterna(); break;
        case GAME_OVER_FASE1: break;
        case VITORIA_FASE1: break;
        case TRANSICAO_FASE2: break;
    }
}

static void AtualizarTurnoJogadorInterna(void) {
    if (IsKeyPressed(KEY_RIGHT) && (menuSel == 0 || menuSel == 2)) menuSel++;
    if (IsKeyPressed(KEY_LEFT) && (menuSel == 1 || menuSel == 3)) menuSel--;
    if (IsKeyPressed(KEY_DOWN) && (menuSel == 0 || menuSel == 1)) menuSel += 2;
    if (IsKeyPressed(KEY_UP) && (menuSel == 2 || menuSel == 3)) menuSel -= 2;

    if (IsKeyPressed(KEY_Z)) {
        PlaySound(fxMenuSelectGame);
        acaoTerminaTurno = false;
        estadoAtualInterno = EXIBINDO_TEXTO;

        switch (menuSel) {
            case 0:
                estadoAtualInterno = MENU_AGIR; 
                menuAgirSel = 0; 
                break;
            case 1:
                strcpy(textoDialogo, "* Voce nao tem itens para usar."); 
                break;
            case 2:
                if (acm.podeEntregarProva) {
                    strcpy(textoDialogo, "* Voce entrega a prova. Prepare-se para a avaliacao final!");
                    acm.turnoFinalAtivo = true;
                    acaoTerminaTurno = true;
                } else {
                    strcpy(textoDialogo, TextFormat("* Ainda faltam %d questoes.", QUESTOES_TOTAIS - acm.questoesResolvidas));
                }
                break;
            case 3:
                strcpy(textoDialogo, "* Voce desistiu da recuperacao...");
                estadoAtualInterno = GAME_OVER_FASE1;
                break;
        }
    }
}

static void AtualizarMenuAgirInterna(void) {
    if (IsKeyPressed(KEY_DOWN)) menuAgirSel++;
    if (IsKeyPressed(KEY_UP)) menuAgirSel--;
    if (menuAgirSel < 0) menuAgirSel = 2;
    if (menuAgirSel > 2) menuAgirSel = 0;

    if (IsKeyPressed(KEY_Z)) {
        PlaySound(fxMenuSelectGame); 
        acaoTerminaTurno = true;
        estadoAtualInterno = EXIBINDO_TEXTO;

        switch (menuAgirSel) {
            case 0:
                strcpy(textoDialogo, TextFormat("* Prof. ACM. Questoes resolvidas: %d/%d.", acm.questoesResolvidas, QUESTOES_TOTAIS));
                acaoTerminaTurno = false;
                break;
            case 1:
                if (acm.podeResolverQuestao) {
                    strcpy(textoDialogo, "* Voce ja estudou o suficiente.");
                    acaoTerminaTurno = false;
                } else {
                    acm.estudosFeitos++;
                    if (acm.estudosFeitos >= ESTUDOS_POR_QUESTAO) {
                        acm.podeResolverQuestao = true;
                        strcpy(textoDialogo, "* Voce se sente preparado.");
                    } else {
                        strcpy(textoDialogo, TextFormat("* Voce estuda... Falta %d.", ESTUDOS_POR_QUESTAO - acm.estudosFeitos));
                    }
                }
                break;
            case 2:
                if (acm.podeResolverQuestao) {
                    acm.questoesResolvidas++;
                    acm.estudosFeitos = 0;
                    acm.podeResolverQuestao = false;
                    strcpy(textoDialogo, TextFormat("* Voce resolveu a questao %d!", acm.questoesResolvidas));
                    if (acm.questoesResolvidas >= QUESTOES_TOTAIS) acm.podeEntregarProva = true;
                } else {
                    strcpy(textoDialogo, "* Voce precisa estudar mais.");
                    acaoTerminaTurno = false;
                }
                break;
        }
    }

    if (IsKeyPressed(KEY_X)) {
        PlaySound(fxMenuSelectGame);
        estadoAtualInterno = TURNO_JOGADOR;
    }
}

static void AtualizarExibindoTextoInterna(void) {
    if (IsKeyPressed(KEY_Z)) {
        PlaySound(fxMenuSelectGame);

        if (acm.turnoFinalAtivo && acaoTerminaTurno) {
            // Transição para Fase 2!
            estadoAtualInterno = TRANSICAO_FASE2;
            transicaoTimer = 0.0f;
            transicaoAlpha = 0.0f;
            StopMusicStream(musicBackground);
        }
        else if (acaoTerminaTurno) MudarParaTurnoInimigoInterno();
        else estadoAtualInterno = TURNO_JOGADOR;
    }
}

static void AtualizarTurnoInimigoInterna(void) {
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();
    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;

    if (aluno.temporizadorInvencibilidade > 0) aluno.temporizadorInvencibilidade -= GetFrameTime();
    if (aluno.dashCooldown > 0) aluno.dashCooldown -= GetFrameTime();

    if (IsKeyPressed(KEY_LEFT_SHIFT) && aluno.dashCooldown <= 0) {
        aluno.acabouDeDarDash = true;
        aluno.temporizadorInvencibilidade = 0.3f;
        aluno.dashCooldown = 1.0f;
    }

    float velAluno = 600.0f * scaleY * GetFrameTime();
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) aluno.position.y -= velAluno;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) aluno.position.y += velAluno;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) aluno.position.x -= velAluno;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) aluno.position.x += velAluno;

    if (aluno.position.x < caixaBatalha.x) aluno.position.x = caixaBatalha.x;
    if (aluno.position.x > caixaBatalha.x + caixaBatalha.width) aluno.position.x = caixaBatalha.x + caixaBatalha.width;
    if (aluno.position.y < caixaBatalha.y) aluno.position.y = caixaBatalha.y;
    if (aluno.position.y > caixaBatalha.y + caixaBatalha.height) aluno.position.y = caixaBatalha.y + caixaBatalha.height;

    if (acm.delayInicialTimer > 0) {
        acm.delayInicialTimer -= GetFrameTime();
        if (acm.delayInicialTimer <= 0) acm.temporizadorTurno = TEMPO_TURNO_INIMIGO;
    } else {
        acm.temporizadorTurno -= GetFrameTime();
        if (acm.temporizadorTurno <= 0) {
            estadoAtualInterno = TURNO_JOGADOR;
            acm.turnoFinalAtivo = false;
            acm.turnosTotaisPassados++;
            strcpy(textoDialogo, "* O professor ACM aguarda sua proxima acao.");
        }

        float modVel = 1.0f + (float)acm.turnosTotaisPassados * 0.1f;
        ExecutarAtaqueChefe(modVel);

        for (int i=0; i<PROJETEIS_MAXIMOS; i++) {
            if (projeteis[i].active) {
                projeteis[i].position.x += projeteis[i].speed.x * GetFrameTime();
                projeteis[i].position.y += projeteis[i].speed.y * GetFrameTime();

                if (projeteis[i].lifeTimer > 0) {
                    projeteis[i].lifeTimer -= GetFrameTime();
                    if (projeteis[i].lifeTimer <= 0) {
                        projeteis[i].active = false;
                        if (projeteis[i].type == ESTRELA_MAE) CriarExplosaoEstelar(projeteis[i].position, modVel);
                        if (projeteis[i].type == LANCA_AVISO) {
                            Vector2 vel = {0,0};
                            float velLanca = 1200.0f * scaleY * modVel;
                            if (projeteis[i].position.x < caixaBatalha.x) vel.x = velLanca;
                            else if (projeteis[i].position.x > caixaBatalha.x + caixaBatalha.width) vel.x = -velLanca;
                            else if (projeteis[i].position.y < caixaBatalha.y) vel.y = velLanca;
                            else if (projeteis[i].position.y > caixaBatalha.y + caixaBatalha.height) vel.y = -velLanca;
                            AtivarProjetil(projeteis[i].position, vel, 10.0f * scaleX, BLUE, LANCA_PROJETIL, 0);
                        }
                    }
                }

                if (projeteis[i].type == PROJETIL_RICOCHETE) {
                    if (projeteis[i].ricochetes < 2) {
                        if (projeteis[i].position.x <= caixaBatalha.x || projeteis[i].position.x >= caixaBatalha.x + caixaBatalha.width - projeteis[i].radius) { 
                            projeteis[i].speed.x *= -1; 
                            projeteis[i].ricochetes++; 
                        }
                        if (projeteis[i].position.y <= caixaBatalha.y || projeteis[i].position.y >= caixaBatalha.y + caixaBatalha.height - projeteis[i].radius) { 
                            projeteis[i].speed.y *= -1; 
                            projeteis[i].ricochetes++; 
                        }
                    }
                }

                if (!CheckCollisionPointRec(projeteis[i].position, (Rectangle){-200 * scaleX, -200 * scaleY, larguraTelaAtual + 400 * scaleX, alturaTelaAtual + 400 * scaleY}) && (projeteis[i].type != PROJETIL_RICOCHETE || projeteis[i].ricochetes >= 2) ) { 
                    projeteis[i].active = false; 
                }

                if (CheckCollisionCircles(aluno.position, 8.0f * scaleX, projeteis[i].position, projeteis[i].radius * scaleX) && aluno.temporizadorInvencibilidade <= 0) {
                    aluno.sanidade--; 
                    aluno.temporizadorInvencibilidade = 1.2f; 
                    aluno.acabouDeDarDash = false;
                    PlaySound(fxDamage);
                    if (aluno.sanidade <= 0) { aluno.sanidade = 0; estadoAtualInterno = GAME_OVER_FASE1; }
                }
            }
        }

        for (int i=0; i<MAXIMO_PROJETEIS; i++) {
            if (acm.lasers[i].stage > 0) {
                acm.lasers[i].timer -= GetFrameTime();
                if (acm.lasers[i].timer <= 0) {
                    if (acm.lasers[i].stage == 1) { 
                        acm.lasers[i].stage = 2; 
                        acm.lasers[i].timer = 0.25f; 
                        PlaySound(fxLaserFire); 
                    } else { 
                        acm.lasers[i].stage = 0; 
                    }
                }

                if (acm.lasers[i].stage == 2 && CheckCollisionCircleRec(aluno.position, 8.0f * scaleX, acm.lasers[i].rect) && aluno.temporizadorInvencibilidade <= 0) {
                    aluno.sanidade--; 
                    aluno.temporizadorInvencibilidade = 1.2f; 
                    aluno.acabouDeDarDash = false;
                    PlaySound(fxDamage);
                    if (aluno.sanidade <= 0) { aluno.sanidade = 0; estadoAtualInterno = GAME_OVER_FASE1; }
                }
            }
        }
    }
}

static void DesenharJogoInterno(void){
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();
    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;

    BeginDrawing();
    ClearBackground(BLACK);

    if (texturaProfessor.id != 0) {
        DrawTexturePro(texturaProfessor,
                       (Rectangle){0, 0, (float)texturaProfessor.width, (float)texturaProfessor.height},
                       acm.corpoVisual,
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleRec(acm.corpoVisual, MAROON);
        DrawText("IMG?", (int)acm.corpoVisual.x + 20, (int)acm.corpoVisual.y + 20, 40, WHITE);
    }

    switch (estadoAtualInterno) {
        case TURNO_JOGADOR: 
        case MENU_AGIR: 
        case EXIBINDO_TEXTO:
            DrawRectangleLinesEx(caixaInterfaceInferior, 4.0f * scaleX, WHITE);
            if (estadoAtualInterno == TURNO_JOGADOR) DesenharTurnoJogadorInterna();
            else if (estadoAtualInterno == MENU_AGIR) DesenharMenuAgirInterna();
            else if (estadoAtualInterno == EXIBINDO_TEXTO) DesenharCaixaDialogoInterna();
            break;
        case TURNO_INIMIGO: 
            DesenharTurnoInimigoInterna(); 
            break;
        case GAME_OVER_FASE1: 
            DrawText("REPROVADO", larguraTelaAtual/2-MeasureText("REPROVADO", (int)(100*scaleY))/2, alturaTelaAtual/2-(int)(50*scaleY), (int)(100*scaleY), GRAY); 
            DrawText("[Z] Voltar ao Menu", larguraTelaAtual/2-MeasureText("[Z] Voltar ao Menu", (int)(30*scaleY))/2, alturaTelaAtual/2+(int)(100*scaleY), (int)(30*scaleY), WHITE);
            break;
        case VITORIA_FASE1: 
            DrawText("APROVADO!", larguraTelaAtual/2-MeasureText("APROVADO!", (int)(100*scaleY))/2, alturaTelaAtual/2-(int)(50*scaleY), (int)(100*scaleY), GOLD); 
            break;
        case TRANSICAO_FASE2:
            DrawRectangle(0, 0, larguraTelaAtual, alturaTelaAtual, Fade(WHITE, transicaoAlpha));
                        if(transicaoTimer > 1.0f){
                const char* txt = "AGORA VOCE ENTRA NO DESAFIO FINAL, BOA SORTE";
                int fontSize = (int)(40 * scaleY);
                DrawText(txt,
                         larguraTelaAtual/2 - MeasureText(txt, fontSize)/2,
                         alturaTelaAtual/2,
                         fontSize,
                         Fade(BLACK, transicaoAlpha));
            }

            break;
    }

    DesenharInterfaceInterna();
    EndDrawing();
}

static void DesenharInterfaceInterna(void){
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();
    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;

    DrawText(TextFormat("Questoes Resolvidas: %d/%d", acm.questoesResolvidas, QUESTOES_TOTAIS), 
             larguraTelaAtual/2-(int)(MeasureText("Questoes Resolvidas: 0/0", (int)(40*scaleY))/2), 
             (int)(45*scaleY), (int)(40*scaleY), WHITE);
    DrawText(TextFormat("SANIDADE: %d", aluno.sanidade), (int)(40*scaleX), alturaTelaAtual-(int)(60*scaleY), (int)(40*scaleY), WHITE);
}

static void DesenharTurnoJogadorInterna(void){
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();
    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;

    // Tamanho de fonte fixo (~30px em 1080p)
    int fontSizeText = (int)(30 * scaleY);
    int fontSizeButtons = (int)(40 * scaleY);

    DrawText(textoDialogo,
             (int)(caixaInterfaceInferior.x + 30 * scaleX),
             (int)(caixaInterfaceInferior.y + 30 * scaleY),
             fontSizeText,
             WHITE);

    DrawRectangleRec(botoesMenuGame[0], DARKGRAY);
    DrawText("AGIR",
             (int)(botoesMenuGame[0].x + 75 * scaleX),
             (int)(botoesMenuGame[0].y + 15 * scaleY),
             fontSizeButtons,
             WHITE);

    DrawRectangleRec(botoesMenuGame[1], DARKGRAY);
    DrawText("ITEM",
             (int)(botoesMenuGame[1].x + 75 * scaleX),
             (int)(botoesMenuGame[1].y + 15 * scaleY),
             fontSizeButtons,
             WHITE);

    DrawRectangleRec(botoesMenuGame[2], DARKGRAY);
    DrawText("PROVA",
             (int)(botoesMenuGame[2].x + 65 * scaleX),
             (int)(botoesMenuGame[2].y + 15 * scaleY),
             fontSizeButtons,
             WHITE);

    DrawRectangleRec(botoesMenuGame[3], DARKGRAY);
    DrawText("DESISTIR",
             (int)(botoesMenuGame[3].x + 35 * scaleX),
             (int)(botoesMenuGame[3].y + 15 * scaleY),
             fontSizeButtons,
             WHITE);

    DrawCircle((int)(botoesMenuGame[menuSel].x - 30 * scaleX),
               (int)(botoesMenuGame[menuSel].y + 35 * scaleY),
               (int)(12 * scaleX),
               RED);
}


static void DesenharMenuAgirInterna(void){
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();
    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;

    int fontSizeButtons = (int)(40 * scaleY);
    int fontSizeText = (int)(40 * scaleY);

    DrawRectangleRec(botoesMenuGame[0], DARKGRAY);
    DrawText("AGIR",
             (int)(botoesMenuGame[0].x + 75 * scaleX),
             (int)(botoesMenuGame[0].y + 15 * scaleY),
             fontSizeButtons,
             GRAY);

    DrawRectangleRec(botoesMenuGame[1], DARKGRAY);
    DrawText("ITEM",
             (int)(botoesMenuGame[1].x + 75 * scaleX),
             (int)(botoesMenuGame[1].y + 15 * scaleY),
             fontSizeButtons,
             GRAY);

    DrawRectangleRec(botoesMenuGame[2], DARKGRAY);
    DrawText("PROVA",
             (int)(botoesMenuGame[2].x + 65 * scaleX),
             (int)(botoesMenuGame[2].y + 15 * scaleY),
             fontSizeButtons,
             GRAY);

    DrawRectangleRec(botoesMenuGame[3], DARKGRAY);
    DrawText("DESISTIR",
             (int)(botoesMenuGame[3].x + 35 * scaleX),
             (int)(botoesMenuGame[3].y + 15 * scaleY),
             fontSizeButtons,
             GRAY);

    for(int i = 0; i < 3; i++){
        DrawText(opcoesMenuAgir[i],
                 (int)(caixaInterfaceInferior.x + 70 * scaleX),
                 (int)(caixaInterfaceInferior.y + 50 * scaleY + i * 70 * scaleY),
                 fontSizeText,
                 WHITE);
    }

    DrawCircle((int)(caixaInterfaceInferior.x + 40 * scaleX),
               (int)(caixaInterfaceInferior.y + 70 * scaleY + menuAgirSel * 70 * scaleY),
               (int)(10 * scaleX),
               RED);
}


static void DesenharCaixaDialogoInterna(void){
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();
    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;

    int fontSizeText = (int)(40 * scaleY);

    DrawText(textoDialogo,
             (int)(caixaInterfaceInferior.x + 30 * scaleX),
             (int)(caixaInterfaceInferior.y + 30 * scaleY),
             fontSizeText,
             WHITE);
}


static void DesenharTurnoInimigoInterna(void){
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();
    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;

    Color corCaixa = acm.turnoFinalAtivo ? GOLD : WHITE;
    if (acm.delayInicialTimer > 0) {
        if (fmodf(acm.delayInicialTimer*5, 0.5f) > 0.25f) corCaixa = RED;
    }

    if (acm.turnoFinalAtivo && (int)(acm.temporizadorTurno*2)%2 == 0) {
        DrawRectangleLinesEx(caixaBatalha, 6.0f * scaleX, RED);
    } else {
        DrawRectangleLinesEx(caixaBatalha, 4.0f * scaleX, corCaixa);
    }

    if (acm.padraoAtual == ATAQUE_LETRA_C && acm.estagioAtaque == 1) {
        DrawRingLines(acm.posLetraC, 100 * scaleX, 120 * scaleX, 45+acm.rotacaoLetraC, 315+acm.rotacaoLetraC, 30 * scaleX, Fade(DARKGRAY, 0.8f));
    }

    if (acm.padraoAtual == ATAQUE_FINAL) {
        for(int i=0; i<MAXIMO_PROJETEIS; i++) {
            if (acm.lasers[i].blasterPos.y > 0) {
                DrawRingLines(acm.lasers[i].blasterPos, 40 * scaleX, 50 * scaleX, 45+acm.lasers[i].rotation, 315+acm.lasers[i].rotation, 20 * scaleX, Fade(DARKGRAY, 0.9f));
            }
        }
    }

    if (aluno.acabouDeDarDash && aluno.temporizadorInvencibilidade > 0) {
        float a = (aluno.temporizadorInvencibilidade/0.3f);
        DrawCircleV(aluno.position, 15.0f * scaleX, Fade(SKYBLUE, 0.5f*a));
    }

    bool drawP = true;
    if (aluno.temporizadorInvencibilidade > 0 && !aluno.acabouDeDarDash) {
        if (fmodf(aluno.temporizadorInvencibilidade, 0.2f) < 0.1f) drawP = false;
    }
    if (drawP) DrawCircleV(aluno.position, 12.0f * scaleX, RED);

    if (acm.delayInicialTimer <= 0) {
        for(int i=0; i<PROJETEIS_MAXIMOS; i++) {
            if (projeteis[i].active) DrawCircleV(projeteis[i].position, projeteis[i].radius * scaleX, projeteis[i].color);
        }

        for(int i=0; i<MAXIMO_PROJETEIS; i++) {
            if (acm.lasers[i].stage > 0) {
                if(acm.lasers[i].stage == 1) {
                    DrawRectanglePro(acm.lasers[i].rect, (Vector2){0, acm.lasers[i].rect.height/2}, acm.lasers[i].rotation, Fade(YELLOW, 0.5f));
                } else {

                    DrawRectanglePro(acm.lasers[i].rect, (Vector2){0, acm.lasers[i].rect.height/2}, acm.lasers[i].rotation, WHITE);
                }
            }
        }
    }
}

static void AtivarProjetil(Vector2 p, Vector2 v, float r, Color c, TipoProjetil t, float life){
    for(int i=0; i<PROJETEIS_MAXIMOS; i++){
        if(!projeteis[i].active){
            projeteis[i] = (Projetil){p, v, r, life, 0, true, c, t};
            PlaySound(fxShoot);
            return;
        }
    }
}

static void CriarExplosaoEstelar(Vector2 p, float modVel){
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();
    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;
    int nF = 4;
    for(int i=0; i<nF; i++){
        float a = PI + (((float)i/(nF-1))*PI);
        Vector2 v = {cosf(a)*300.0f*modVel*scaleY, sinf(a)*300.0f*modVel*scaleY};
        AtivarProjetil(p, v, 8.0f * scaleX, Fade(WHITE, 0.8f), FRAGMENTO_ESTELAR, 0);
    }
}

static void ExecutarAtaqueChefe(float modVel){
    acm.temporizadorSubAtaque -= GetFrameTime();
    Vector2 pOrigem = {acm.corpoVisual.x+acm.corpoVisual.width/2, acm.corpoVisual.y+acm.corpoVisual.height};
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();
    float scaleX = larguraTelaAtual / GAME_LARGURA;
    float scaleY = alturaTelaAtual / GAME_ALTURA;

    switch(acm.padraoAtual){
        case RAJADA_RAPIDA:
            if(acm.temporizadorSubAtaque > 0) return;
            acm.temporizadorSubAtaque = 0.15f;
            float angR = atan2f(aluno.position.y-pOrigem.y, aluno.position.x-pOrigem.x);
            Vector2 velR = {cosf(angR)*660.0f*modVel*scaleY, sinf(angR)*660.0f*modVel*scaleY};
            AtivarProjetil(pOrigem, velR, 10.0f * scaleX, ORANGE, PROJETIL_NORMAL, 0);
            break;
        case ATAQUE_LETRA_C:
            if(acm.temporizadorSubAtaque > 0) return;
            if(acm.estagioAtaque == 0){
                acm.temporizadorSubAtaque = 1.0f;
                float anguloDoCentro = atan2f(aluno.position.y-(caixaBatalha.y+caixaBatalha.height/2), aluno.position.x-(caixaBatalha.x+caixaBatalha.width/2));
                acm.posLetraC.x = (caixaBatalha.x+caixaBatalha.width/2) + cosf(anguloDoCentro)*(500 * scaleX);
                acm.posLetraC.y = (caixaBatalha.y+caixaBatalha.height/2) + sinf(anguloDoCentro)*(500 * scaleY);
                acm.rotacaoLetraC = atan2f(aluno.position.y-acm.posLetraC.y, aluno.position.x-acm.posLetraC.y)*RAD2DEG;
                acm.estagioAtaque = 1;
                PlaySound(fxLaserCharge);
            }
            else if(acm.estagioAtaque == 1){
                acm.temporizadorSubAtaque = 1.8f;
                for(int i=0; i<MAXIMO_PROJETEIS; i++){
                    if(acm.lasers[i].stage == 0){
                        acm.lasers[i].stage = 1; 
                        acm.lasers[i].timer = 0.7f;
                        acm.lasers[i].rotation = acm.rotacaoLetraC;
                        acm.lasers[i].rect = (Rectangle){acm.posLetraC.x, acm.posLetraC.y, 1400 * scaleX, 70 * scaleY};
                        break;
                    }
                }
                acm.estagioAtaque = 2;
            }
            else{ 
                acm.posLetraC = (Vector2){-1000,-1000}; 
                acm.temporizadorSubAtaque = 0.8f; 
                acm.estagioAtaque = 0; 
            }
            break;
        case ESPIRAL_PULSANTE:
            if(acm.temporizadorSubAtaque > 0) return;
            acm.temporizadorSubAtaque = 0.04f;
            acm.anguloEspiral += 0.35f; 
            float r = 10.0f * (1.0f + sinf(GetTime()*20.0f)*0.4f);
            float velEsp = 450.0f * modVel * scaleY;
            Vector2 v1 = {cosf(acm.anguloEspiral)*velEsp, sinf(acm.anguloEspiral)*velEsp}; 
            AtivarProjetil(pOrigem, v1, r * scaleX, VIOLET, PROJETIL_NORMAL, 0);
            Vector2 v2 = {cosf(-acm.anguloEspiral)*velEsp, sinf(-acm.anguloEspiral)*velEsp}; 
            AtivarProjetil(pOrigem, v2, r * scaleX, SKYBLUE, PROJETIL_NORMAL, 0);
            if((int)(acm.temporizadorTurno*10)%15 == 0){
                float angVerde = atan2f(aluno.position.y-pOrigem.y, aluno.position.x-pOrigem.x);
                Vector2 v3 = {cosf(angVerde)*400.0f*modVel*scaleY, sinf(angVerde)*400.0f*modVel*scaleY}; 
                AtivarProjetil(pOrigem, v3, 15.0f * scaleX, GREEN, PROJETIL_NORMAL, 0);
            }
            break;
        case CHUVA_ESTELAR_ANGULAR:
            if(acm.temporizadorSubAtaque > 0) return;
            acm.temporizadorSubAtaque = 0.45f;
            Vector2 pE = {caixaBatalha.x + (rand()%(int)caixaBatalha.width), caixaBatalha.y - (20 * scaleY)};
            Vector2 vE = {(aluno.position.x - pE.x)/80.0f, 350.0f * modVel * scaleY};
            AtivarProjetil(pE, vE, 15.0f * scaleX, YELLOW, ESTRELA_MAE, 0);
            break;
        case ATAQUE_LANCA:
            if(acm.temporizadorSubAtaque > 0) return;
            acm.temporizadorSubAtaque = 0.25f;
            acm.contadorAtaque++;
            Vector2 pL; 
            int side = rand() % 4;
            if(side == 0) pL = (Vector2){caixaBatalha.x-(20 * scaleX), caixaBatalha.y+rand()%(int)caixaBatalha.height}; 
            else if(side == 1) pL = (Vector2){caixaBatalha.x+caixaBatalha.width+(20 * scaleX), caixaBatalha.y+rand()%(int)caixaBatalha.height};
            else if(side == 2) pL = (Vector2){caixaBatalha.x+rand()%(int)caixaBatalha.width, caixaBatalha.y-(20 * scaleY)}; 
            else pL = (Vector2){caixaBatalha.x+rand()%(int)caixaBatalha.width, caixaBatalha.y+caixaBatalha.height+(20 * scaleY)};
            AtivarProjetil(pL, (Vector2){0,0}, 10.0f * scaleX, YELLOW, LANCA_AVISO, 0.7f);
            break;
        case COMBO_CAOTICO:
            acm.anguloEspiral += 0.2f;
            float velCaos = 450.0f * modVel * scaleY;
            Vector2 vC1 = {cosf(acm.anguloEspiral)*velCaos, sinf(acm.anguloEspiral)*velCaos};
            AtivarProjetil(pOrigem, vC1, 8.0f * scaleX, VIOLET, PROJETIL_NORMAL, 0);
            if(acm.temporizadorSubAtaque <= 0){
                acm.temporizadorSubAtaque = 1.3f;
                for(int i=0; i<3; i++){
                    float angBase = atan2f(aluno.position.y-pOrigem.y, aluno.position.x-pOrigem.x);
                    float angOffset = ((float)i - 1.0f) * 0.2f;
                    Vector2 vel = {cosf(angBase+angOffset)*500.0f*modVel*scaleY, sinf(angBase+angOffset)*500.0f*modVel*scaleY};
                    AtivarProjetil(pOrigem, vel, 10.0f * scaleX, RED, PROJETIL_RICOCHETE, 0);
                }
            }
            break;
        case VARREDURA_LASER:
            if(acm.temporizadorSubAtaque > 0) return;
            acm.temporizadorSubAtaque = 0.6f;
            for(int i=0; i<MAXIMO_PROJETEIS; i++){
                if(acm.lasers[i].stage == 0){
                    float laserX = caixaBatalha.x + (50 * scaleX) + (rand() % ((int)(caixaBatalha.width - (100 * scaleX))));
                    acm.lasers[i].rect = (Rectangle){laserX, caixaBatalha.y, 60 * scaleX, caixaBatalha.height};
                    acm.lasers[i].stage = 1; 
                    acm.lasers[i].timer = 0.4f;
                    acm.lasers[i].rotation = 0;
                    PlaySound(fxLaserCharge);
                    break;
                }
            }
            break;
        case ATAQUE_FINAL:
            if(acm.estagioAtaque == 0){
                acm.anguloEspiral = 0; 
                acm.contadorAtaque = 0;
                for(int i=0; i<8; i++) acm.lasers[i] = (Laser){0};
                acm.estagioAtaque = 1;
            }
            acm.anguloEspiral += 0.015f;
            acm.temporizadorSubAtaque -= GetFrameTime();
            Vector2 centroCaixa = {caixaBatalha.x+caixaBatalha.width/2, caixaBatalha.y+caixaBatalha.height/2};
            for(int i=0; i<8; i++){
                float angBase = (float)i/8.0f * (PI*2);
                acm.lasers[i].blasterPos.x = centroCaixa.x + cosf(angBase+acm.anguloEspiral)*(600 * scaleX);
                acm.lasers[i].blasterPos.y = centroCaixa.y + sinf(angBase+acm.anguloEspiral)*(600 * scaleY);
                acm.lasers[i].rotation = atan2f(centroCaixa.y-acm.lasers[i].blasterPos.y, centroCaixa.x-acm.lasers[i].blasterPos.x)*RAD2DEG;
            }
            if(acm.temporizadorSubAtaque <= 0){
                int idx = acm.contadorAtaque % 8;
                acm.lasers[idx].stage = 1;
                acm.lasers[idx].timer = 0.6f;
                acm.lasers[idx].rect = (Rectangle){acm.lasers[idx].blasterPos.x, acm.lasers[idx].blasterPos.y, 1400 * scaleX, 80 * scaleY};
                acm.contadorAtaque++;
                acm.temporizadorSubAtaque = 0.4f;
                PlaySound(fxLaserFire);
            }
            break;
        default:
            break;
    }
}

static Sound CarregarSomSeguro(const char* nomeArquivo){
    Sound s = { 0 };
    const char* path1 = TextFormat("../resources/%s", nomeArquivo);
    if(FileExists(path1)) s = LoadSound(path1);
    if(s.frameCount == 0){
        const char* path2 = TextFormat("resources/%s", nomeArquivo);
        if(FileExists(path2)) s = LoadSound(path2);
    }
    if(s.frameCount > 0) TraceLog(LOG_INFO, "AUDIO SUCESSO: %s", nomeArquivo);
    else TraceLog(LOG_ERROR, "AUDIO FALHOU: %s nao encontrado!", nomeArquivo);
    return s;
}

static Music CarregarMusicaSegura(const char* nomeArquivo){
    Music m = { 0 };
    const char* path1 = TextFormat("../resources/%s", nomeArquivo);
    if(FileExists(path1)) m = LoadMusicStream(path1);
    if(m.frameCount == 0){
        const char* path2 = TextFormat("resources/%s", nomeArquivo);
        if(FileExists(path2)) m = LoadMusicStream(path2);
    }
    if(m.frameCount > 0) TraceLog(LOG_INFO, "MUSICA SUCESSO: %s", nomeArquivo);
    else TraceLog(LOG_ERROR, "MUSICA FALHOU: %s nao encontrado!", nomeArquivo);
    return m;
}

// ============================================================================
// FASE 2 - ASRIEL (BALANCEADA E MAIS FÁCIL)
// ============================================================================
void IniciarFase2(void){
    estadoFase2 = FASE2_GAMEPLAY;

    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();

    arena.x = (larguraTelaAtual - ARENA_WIDTH) / 2;
    arena.y = (alturaTelaAtual - ARENA_HEIGHT) / 2 + 50;
    arena.width = ARENA_WIDTH;
    arena.height = ARENA_HEIGHT;

    player.position = (Vector2){ arena.x + arena.width / 2, arena.y + arena.height / 2 };
    player.hp = aluno.sanidade; // Mantém a vida da Fase 1!
    if(player.hp <= 0) player.hp = 10; // Mínimo de vida
    player.invincibilityTimer = 0;
    player.isDamaged = false;

    for(int i = 0; i < MAX_STARS; i++) stars[i].active = false;
    for(int i = 0; i < MAX_LIGHTNINGS; i++) lightnings[i].active = false;
    for(int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;

    gonerActive = false;
    gonerCharging = false;
    gonerBeaming = false;
    gonerChargeTimer = 0;
    gonerBeamTimer = 0;

    texHeart = CreateHeartTexture();
    texStar = CreateStarTexture();
    texLightningWarning = CreateLightningWarningTexture();
    texLightningStrike = CreateLightningStrikeTexture();
    texChaosSaber = CreateChaosSaberTexture();
    texHyperGonerMouth = CreateHyperGonerMouthTexture();
    texHyperGonerCharge = CreateHyperGonerChargeTexture();
    texHyperGonerBeam = CreateHyperGonerBeamTexture();
    texBackgroundStar = CreateBackgroundStarTexture();

    // Musica da fase 2 (final boss)
    musicFase2 = CarregarMusicaSegura("finall_boss_music.mp3");
    if(musicFase2.frameCount > 0){
    PlayMusicStream(musicFase2);
    SetMusicVolume(musicFase2, 0.7f);
    }

    // Musica de vitoria final (carregada aqui, tocada depois)
    musicFinal = CarregarMusicaSegura("musica_final.mp3");

    InitAttack(STAR_BLAZING);
    attackDuration = 12.0f; // Mais tempo por ataque (era 10)
}

void AtualizarFase2(void){

    if(estadoFase2 != FASE2_GAMEPLAY) return;

    static int waveIndex = 0;

    attackTimer += GetFrameTime();

    attackTimer += GetFrameTime();
    if(attackTimer >= attackDuration){
        waveIndex++;

        switch(waveIndex){
            case 1:
            case 2:
                InitAttack(STAR_BLAZING);
                break;
            case 3:
            case 4:
                InitAttack(SHOCKER_BREAKER);
                break;
            case 5:
                InitAttack(CHAOS_SABER);
                break;
            case 6:
                InitAttack(STAR_BLAZING);
                break;
            case 7:
                InitAttack(SHOCKER_BREAKER);
                break;
            case 8:
                InitAttack(CHAOS_SABER);
                break;
            default:
                // Ciclo mais difícil, mas previsível
                if(waveIndex % 3 == 0) InitAttack(CHAOS_SABER);
                else if(waveIndex % 3 == 1) InitAttack(SHOCKER_BREAKER);
                else InitAttack(STAR_BLAZING);
                // Opcional: reduzir levemente a duração depois de muitas waves
                if(attackDuration > 8.0f) attackDuration -= 0.5f;
                break;
        }
    }


    // Movimentação mais rápida
    if(IsKeyDown(KEY_LEFT)) player.position.x -= PLAYER_SPEED * 1.3f;
    if(IsKeyDown(KEY_RIGHT)) player.position.x += PLAYER_SPEED * 1.3f;
    if(IsKeyDown(KEY_UP)) player.position.y -= PLAYER_SPEED * 1.3f;
    if(IsKeyDown(KEY_DOWN)) player.position.y += PLAYER_SPEED * 1.3f;

    if(player.position.x < arena.x) player.position.x = arena.x;
    if(player.position.x > arena.x + arena.width) player.position.x = arena.x + arena.width;
    if(player.position.y < arena.y) player.position.y = arena.y;
    if(player.position.y > arena.y + arena.height) player.position.y = arena.y + arena.height;

    if(player.invincibilityTimer > 0) player.invincibilityTimer -= GetFrameTime();
    else player.isDamaged = false;

    switch(currentAttack){
        case STAR_BLAZING: UpdateStarBlazing(); break;
        case SHOCKER_BREAKER: UpdateShockerBreaker(); break;
        case CHAOS_SABER: UpdateChaosSaber(); break;
        case HYPER_GONER: UpdateHyperGoner(); break;
        default: break;
    }

    UpdateParticles();

    if(screenShakeIntensity > 0){
        screenShakeIntensity -= GetFrameTime() * 10;
        if(screenShakeIntensity < 0) screenShakeIntensity = 0;
    }

    if(screenFlashTimer > 0) screenFlashTimer -= GetFrameTime();
    backgroundStarRotation += 10 * GetFrameTime();

    if(player.hp <= 0){
    estadoFase2 = FASE2_GAME_OVER;
    StopMusicStream(musicFase2);
    }

    // Vitória após 60 segundos
    static float tempoTotal = 0;
    tempoTotal += GetFrameTime();
    if(tempoTotal > 60.0f && estadoFase2 == FASE2_GAMEPLAY){
        estadoFase2 = FASE2_VITORIA;
        StopMusicStream(musicFase2);

    // Toca musica_final.mp3 na tela de vitória
    if(musicFinal.frameCount > 0){
        PlayMusicStream(musicFinal);
        SetMusicVolume(musicFinal, 0.8f);
    }
}

}

void DesenharFase2(void){
    float larguraTelaAtual = (float)GetScreenWidth();
    float alturaTelaAtual = (float)GetScreenHeight();

    BeginDrawing();
    Vector2 shakeOffset = {0};
    if(screenShakeIntensity > 0){
        shakeOffset.x = GetRandomValue(-screenShakeIntensity, screenShakeIntensity);
        shakeOffset.y = GetRandomValue(-screenShakeIntensity, screenShakeIntensity);
    }

    BeginMode2D((Camera2D){0,0, (Vector2){shakeOffset.x, shakeOffset.y}, 0, 1.0f});
    ClearBackground(BLACK);

        if(estadoFase2 != FASE2_GAME_OVER){
        float w = (float)texBackgroundStar.width;
        float h = (float)texBackgroundStar.height;

        // Camada 1 - verde
        DrawTexturePro(texBackgroundStar,
                       (Rectangle){0,0,w,h},
                       (Rectangle){larguraTelaAtual/2, alturaTelaAtual/2, w*4, h*4},
                       (Vector2){w, h},
                       backgroundStarRotation * 0.5f,
                       Fade(GREEN, 0.35f));

        // Camada 2 - rosa
        DrawTexturePro(texBackgroundStar,
                       (Rectangle){0,0,w,h},
                       (Rectangle){larguraTelaAtual/2 + 40*sinf(GetTime()*0.7f),
                                   alturaTelaAtual/2 + 30*cosf(GetTime()*0.9f),
                                   w*3.2f,
                                   h*3.2f},
                       (Vector2){w, h},
                       -backgroundStarRotation * 0.8f,
                       Fade(MAGENTA, 0.35f));

        // Camada 3 - azul
        DrawTexturePro(texBackgroundStar,
                       (Rectangle){0,0,w,h},
                       (Rectangle){larguraTelaAtual/2 + 60*cosf(GetTime()*1.3f),
                                   alturaTelaAtual/2 + 50*sinf(GetTime()*1.1f),
                                   w*3.8f,
                                   h*3.8f},
                       (Vector2){w, h},
                       backgroundStarRotation * 1.2f,
                       Fade(SKYBLUE, 0.25f));

        // Uma camada leve de "scanlines"
        for(int y = 0; y < (int)alturaTelaAtual; y += 4){
            DrawRectangle(0, y, (int)larguraTelaAtual, 1, Fade(BLACK, 0.15f));
        }
    }


    switch(estadoFase2){
        case FASE2_GAMEPLAY:
        {
            DrawRectangleLinesEx(arena, 3, WHITE);

            switch(currentAttack){
                case STAR_BLAZING: DrawStarBlazing(); break;
                case SHOCKER_BREAKER: DrawShockerBreaker(); break;
                case CHAOS_SABER: DrawChaosSaber(); break;
                case HYPER_GONER: DrawHyperGoner(); break;
                default: break;
            }

            Color playerColor = RED;
            if(player.invincibilityTimer > 0){
                if(fmod(player.invincibilityTimer, 0.2f) > 0.1f) playerColor = BLANK;
                else playerColor = RED;
            }
            else if(player.isDamaged){
                playerColor = BLANK;
            }

            DrawTexturePro(texHeart, (Rectangle){0,0, (float)texHeart.width, (float)texHeart.height},
                           (Rectangle){player.position.x, player.position.y, (float)texHeart.width, (float)texHeart.height},
                           (Vector2){(float)texHeart.width/2, (float)texHeart.height/2},
                           0, playerColor);

            DrawParticles();
            DrawText(TextFormat("HP: %02i", player.hp), 20, 20, 30, WHITE);
            DrawText("SOBREVIVA!", larguraTelaAtual/2 - MeasureText("SOBREVIVA!", 20)/2, 20, 20, YELLOW);

            const char *attackName = "";
            switch(currentAttack){
                case STAR_BLAZING: attackName = "Star Blazing"; break;
                case SHOCKER_BREAKER: attackName = "Shocker Breaker"; break;
                case CHAOS_SABER: attackName = "Chaos Saber"; break;
                case HYPER_GONER: attackName = "Hyper Goner"; break;
                default: break;
            }
            DrawText(attackName, larguraTelaAtual - MeasureText(attackName, 20) - 20, 20, 20, RAYWHITE);
        }
        break;

        case FASE2_GAME_OVER:
        {
            DrawText("GAME OVER", larguraTelaAtual/2 - MeasureText("GAME OVER", 60)/2, alturaTelaAtual/2 - 60, 60, RED);
            DrawText("Ainda não terminou...", larguraTelaAtual/2 - MeasureText("Ainda não terminou...", 20)/2, alturaTelaAtual/2 + 20, 20, WHITE);
            DrawText("[Z] Voltar ao Menu", larguraTelaAtual/2 - MeasureText("[Z] Voltar ao Menu", 20)/2, alturaTelaAtual - 50, 20, GRAY);
        }
        break;

                case FASE2_VITORIA:
        {
            DrawText("VITORIA!", larguraTelaAtual/2 - MeasureText("VITORIA!", 80)/2, alturaTelaAtual/2 - 80, 80, GOLD);
            DrawText("Voce foi aprovado na cadeira!",
                     larguraTelaAtual/2 - MeasureText("Voce foi aprovado na cadeira!", 30)/2,
                     alturaTelaAtual/2 + 20,
                     30,
                     WHITE);
            DrawText("[Z] Voltar ao Menu",
                     larguraTelaAtual/2 - MeasureText("[Z] Voltar ao Menu", 20)/2,
                     alturaTelaAtual - 50,
                     20,
                     YELLOW);
        }
        break;

    }

    if(screenFlashTimer > 0){
        DrawRectangle(0, 0, larguraTelaAtual, alturaTelaAtual, Fade(WHITE, screenFlashTimer / 0.1f));
    }

    EndMode2D();
    EndDrawing();
}

// ============================================================================
// TEXTURAS FASE 2
// ============================================================================
Texture2D CreateHeartTexture(void){
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
    for(int y = 0; y < 16; y++){
        for(int x = 0; x < 16; x++){
            if(heartShape[y][x] == 1) ImageDrawPixel(&heart, x, y, RED);
        }
    }
    Texture2D texture = LoadTextureFromImage(heart);
    UnloadImage(heart);
    return texture;
}

Texture2D CreateStarTexture(void){
    Image star = GenImageColor(24, 24, BLANK);
    int cx = 12, cy = 12;
    int outerR = 11, innerR = 5;
    Vector2 points[10];
    for(int i = 0; i < 5; i++){
        points[i * 2].x = cx + outerR * sinf(DEG2RAD * (i * 72));
        points[i * 2].y = cy - outerR * cosf(DEG2RAD * (i * 72));
        points[i * 2 + 1].x = cx + innerR * sinf(DEG2RAD * (i * 72 + 36));
        points[i * 2 + 1].y = cy - innerR * cosf(DEG2RAD * (i * 72 + 36));
    }
    for(int i = 0; i < 5; i++){
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

Texture2D CreateLightningWarningTexture(void){
    Image img = GenImageColor(20, ARENA_HEIGHT, BLANK);
    ImageDrawRectangle(&img, 0, 0, 20, ARENA_HEIGHT, Fade(YELLOW, 0.5f));
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

Texture2D CreateLightningStrikeTexture(void){
    Image img = GenImageColor(20, ARENA_HEIGHT, BLANK);
    ImageDrawRectangle(&img, 0, 0, 20, ARENA_HEIGHT, WHITE);
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

Texture2D CreateChaosSaberTexture(void){
    Image saber = GenImageColor(1920, 540, BLANK);
    Color startColor = {0, 121, 241, 150};
    Color endColor = {41, 178, 255, 150};
    for(int y = 0; y < saber.height; y++){
        float amount = (float)y / (float)saber.height;
        unsigned char r = (unsigned char)(startColor.r + (endColor.r - startColor.r) * amount);
        unsigned char g = (unsigned char)(startColor.g + (endColor.g - startColor.g) * amount);
        unsigned char b = (unsigned char)(startColor.b + (endColor.b - startColor.b) * amount);
        unsigned char a = (unsigned char)(startColor.a + (endColor.a - startColor.a) * amount);
        Color rowColor = { r, g, b, a };
        for(int x = 0; x < saber.width; x++){
            ImageDrawPixel(&saber, x, y, rowColor);
        }
    }
    ImageDrawRectangle(&saber, 0, saber.height / 2 - 5, saber.width, 10, Fade(WHITE, 0.7f));
    Texture2D texture = LoadTextureFromImage(saber);
    UnloadImage(saber);
    return texture;
}

Texture2D CreateHyperGonerMouthTexture(void){
    Image img = GenImageColor(200, 100, BLANK);
    int cx = 100; int cy = 50;
    int rx_outer = 100; int ry_outer = 50;
    int rx_inner = 80; int ry_inner = 40;
    for(int y = 0; y < img.height; y++){
        for(int x = 0; x < img.width; x++){
            if(((float)(x-cx)*(x-cx))/(rx_outer*rx_outer) + ((float)(y-cy)*(y-cy))/(ry_outer*ry_outer) <= 1){
                ImageDrawPixel(&img, x, y, GRAY);
            }
        }
    }
    for(int y = 0; y < img.height; y++){
        for(int x = 0; x < img.width; x++){
            if(((float)(x-cx)*(x-cx))/(rx_inner*rx_inner) + ((float)(y-cy)*(y-cy))/(ry_inner*ry_inner) <= 1){
                ImageDrawPixel(&img, x, y, BLACK);
            }
        }
    }
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

Texture2D CreateHyperGonerChargeTexture(void){
    Image img = GenImageColor(100, 100, BLANK);
    for(int i = 0; i < 50; i++){
        ImageDrawCircle(&img, 50, 50, i, Fade(BLUE, (float)i/50.0f));
    }
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

Texture2D CreateHyperGonerBeamTexture(void){
    Image img = GenImageColor(1920, 50, BLANK);
    Color startColor = Fade(SKYBLUE, 0.5f);
    Color endColor = WHITE;
    for(int y = 0; y < img.height; y++){
        float amount = (float)y / (float)img.height;
        unsigned char r = (unsigned char)(startColor.r + (endColor.r - startColor.r) * amount);
        unsigned char g = (unsigned char)(startColor.g + (endColor.g - startColor.g) * amount);
        unsigned char b = (unsigned char)(startColor.b + (endColor.b - startColor.b) * amount);
        unsigned char a = (unsigned char)(startColor.a + (endColor.a - startColor.a) * amount);
        Color rowColor = { r, g, b, a };
        for(int x = 0; x < img.width; x++){
            ImageDrawPixel(&img, x, y, rowColor);
        }
    }
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

Texture2D CreateBackgroundStarTexture(void){
    Image img = GenImageColor(100, 100, BLANK);
    ImageDrawCircle(&img, 50, 50, 45, Fade(BLUE, 0.1f));
    ImageDrawCircle(&img, 50, 50, 30, Fade(SKYBLUE, 0.15f));
    ImageDrawCircle(&img, 50, 50, 15, Fade(WHITE, 0.2f));
    Texture2D texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return texture;
}

// ============================================================================
// PARTÍCULAS
// ============================================================================
void EmitParticle(Vector2 pos, Vector2 speed, Color color, float radius, float lifetime){
    for(int i = 0; i < MAX_PARTICLES; i++){
        if(!particles[i].active){
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

void UpdateParticles(void){
    for(int i = 0; i < MAX_PARTICLES; i++){
        if(particles[i].active){
            particles[i].position.x += particles[i].speed.x * GetFrameTime();
            particles[i].position.y += particles[i].speed.y * GetFrameTime();
            particles[i].lifetime -= GetFrameTime();
            if(particles[i].lifetime <= 0){
                particles[i].active = false;
            }
        }
    }
}

void DrawParticles(void){
    for(int i = 0; i < MAX_PARTICLES; i++){
        if(particles[i].active){
            DrawCircleV(particles[i].position, particles[i].radius * (particles[i].lifetime / 1.0f), particles[i].color);
        }
    }
}

// ============================================================================
// ATAQUES FASE 2 (BALANCEADOS)
// ============================================================================
void InitAttack(AttackType newAttack){
    for(int i = 0; i < MAX_STARS; i++) stars[i].active = false;
    for(int i = 0; i < MAX_LIGHTNINGS; i++) lightnings[i].active = false;
    saberPhase = 0;
    gonerActive = false;
    gonerCharging = false;
    gonerBeaming = false;
    currentAttack = newAttack;
    attackTimer = 0.0f;
    attackDuration = 12.0f; // Mais tempo

    if(currentAttack == HYPER_GONER){
        gonerActive = true;
        gonerCharging = true;
        gonerChargeTimer = 3.0f;
        gonerBeamTimer = 0.5f;
        gonerPosition = (Vector2){ GetScreenWidth() / 2, GetScreenHeight() / 2 };
        attackDuration = 5.0f;
    }
}

void UpdateStarBlazing(void){
    // Reduzido de 3 para 1 estrela por ciclo
    if(fmod(attackTimer, 0.15f) < GetFrameTime() * 1.5f){
        for(int i = 0; i < MAX_STARS; i++){
            if(!stars[i].active){
                int side = GetRandomValue(0, 3);
                if(side == 0){ stars[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth()), -50 }; }
                else if(side == 1){ stars[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth()), GetScreenHeight() + 50 }; }
                else if(side == 2){ stars[i].position = (Vector2){ -50, GetRandomValue(0, GetScreenHeight()) }; }
                else{ stars[i].position = (Vector2){ GetScreenWidth() + 50, GetRandomValue(0, GetScreenHeight()) }; }

                stars[i].radius = 12;
                Vector2 target = (Vector2){ arena.x + arena.width / 2 + GetRandomValue(-50, 50), arena.y + arena.height / 2 + GetRandomValue(-50, 50) };
                Vector2 direction = { target.x - stars[i].position.x, target.y - stars[i].position.y };
                float length = sqrt(direction.x*direction.x + direction.y*direction.y);
                if(length == 0) length = 1;
                float speed = GetRandomValue(150, 250); // Reduzido de 200-400
                stars[i].speed = (Vector2){ (direction.x / length) * speed, (direction.y / length) * speed };
                stars[i].color = (Color){ GetRandomValue(100, 255), GetRandomValue(100, 255), GetRandomValue(100, 255), 255 };
                stars[i].rotation = GetRandomValue(0, 360);
                stars[i].active = true;
                break;
            }
        }
    }

    Rectangle playerRect = { player.position.x - texHeart.width/2, player.position.y - texHeart.height/2, (float)texHeart.width, (float)texHeart.height };
    for(int i = 0; i < MAX_STARS; i++){
        if(stars[i].active){
            stars[i].position.x += stars[i].speed.x * GetFrameTime();
            stars[i].position.y += stars[i].speed.y * GetFrameTime();
            stars[i].rotation += 300 * GetFrameTime();

            if(CheckCollisionCircleRec(stars[i].position, stars[i].radius, playerRect) && player.invincibilityTimer <= 0){
                player.hp--;
                player.invincibilityTimer = 1.5f; // Aumentado de 1.0
                player.isDamaged = true;
                screenShakeIntensity = 5.0f;
            }

            if(stars[i].position.x < -100 || stars[i].position.x > GetScreenWidth() + 100 || stars[i].position.y < -100 || stars[i].position.y > GetScreenHeight() + 100){
                stars[i].active = false;
            }
        }
    }
}

void DrawStarBlazing(void){
    for(int i = 0; i < MAX_STARS; i++){
        if(stars[i].active){
            DrawTexturePro(texStar, (Rectangle){0,0, (float)texStar.width, (float)texStar.height},
                           (Rectangle){stars[i].position.x, stars[i].position.y, stars[i].radius*2, stars[i].radius*2},
                           (Vector2){stars[i].radius, stars[i].radius},
                           stars[i].rotation, stars[i].color);
        }
    }
}

void UpdateShockerBreaker(void){
    static float spawnTimer = 0.0f;
    spawnTimer += GetFrameTime();
    if(spawnTimer >= 0.8f){ // Aumentado de 0.5
        for(int i = 0; i < MAX_LIGHTNINGS; i++){
            if(!lightnings[i].active){
                lightnings[i].active = true;
                lightnings[i].state = 1;
                lightnings[i].timer = 1.2f; // Aumentado de 0.8
                lightnings[i].warningRect = (Rectangle){ arena.x + GetRandomValue(0, arena.width - 20), arena.y, 20, arena.height };
                lightnings[i].color = (Color){ GetRandomValue(0,255), GetRandomValue(0,255), 255, 255 };
                spawnTimer = 0.0f;
                break;
            }
        }
    }

    Rectangle playerRect = { player.position.x - texHeart.width/2, player.position.y - texHeart.height/2, (float)texHeart.width, (float)texHeart.height };
    for(int i = 0; i < MAX_LIGHTNINGS; i++){
        if(lightnings[i].active){
            lightnings[i].timer -= GetFrameTime();
            if(lightnings[i].timer <= 0){
                if(lightnings[i].state == 1){
                    lightnings[i].state = 2;
                    lightnings[i].timer = 0.2f;
                    lightnings[i].strikeRect = lightnings[i].warningRect;
                    screenShakeIntensity = 7.0f;
                }
                else if(lightnings[i].state == 2){
                    lightnings[i].active = false;
                }
            }

            if(lightnings[i].state == 2 && CheckCollisionRecs(playerRect, lightnings[i].strikeRect) && player.invincibilityTimer <= 0){
                player.hp--;
                player.invincibilityTimer = 1.5f;
                player.isDamaged = true;
                screenShakeIntensity = 10.0f;
            }
        }
    }
}

void DrawShockerBreaker(void){
    for(int i = 0; i < MAX_LIGHTNINGS; i++){
        if(lightnings[i].active){
            if(lightnings[i].state == 1){
                DrawTexturePro(texLightningWarning, (Rectangle){0,0, (float)texLightningWarning.width, (float)texLightningWarning.height},
                               lightnings[i].warningRect, (Vector2){0,0}, 0, Fade(lightnings[i].color, lightnings[i].timer / 1.2f));
            }
            else if(lightnings[i].state == 2){
                DrawTexturePro(texLightningStrike, (Rectangle){0,0, (float)texLightningStrike.width, (float)texLightningStrike.height},
                               lightnings[i].strikeRect, (Vector2){0,0}, 0, WHITE);
            }
        }
    }
}

void UpdateChaosSaber(void){
    float saberSpeed = 800.0f; // Reduzido de 1000
    if(attackTimer > 1.2f && saberPhase == 0){ // Aumentado delay
        chaosSabers[0] = (Rectangle){ -GetScreenWidth(), arena.y - 50, GetScreenWidth() + arena.width, arena.height / 2 + 50 };
        chaosSabers[1] = (Rectangle){ GetScreenWidth(), arena.y + arena.height / 2, GetScreenWidth() + arena.width, arena.height / 2 + 50 };
        saberPhase = 1;
        screenFlashTimer = 0.1f;
    }
    else if(attackTimer > 4.0f && saberPhase == 1){
        chaosSabers[0] = (Rectangle){ -GetScreenWidth(), arena.y + arena.height / 2, GetScreenWidth() + arena.width, arena.height / 2 + 50 };
        chaosSabers[1] = (Rectangle){ GetScreenWidth(), arena.y - 50, GetScreenWidth() + arena.width, arena.height / 2 + 50 };
        saberPhase = 2;
        screenFlashTimer = 0.1f;
    }

    if(saberPhase == 1 || saberPhase == 2){
        chaosSabers[0].x += saberSpeed * GetFrameTime();
        chaosSabers[1].x -= saberSpeed * GetFrameTime();
        if(chaosSabers[0].x > arena.x - arena.width) screenShakeIntensity = 10.0f;
    }

    Rectangle playerRect = { player.position.x - texHeart.width/2, player.position.y - texHeart.height/2, (float)texHeart.width, (float)texHeart.height };
    if(saberPhase >= 1 && saberPhase <= 2 && player.invincibilityTimer <= 0){
        if(CheckCollisionRecs(playerRect, chaosSabers[0]) || CheckCollisionRecs(playerRect, chaosSabers[1])){
            player.hp -= 2; // Reduzido de 1 (mas ainda perigoso)
            player.invincibilityTimer = 1.5f;
            player.isDamaged = true;
            screenShakeIntensity = 15.0f;
        }
    }
}

void DrawChaosSaber(void){
    if(saberPhase >= 1 && saberPhase <= 2){
        DrawTexturePro(texChaosSaber, (Rectangle){0,0, (float)texChaosSaber.width, (float)texChaosSaber.height},
                       chaosSabers[0], (Vector2){0,0}, 0, WHITE);
        DrawTexturePro(texChaosSaber, (Rectangle){0,0, (float)texChaosSaber.width, (float)texChaosSaber.height},
                       chaosSabers[1], (Vector2){0,0}, 0, WHITE);
    }
}

void UpdateHyperGoner(void){
    gonerRotation += 50 * GetFrameTime();
    if(gonerCharging){
        gonerChargeTimer -= GetFrameTime();
        if(gonerChargeTimer <= 0){
            gonerCharging = false;
            gonerBeaming = true;
            screenFlashTimer = 0.2f;
            screenShakeIntensity = 20.0f;
            player.invincibilityTimer = 0;
        }
    }

    if(gonerBeaming){
        gonerBeamTimer -= GetFrameTime();
        Rectangle playerRect = { player.position.x - texHeart.width/2, player.position.y - texHeart.height/2, (float)texHeart.width, (float)texHeart.height };
        Rectangle beamRect = { gonerPosition.x, gonerPosition.y - 25, GetScreenWidth() - gonerPosition.x, 50 };
        if(CheckCollisionRecs(playerRect, beamRect) && player.invincibilityTimer <= 0){
            player.hp -= 3; // Reduzido de 5
            player.invincibilityTimer = 1.5f;
            player.isDamaged = true;
            screenShakeIntensity = 30.0f;
        }
        if(gonerBeamTimer <= 0){
            gonerBeaming = false;
            gonerActive = false;
        }
    }
}

void DrawHyperGoner(void){
    if(gonerActive){
        if(gonerCharging){
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

        if(gonerBeaming){
            DrawTexturePro(texHyperGonerBeam, (Rectangle){0,0, (float)texHyperGonerBeam.width, (float)texHyperGonerBeam.height},
                           (Rectangle){gonerPosition.x, gonerPosition.y, GetScreenWidth() - gonerPosition.x, (float)texHyperGonerBeam.height},
                           (Vector2){0, (float)texHyperGonerBeam.height/2},
                           0, WHITE);
            DrawRectangle(gonerPosition.x, gonerPosition.y - 40, GetScreenWidth() - gonerPosition.x, 80, Fade(SKYBLUE, 0.2f));
        }
    }
}
