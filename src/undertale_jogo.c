// undertale_jogo.c
/*******************************************************************************************
*
* Parceiro de Programação - Jogo "Recuperação Final" (Versão 8.6 - FINAL COMPLETA)
*
* Resumo das Correções:
* - Velocidade ajustada para resolução HD (Multiplicada por ScaleY).
* - Carregamento de Som "Blindado" (Tenta caminhos diferentes).
* - Carregamento de Imagem "Blindado".
* - Sistema de "Ressuscitação" de Áudio (Verifica se device está ready).
* - Todas as funções de desenho incluídas.
*
********************************************************************************************/
#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h> 
#include <stdio.h> // Para logs de debug

// Enum para telas (Compartilhado com menu.c)
typedef enum {
    SCREEN_MENU = 0,
    SCREEN_GAME,
    SCREEN_OPTIONS,
    SCREEN_CREDITS,
    SCREEN_EXIT
} GameScreen;

// Dimensões de Design
#define DESIGN_LARGURA_ECRA 1920 
#define DESIGN_ALTURA_ECRA 1080  

// Balanceamento
#define SANIDADE_MAX_JOGADOR 5000
#define QUESTOES_TOTAIS 0 
#define ESTUDOS_POR_QUESTAO 3
#define TEMPO_TURNO_INIMIGO 10.0f
#define MAX_PROJETEIS 500
#define MAX_LASERS 8

// Estados do Jogo Interno
typedef enum { TURNO_JOGADOR, MENU_AGIR, EXIBINDO_TEXTO, TURNO_INIMIGO, GAME_OVER, VITORIA } EstadoJogoInterno;

// Estruturas
typedef struct { Vector2 position; int sanidade; float temporizadorInvencibilidade; float dashCooldown; bool acabouDeDarDash; } Aluno;

typedef enum {
    PADRAO_NULO, RAJADA_RAPIDA, ATAQUE_LETRA_C, ESPIRAL_PULSANTE, CHUVA_ESTELAR_ANGULAR,
    ATAQUE_LANCA, COMBO_CAOTICO, VARREDURA_LASER, ATAQUE_FINAL
} PadraoAtaque;

typedef struct {
    Rectangle rect; float rotation; float timer; int stage;
    Vector2 blasterPos;
} Laser;

typedef struct {
    Rectangle corpoVisual; int estudosFeitos; int questoesResolvidas; bool podeResolverQuestao; bool podeEntregarProva; bool turnoFinalAtivo; int turnosTotaisPassados;
    PadraoAtaque padraoAtual; float temporizadorTurno; float delayInicialTimer; float anguloEspiral; float temporizadorSubAtaque;
    int estagioAtaque; int contadorAtaque; Vector2 posAtaqueSalva;
    Vector2 posLetraC; float rotacaoLetraC;
    Laser lasers[MAX_LASERS];
} Professor;

typedef enum { PROJETIL_NORMAL, ESTRELA_MAE, FRAGMENTO_ESTELAR, LANCA_AVISO, LANCA_PROJETIL, PROJETIL_RICOCHETE } TipoProjetil;

typedef struct { Vector2 position; Vector2 speed; float radius; float lifeTimer; int ricochetes; bool active; Color color; TipoProjetil type; } Projetil;

// Variáveis Estáticas do Jogo
static EstadoJogoInterno estadoAtualInterno;
static Aluno aluno;
static Professor acm;
static Projetil projeteis[MAX_PROJETEIS];
static Rectangle caixaBatalha, caixaInterfaceInferior;
static char textoDialogo[256];
static bool acaoTerminaTurno;
static Rectangle botoesMenu[4];
static int menuSel = 0;
static const char *opcoesMenuAgir[] = { "Descricao", "Estudar", "Resolver Questao" };
static int menuAgirSel = 0;

// Variáveis de Mídia (Sons e Imagens)
static Sound fxHit;
static Sound fxShoot;
static Sound fxLaserCharge;
static Sound fxLaserFire;
static Sound fxMenuSelectGame;
static Texture2D texturaProfessor;

// ==================== PROTÓTIPOS DE FUNÇÕES ====================
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

// --- HELPER: CARREGAMENTO SEGURO DE ÁUDIO ---
static Sound CarregarSomSeguro(const char* nomeArquivo) {
    Sound s = { 0 };
    // Tentativa 1: ../resources/ (Executável na pasta build)
    const char* path1 = TextFormat("../resources/%s", nomeArquivo);
    if (FileExists(path1)) s = LoadSound(path1);
    
    // Tentativa 2: resources/ (Executando da raiz/VSCode)
    if (s.frameCount == 0) {
        const char* path2 = TextFormat("resources/%s", nomeArquivo);
        if (FileExists(path2)) s = LoadSound(path2);
    }

    if (s.frameCount > 0) TraceLog(LOG_INFO, "AUDIO SUCESSO: %s", nomeArquivo);
    else TraceLog(LOG_ERROR, "AUDIO FALHOU: %s nao encontrado!", nomeArquivo);
    
    return s;
}

// ==================== FUNÇÃO PRINCIPAL (CHAMADA PELO MENU) ====================
void RunUndertaleGame(GameScreen *currentScreen) {
    SetTargetFPS(60);
    SetRandomSeed(time(NULL)); 

    // --- GARANTIA DE ÁUDIO ---
    if (!IsAudioDeviceReady()) InitAudioDevice();
    SetMasterVolume(1.0f); // Volume máximo

    // Carregamento de Sons
    fxHit = CarregarSomSeguro("hit.wav");
    fxShoot = CarregarSomSeguro("shoot.wav");
    fxLaserCharge = CarregarSomSeguro("laser_charge.wav");
    fxLaserFire = CarregarSomSeguro("laser_fire.wav");
    fxMenuSelectGame = CarregarSomSeguro("select.wav");

    // Carregamento de Imagem (Com fallback)
    texturaProfessor = LoadTexture("../resources/professor.png");
    if (texturaProfessor.id == 0) texturaProfessor = LoadTexture("resources/professor.png");

    // Resetar estado do jogo
    IniciarJogoInterno();

    // LOOP DO JOGO
    while (*currentScreen == SCREEN_GAME && !WindowShouldClose()) {
        // Segurança extra: se o áudio cair, tenta religar
        if (!IsAudioDeviceReady()) InitAudioDevice();

        AtualizarJogoInterno();
        DesenharJogoInterno();

        // Condição de Saída (Vitória/Derrota)
        if (estadoAtualInterno == GAME_OVER || estadoAtualInterno == VITORIA) {
            if (IsKeyPressed(KEY_Z)) {
                PlaySound(fxMenuSelectGame); 
                *currentScreen = SCREEN_MENU;
                
                // Limpeza
                UnloadSound(fxHit); UnloadSound(fxShoot); UnloadSound(fxLaserCharge); 
                UnloadSound(fxLaserFire); UnloadSound(fxMenuSelectGame); UnloadTexture(texturaProfessor);
                return;
            }
        }
    }

    // Se fechar a janela pelo X
    if (WindowShouldClose()) {
        *currentScreen = SCREEN_EXIT;
        UnloadSound(fxHit); UnloadSound(fxShoot); UnloadSound(fxLaserCharge); 
        UnloadSound(fxLaserFire); UnloadSound(fxMenuSelectGame); UnloadTexture(texturaProfessor);
    }
}

static void IniciarJogoInterno(void) {
    estadoAtualInterno=EXIBINDO_TEXTO; acaoTerminaTurno=false; menuSel=0; menuAgirSel=0;
    strcpy(textoDialogo, "* Professor ACM encara voce. A prova final definira seu futuro.");

    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    caixaBatalha=(Rectangle){currentScreenWidth/2.0f - (800.0f * scaleX)/2.0f, 320.0f * scaleY, 800.0f * scaleX, 400.0f * scaleY};
    caixaInterfaceInferior=(Rectangle){currentScreenWidth/2.0f - (1520.0f * scaleX)/2.0f, 750.0f * scaleY, 1520.0f * scaleX, 300.0f * scaleY};

    aluno=(Aluno){(Vector2){caixaBatalha.x+caixaBatalha.width/2.0f,caixaBatalha.y+caixaBatalha.height/2.0f},SANIDADE_MAX_JOGADOR,0,0,false};
    acm=(Professor){(Rectangle){currentScreenWidth/2.0f - (100.0f * scaleX), 100.0f * scaleY, 200.0f * scaleX, 200.0f * scaleY},0,0,false,false,false,0,PADRAO_NULO,TEMPO_TURNO_INIMIGO,0,0,0,0,0,(Vector2){-1000,-1000},0};

    float btnW = 220.0f * scaleX; float btnH = 70.0f * scaleY; float sp = 40.0f * scaleX;
    float paddingX = 40.0f * scaleX; float paddingY = (caixaInterfaceInferior.height - (btnH*2+sp))/2;
    float sX = caixaInterfaceInferior.x + caixaInterfaceInferior.width - (btnW*2+sp) - paddingX;
    float sY = caixaInterfaceInferior.y + paddingY;

    botoesMenu[0]=(Rectangle){sX,sY,btnW,btnH};
    botoesMenu[1]=(Rectangle){sX+btnW+sp,sY,btnW,btnH};
    botoesMenu[2]=(Rectangle){sX,sY+btnH+sp,btnW,btnH};
    botoesMenu[3]=(Rectangle){sX+btnW+sp,sY+btnH+sp,btnW,btnH};

    for(int i=0; i<MAX_PROJETEIS; i++) projeteis[i].active = false;
    for(int i=0; i<MAX_LASERS; i++) acm.lasers[i] = (Laser){0};
}

static void MudarParaTurnoInimigoInterno(void) {
    estadoAtualInterno = TURNO_INIMIGO;
    acm.temporizadorTurno = TEMPO_TURNO_INIMIGO;
    acm.delayInicialTimer = 1.5f;
    acm.anguloEspiral = 0; acm.temporizadorSubAtaque = 0;
    acm.estagioAtaque = 0; acm.contadorAtaque = 0;
    for(int i=0; i<MAX_PROJETEIS; i++) projeteis[i].active = false;
    for(int i=0; i<MAX_LASERS; i++) acm.lasers[i] = (Laser){0};
    
    int r = rand()%100;
    if(acm.turnoFinalAtivo) acm.padraoAtual = ATAQUE_FINAL;
    else if(r<20) acm.padraoAtual = RAJADA_RAPIDA;
    else if(r<40) acm.padraoAtual = ATAQUE_LETRA_C;
    else if(r<60) acm.padraoAtual = ESPIRAL_PULSANTE;
    else if(r<75) acm.padraoAtual = CHUVA_ESTELAR_ANGULAR;
    else if(r<90) acm.padraoAtual = ATAQUE_LANCA;
    else acm.padraoAtual = COMBO_CAOTICO;
}

static void AtualizarJogoInterno(void) {
    switch (estadoAtualInterno) {
        case TURNO_JOGADOR: AtualizarTurnoJogadorInterna(); break;
        case MENU_AGIR: AtualizarMenuAgirInterna(); break;
        case EXIBINDO_TEXTO: AtualizarExibindoTextoInterna(); break;
        case TURNO_INIMIGO: AtualizarTurnoInimigoInterna(); break;
        case GAME_OVER: break;
        case VITORIA: break;
    }
}

static void AtualizarTurnoJogadorInterna(void) {
    if (IsKeyPressed(KEY_RIGHT) && (menuSel == 0 || menuSel == 2)) { menuSel++; PlaySound(fxMenuSelectGame); }
    if (IsKeyPressed(KEY_LEFT) && (menuSel == 1 || menuSel == 3)) { menuSel--; PlaySound(fxMenuSelectGame); }
    if (IsKeyPressed(KEY_DOWN) && (menuSel == 0 || menuSel == 1)) { menuSel += 2; PlaySound(fxMenuSelectGame); }
    if (IsKeyPressed(KEY_UP) && (menuSel == 2 || menuSel == 3)) { menuSel -= 2; PlaySound(fxMenuSelectGame); }

    if (IsKeyPressed(KEY_Z)) {
        PlaySound(fxMenuSelectGame);
        acaoTerminaTurno = false;
        estadoAtualInterno = EXIBINDO_TEXTO;
        switch (menuSel) {
            case 0: estadoAtualInterno = MENU_AGIR; menuAgirSel = 0; break;
            case 1: strcpy(textoDialogo, "* Voce nao tem itens para usar."); break;
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
                estadoAtualInterno = GAME_OVER;
                break;
        }
    }
}

static void AtualizarMenuAgirInterna(void) {
    if (IsKeyPressed(KEY_DOWN)) { menuAgirSel++; PlaySound(fxMenuSelectGame); }
    if (IsKeyPressed(KEY_UP)) { menuAgirSel--; PlaySound(fxMenuSelectGame); }
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
        if (acaoTerminaTurno) MudarParaTurnoInimigoInterno();
        else estadoAtualInterno = TURNO_JOGADOR;
    }
}

static void AtualizarTurnoInimigoInterna(void) {
    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    if (aluno.temporizadorInvencibilidade > 0) aluno.temporizadorInvencibilidade -= GetFrameTime();
    if (aluno.dashCooldown > 0) aluno.dashCooldown -= GetFrameTime();
    if (IsKeyPressed(KEY_LEFT_SHIFT) && aluno.dashCooldown <= 0) {
        aluno.acabouDeDarDash = true;
        aluno.temporizadorInvencibilidade = 0.3f;
        aluno.dashCooldown = 1.0f;
    }
    
    // --- VELOCIDADE AJUSTADA (600 pixels por segundo * escala) ---
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

        for (int i=0; i<MAX_PROJETEIS; i++) if (projeteis[i].active) {
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
                    if (projeteis[i].position.x <= caixaBatalha.x || projeteis[i].position.x >= caixaBatalha.x + caixaBatalha.width - projeteis[i].radius) { projeteis[i].speed.x *= -1; projeteis[i].ricochetes++; }
                    if (projeteis[i].position.y <= caixaBatalha.y || projeteis[i].position.y >= caixaBatalha.y + caixaBatalha.height - projeteis[i].radius) { projeteis[i].speed.y *= -1; projeteis[i].ricochetes++; }
                }
            }
            if (!CheckCollisionPointRec(projeteis[i].position, (Rectangle){-200 * scaleX, -200 * scaleY, currentScreenWidth + 400 * scaleX, currentScreenHeight + 400 * scaleY}) && (projeteis[i].type != PROJETIL_RICOCHETE || projeteis[i].ricochetes >= 2) ) { projeteis[i].active = false; }
            if (CheckCollisionCircles(aluno.position, 8.0f * scaleX, projeteis[i].position, projeteis[i].radius * scaleX) && aluno.temporizadorInvencibilidade <= 0) {
                aluno.sanidade--; aluno.temporizadorInvencibilidade = 1.2f; aluno.acabouDeDarDash = false;
                PlaySound(fxHit);
                if (aluno.sanidade <= 0) { aluno.sanidade = 0; estadoAtualInterno = GAME_OVER; }
            }
        }
        for (int i=0; i<MAX_LASERS; i++) if (acm.lasers[i].stage > 0) {
            acm.lasers[i].timer -= GetFrameTime();
            if (acm.lasers[i].timer <= 0) {
                if (acm.lasers[i].stage == 1) { acm.lasers[i].stage = 2; acm.lasers[i].timer = 0.25f; PlaySound(fxLaserFire); }
                else { acm.lasers[i].stage = 0; }
            }
            if (acm.lasers[i].stage == 2 && CheckCollisionCircleRec(aluno.position, 8.0f * scaleX, acm.lasers[i].rect) && aluno.temporizadorInvencibilidade <= 0) {
                aluno.sanidade--; aluno.temporizadorInvencibilidade = 1.2f; aluno.acabouDeDarDash = false;
                PlaySound(fxHit);
                if (aluno.sanidade <= 0) { aluno.sanidade = 0; estadoAtualInterno = GAME_OVER; }
            }
        }
    }
}

static void DesenharJogoInterno(void){
    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    BeginDrawing();
    ClearBackground(BLACK);
    
    // Desenho da Imagem com Segurança
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
        case TURNO_JOGADOR: case MENU_AGIR: case EXIBINDO_TEXTO:
            DrawRectangleLinesEx(caixaInterfaceInferior, 4.0f * scaleX, WHITE);
            if (estadoAtualInterno == TURNO_JOGADOR) DesenharTurnoJogadorInterna();
            else if (estadoAtualInterno == MENU_AGIR) DesenharMenuAgirInterna();
            else if (estadoAtualInterno == EXIBINDO_TEXTO) DesenharCaixaDialogoInterna();
            break;
        case TURNO_INIMIGO: DesenharTurnoInimigoInterna(); break;
        case GAME_OVER: DrawText("REPROVADO", currentScreenWidth/2-MeasureText("REPROVADO", (int)(100*scaleY))/2, currentScreenHeight/2-(int)(50*scaleY), (int)(100*scaleY), GRAY); break;
        case VITORIA: DrawText("APROVADO!", currentScreenWidth/2-MeasureText("APROVADO!", (int)(100*scaleY))/2, currentScreenHeight/2-(int)(50*scaleY), (int)(100*scaleY), GOLD); break;
    }
    DesenharInterfaceInterna();
    EndDrawing();
}

static void DesenharInterfaceInterna(void){
    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    DrawText(TextFormat("Questoes Resolvidas: %d/%d", acm.questoesResolvidas, QUESTOES_TOTAIS), currentScreenWidth/2-(int)(MeasureText("Questoes Resolvidas: 0/0", (int)(40*scaleY))/2), (int)(45*scaleY), (int)(40*scaleY), WHITE);
    DrawText(TextFormat("SANIDADE: %d", aluno.sanidade), (int)(40*scaleX), currentScreenHeight-(int)(60*scaleY), (int)(40*scaleY), WHITE);
}

static void DesenharTurnoJogadorInterna(void){
    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    DrawText(textoDialogo, caixaInterfaceInferior.x+(int)(30*scaleX), caixaInterfaceInferior.y+(int)(30*scaleY), (int)(30*scaleY), WHITE);
    DrawRectangleRec(botoesMenu[0], DARKGRAY); DrawText("AGIR", botoesMenu[0].x+(int)(75*scaleX), botoesMenu[0].y+(int)(15*scaleY), (int)(40*scaleY), WHITE);
    DrawRectangleRec(botoesMenu[1], DARKGRAY); DrawText("ITEM", botoesMenu[1].x+(int)(75*scaleX), botoesMenu[1].y+(int)(15*scaleY), (int)(40*scaleY), WHITE);
    DrawRectangleRec(botoesMenu[2], DARKGRAY); DrawText("PROVA", botoesMenu[2].x+(int)(65*scaleX), botoesMenu[2].y+(int)(15*scaleY), (int)(40*scaleY), WHITE);
    DrawRectangleRec(botoesMenu[3], DARKGRAY); DrawText("DESISTIR", botoesMenu[3].x+(int)(35*scaleX), botoesMenu[3].y+(int)(15*scaleY), (int)(40*scaleY), WHITE);
    DrawCircle(botoesMenu[menuSel].x-(int)(30*scaleX), botoesMenu[menuSel].y+(int)(35*scaleY), (int)(12*scaleX), RED);
}

static void DesenharMenuAgirInterna(void){
    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    DrawRectangleRec(botoesMenu[0], DARKGRAY); DrawText("AGIR", botoesMenu[0].x+(int)(75*scaleX), botoesMenu[0].y+(int)(15*scaleY), (int)(40*scaleY), GRAY);
    DrawRectangleRec(botoesMenu[1], DARKGRAY); DrawText("ITEM", botoesMenu[1].x+(int)(75*scaleX), botoesMenu[1].y+(int)(15*scaleY), (int)(40*scaleY), GRAY);
    DrawRectangleRec(botoesMenu[2], DARKGRAY); DrawText("PROVA", botoesMenu[2].x+(int)(65*scaleX), botoesMenu[2].y+(int)(15*scaleY), (int)(40*scaleY), GRAY);
    DrawRectangleRec(botoesMenu[3], DARKGRAY); DrawText("DESISTIR", botoesMenu[3].x+(int)(35*scaleX), botoesMenu[3].y+(int)(15*scaleY), (int)(40*scaleY), GRAY);
    for(int i=0; i<3; i++){DrawText(opcoesMenuAgir[i], caixaInterfaceInferior.x+(int)(70*scaleX), caixaInterfaceInferior.y+(int)(50*scaleY)+i*(int)(70*scaleY), (int)(40*scaleY), WHITE);}
    DrawCircle(caixaInterfaceInferior.x+(int)(40*scaleX), caixaInterfaceInferior.y+(int)(70*scaleY)+menuAgirSel*(int)(70*scaleY), (int)(10*scaleX), RED);
}

static void DesenharCaixaDialogoInterna(void){
    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    DrawText(textoDialogo, caixaInterfaceInferior.x+(int)(30*scaleX), caixaInterfaceInferior.y+(int)(30*scaleY), (int)(40*scaleY), WHITE);
}

static void DesenharTurnoInimigoInterna(void){
    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    Color corCaixa = acm.turnoFinalAtivo ? GOLD : WHITE;
    if(acm.delayInicialTimer>0) if(fmodf(acm.delayInicialTimer*5,0.5f)>0.25f) corCaixa=RED;
    if(acm.turnoFinalAtivo && (int)(acm.temporizadorTurno*2)%2==0) DrawRectangleLinesEx(caixaBatalha, 6.0f * scaleX, RED);
    else DrawRectangleLinesEx(caixaBatalha, 4.0f * scaleX, corCaixa);

    if(acm.padraoAtual==ATAQUE_LETRA_C && acm.estagioAtaque==1) DrawRingLines(acm.posLetraC, 100 * scaleX, 120 * scaleX, 45+acm.rotacaoLetraC, 315+acm.rotacaoLetraC, 30 * scaleX, Fade(DARKGRAY, 0.8f));
    if(acm.padraoAtual==ATAQUE_FINAL) for(int i=0; i<MAX_LASERS; i++) if(acm.lasers[i].blasterPos.y > 0) DrawRingLines(acm.lasers[i].blasterPos, 40 * scaleX, 50 * scaleX, 45+acm.lasers[i].rotation, 315+acm.lasers[i].rotation, 20 * scaleX, Fade(DARKGRAY, 0.9f));

    if(aluno.acabouDeDarDash && aluno.temporizadorInvencibilidade > 0){ float a = (aluno.temporizadorInvencibilidade/0.3f); DrawCircleV(aluno.position, 15.0f * scaleX, Fade(SKYBLUE, 0.5f*a)); }
    bool drawP=true; if(aluno.temporizadorInvencibilidade > 0 && !aluno.acabouDeDarDash) if(fmodf(aluno.temporizadorInvencibilidade, 0.2f) < 0.1f) drawP=false;
    if(drawP) DrawCircleV(aluno.position, 12.0f * scaleX, RED);

    if(acm.delayInicialTimer <= 0){
        for(int i=0; i<MAX_PROJETEIS; i++) if(projeteis[i].active) DrawCircleV(projeteis[i].position, projeteis[i].radius * scaleX, projeteis[i].color);
        for(int i=0; i<MAX_LASERS; i++) if(acm.lasers[i].stage > 0){
            if(acm.lasers[i].stage==1) DrawRectanglePro(acm.lasers[i].rect, (Vector2){0, acm.lasers[i].rect.height/2}, acm.lasers[i].rotation, Fade(YELLOW, 0.5f));
            else DrawRectanglePro(acm.lasers[i].rect, (Vector2){0, acm.lasers[i].rect.height/2}, acm.lasers[i].rotation, WHITE);
        }
    }
}

static void AtivarProjetil(Vector2 p, Vector2 v, float r, Color c, TipoProjetil t, float life){
    for(int i=0; i<MAX_PROJETEIS; i++) if(!projeteis[i].active){
        projeteis[i] = (Projetil){p, v, r, life, 0, true, c, t};
        PlaySound(fxShoot);
        return;
    }
}

static void CriarExplosaoEstelar(Vector2 p, float modVel){
    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    int nF = 4;
    for(int i=0; i<nF; i++){
        float a = PI + (((float)i/(nF-1))*PI);
        Vector2 v = {cosf(a)*300.0f*modVel*scaleY, sinf(a)*300.0f*modVel*scaleY};
        AtivarProjetil(p, v, 8.0f * scaleX, Fade(WHITE, 0.8f), FRAGMENTO_ESTELAR, 0);
    }
}

static void ExecutarAtaqueChefe(float modVel) {
    acm.temporizadorSubAtaque-=GetFrameTime();
    Vector2 pOrigem = {acm.corpoVisual.x+acm.corpoVisual.width/2, acm.corpoVisual.y+acm.corpoVisual.height};

    float currentScreenWidth = (float)GetScreenWidth();
    float currentScreenHeight = (float)GetScreenHeight();
    float scaleX = currentScreenWidth / DESIGN_LARGURA_ECRA;
    float scaleY = currentScreenHeight / DESIGN_ALTURA_ECRA;

    switch(acm.padraoAtual){
        case RAJADA_RAPIDA:
            if(acm.temporizadorSubAtaque>0)return; acm.temporizadorSubAtaque=0.15f;
            float angR=atan2f(aluno.position.y-pOrigem.y,aluno.position.x-pOrigem.x);
            Vector2 velR={cosf(angR)*660.0f*modVel*scaleY, sinf(angR)*660.0f*modVel*scaleY};
            AtivarProjetil(pOrigem,velR,10.0f * scaleX,ORANGE,PROJETIL_NORMAL,0);
            break;
        case ATAQUE_LETRA_C:
            if(acm.temporizadorSubAtaque>0)return;
            if(acm.estagioAtaque==0){
                acm.temporizadorSubAtaque=1.0f;
                float anguloDoCentro = atan2f(aluno.position.y-(caixaBatalha.y+caixaBatalha.height/2), aluno.position.x-(caixaBatalha.x+caixaBatalha.width/2));
                acm.posLetraC.x = (caixaBatalha.x+caixaBatalha.width/2)+cosf(anguloDoCentro)*(500 * scaleX);
                acm.posLetraC.y = (caixaBatalha.y+caixaBatalha.height/2)+sinf(anguloDoCentro)*(500 * scaleY);
                acm.rotacaoLetraC=atan2f(aluno.position.y-acm.posLetraC.y,aluno.position.x-acm.posLetraC.y)*RAD2DEG;
                acm.estagioAtaque=1;
                PlaySound(fxLaserCharge);
            } else if(acm.estagioAtaque==1){
                acm.temporizadorSubAtaque=1.8f;
                for(int i=0;i<MAX_LASERS;i++)if(acm.lasers[i].stage==0){
                    acm.lasers[i].stage=1; acm.lasers[i].timer=0.7f;
                    acm.lasers[i].rotation=acm.rotacaoLetraC;
                    acm.lasers[i].rect=(Rectangle){acm.posLetraC.x,acm.posLetraC.y,1400 * scaleX,70 * scaleY};
                    break;
                }
                acm.estagioAtaque=2;
            } else { acm.posLetraC=(Vector2){-1000,-1000}; acm.temporizadorSubAtaque=0.8f; acm.estagioAtaque=0; }
            break;
        case ESPIRAL_PULSANTE:
            if(acm.temporizadorSubAtaque>0)return; acm.temporizadorSubAtaque=0.04f;
            acm.anguloEspiral+=0.35f; float r=10.0f*(1.0f+sinf(GetTime()*20.0f)*0.4f);
            float velEsp = 450.0f * modVel * scaleY;
            Vector2 v1={cosf(acm.anguloEspiral)*velEsp, sinf(acm.anguloEspiral)*velEsp}; AtivarProjetil(pOrigem,v1,r * scaleX,VIOLET,PROJETIL_NORMAL,0);
            Vector2 v2={cosf(-acm.anguloEspiral)*velEsp, sinf(-acm.anguloEspiral)*velEsp}; AtivarProjetil(pOrigem,v2,r * scaleX,SKYBLUE,PROJETIL_NORMAL,0);
            if((int)(acm.temporizadorTurno*10)%15==0){
                    float angVerde=atan2f(aluno.position.y-pOrigem.y,aluno.position.x-pOrigem.x);
                    Vector2 v3={cosf(angVerde)*400.0f*modVel*scaleY, sinf(angVerde)*400.0f*modVel*scaleY}; AtivarProjetil(pOrigem,v3,15.0f * scaleX,GREEN,PROJETIL_NORMAL,0);
            }
            break;
        case CHUVA_ESTELAR_ANGULAR:
            if(acm.temporizadorSubAtaque>0)return; acm.temporizadorSubAtaque=0.45f;
            Vector2 pE={caixaBatalha.x+(rand()%(int)caixaBatalha.width),caixaBatalha.y-(20 * scaleY)};
            Vector2 vE={(aluno.position.x-pE.x)/80.0f, 350.0f*modVel*scaleY};
            AtivarProjetil(pE,vE,15.0f * scaleX,YELLOW,ESTRELA_MAE,0);
            break;
        case ATAQUE_LANCA:
            if(acm.temporizadorSubAtaque>0)return;
            acm.temporizadorSubAtaque = 0.25f;
            acm.contadorAtaque++;
            Vector2 pL; int side = rand()%4;
            if(side==0)pL=(Vector2){caixaBatalha.x-(20 * scaleX),caixaBatalha.y+rand()%(int)caixaBatalha.height}; else if(side==1)pL=(Vector2){caixaBatalha.x+caixaBatalha.width+(20 * scaleX),caixaBatalha.y+rand()%(int)caixaBatalha.height};
            else if(side==2)pL=(Vector2){caixaBatalha.x+rand()%(int)caixaBatalha.width,caixaBatalha.y-(20 * scaleY)}; else pL=(Vector2){caixaBatalha.x+rand()%(int)caixaBatalha.width,caixaBatalha.y+caixaBatalha.height+(20 * scaleY)};
            AtivarProjetil(pL,(Vector2){0,0},10.0f * scaleX,YELLOW,LANCA_AVISO,0.7f);
            break;
        case COMBO_CAOTICO:
            acm.anguloEspiral+=0.2f; 
            float velCaos = 450.0f * modVel * scaleY;
            Vector2 vC1={cosf(acm.anguloEspiral)*velCaos, sinf(acm.anguloEspiral)*velCaos};
            AtivarProjetil(pOrigem,vC1,8.0f * scaleX,VIOLET,PROJETIL_NORMAL,0);
            if(acm.temporizadorSubAtaque<=0){
                acm.temporizadorSubAtaque=1.3f;
                for(int i=0;i<3;i++){
                    float angBase=atan2f(aluno.position.y-pOrigem.y,aluno.position.x-pOrigem.x);
                    float angOffset=((float)i-1.0f)*0.2f;
                    Vector2 vel={cosf(angBase+angOffset)*500.0f*modVel*scaleY, sinf(angBase+angOffset)*500.0f*modVel*scaleY};
                    AtivarProjetil(pOrigem,vel,10.0f * scaleX,RED,PROJETIL_RICOCHETE,0);
                }
            }
            break;
        case VARREDURA_LASER:
            if(acm.temporizadorSubAtaque > 0) return;
            acm.temporizadorSubAtaque = 0.6f;
            for(int i=0; i<MAX_LASERS; i++)if(acm.lasers[i].stage==0){
                float laserX = caixaBatalha.x + (50 * scaleX) + (rand() % ((int)(caixaBatalha.width - (100 * scaleX))));
                acm.lasers[i].rect = (Rectangle){laserX, caixaBatalha.y, 60 * scaleX, caixaBatalha.height};
                acm.lasers[i].stage = 1; acm.lasers[i].timer = 0.4f;
                acm.lasers[i].rotation = 0;
                PlaySound(fxLaserCharge);
                break;
            }
            break;
        case ATAQUE_FINAL:
            if(acm.estagioAtaque==0){
                acm.anguloEspiral=0; acm.contadorAtaque=0;
                for(int i=0;i<8;i++) acm.lasers[i] = (Laser){0};
                acm.estagioAtaque=1;
            }
            acm.anguloEspiral += 0.015f;
            acm.temporizadorSubAtaque -= GetFrameTime();
            Vector2 centroCaixa={caixaBatalha.x+caixaBatalha.width/2,caixaBatalha.y+caixaBatalha.height/2};
            for(int i=0;i<8;i++){
                float angBase=(float)i/8.0f*(PI*2);
                acm.lasers[i].blasterPos.x=centroCaixa.x+cosf(angBase+acm.anguloEspiral)*(600 * scaleX);
                acm.lasers[i].blasterPos.y=centroCaixa.y+sinf(angBase+acm.anguloEspiral)*(600 * scaleY);
                acm.lasers[i].rotation=atan2f(centroCaixa.y-acm.lasers[i].blasterPos.y,centroCaixa.x-acm.lasers[i].blasterPos.x)*RAD2DEG;
            }
            if(acm.temporizadorSubAtaque<=0){
                int idx=acm.contadorAtaque%8;
                acm.lasers[idx].stage=1;
                acm.lasers[idx].timer=0.6f;
                acm.lasers[idx].rect=(Rectangle){acm.lasers[idx].blasterPos.x,acm.lasers[idx].blasterPos.y,1400 * scaleX,80 * scaleY};
                acm.contadorAtaque++;
                acm.temporizadorSubAtaque=0.4f;
                PlaySound(fxLaserFire);
            }
            break;
    }
}