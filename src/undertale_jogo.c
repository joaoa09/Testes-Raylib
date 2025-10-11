/*******************************************************************************************
*
* Parceiro de Programação - Jogo "Recuperação Final" (Versão 8.0 - Ataques Épicos)
*
* Descrição: Versão com ataques criativos, incluindo o "C Gaster Blaster" e
* um novo Ataque Final inspirado na luta contra o Sans.
*
********************************************************************************************/
#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define LARGURA_ECRA 1920
#define ALTURA_ECRA 1080
#define SANIDADE_MAX_JOGADOR 5000
#define QUESTOES_TOTAIS 0
#define ESTUDOS_POR_QUESTAO 3
#define TEMPO_TURNO_INIMIGO 10.0f
#define MAX_PROJETEIS 500
#define MAX_LASERS 8 // Aumentado para o ataque final

typedef enum { TURNO_JOGADOR, MENU_AGIR, EXIBINDO_TEXTO, TURNO_INIMIGO, GAME_OVER, VITORIA } EstadoJogo;

typedef struct { Vector2 position; int sanidade; float temporizadorInvencibilidade; float dashCooldown; bool acabouDeDarDash; } Aluno;

typedef enum {
    PADRAO_NULO, RAJADA_RAPIDA, ATAQUE_LETRA_C, ESPIRAL_PULSANTE, CHUVA_ESTELAR_ANGULAR,
    ATAQUE_LANCA, COMBO_CAOTICO, VARREDURA_LASER, ATAQUE_FINAL
} PadraoAtaque;

typedef struct {
    Rectangle rect; float rotation; float timer; int stage;
    Vector2 blasterPos; // Posição do "C" Blaster
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

static EstadoJogo estadoAtual; static Aluno aluno; static Professor acm; static Projetil projeteis[MAX_PROJETEIS];
static Rectangle caixaBatalha, caixaInterfaceInferior; static char textoDialogo[256]; static bool acaoTerminaTurno;
static Rectangle botoesMenu[4]; static int menuSel = 0;
static const char *opcoesMenuAgir[] = { "Descricao", "Estudar", "Resolver Questao" }; static int menuAgirSel = 0;

void IniciarJogo(void); void MudarParaTurnoInimigo(void); void AtualizarJogo(void); void AtualizarTurnoJogador(void);
void AtualizarMenuAgir(void); void AtualizarExibindoTexto(void); void AtualizarTurnoInimigo(void); void DesenharJogo(void);
void DesenharInterface(void); void DesenharTurnoJogador(void); void DesenharMenuAgir(void);
void DesenharCaixaDialogo(void); void DesenharTurnoInimigo(void);
void AtivarProjetil(Vector2 pos, Vector2 vel, float radius, Color color, TipoProjetil type, float life);
void ExecutarAtaqueChefe(float modificadorVelocidade); void CriarExplosaoEstelar(Vector2 pos, float modVel);

int main(void) {
    InitWindow(LARGURA_ECRA, ALTURA_ECRA, "Recuperação Final de IP"); ToggleFullscreen(); SetTargetFPS(60); IniciarJogo();
    while(!WindowShouldClose()){AtualizarJogo(); DesenharJogo();} CloseWindow(); return 0;
}

void IniciarJogo(void) {
    estadoAtual=EXIBINDO_TEXTO; acaoTerminaTurno=false; menuSel=0; menuAgirSel=0;
    strcpy(textoDialogo, "* Professor ACM encara voce. A prova final definira seu futuro.");
    caixaBatalha=(Rectangle){LARGURA_ECRA/2.0f-400,320,800,400}; caixaInterfaceInferior=(Rectangle){200,750,LARGURA_ECRA-400,300};
    aluno=(Aluno){(Vector2){caixaBatalha.x+caixaBatalha.width/2.0f,caixaBatalha.y+caixaBatalha.height/2.0f},SANIDADE_MAX_JOGADOR,0,0,false};
    acm=(Professor){(Rectangle){LARGURA_ECRA/2.0f-100,100,200,200},0,0,false,false,false,0};
    float btnW=220,btnH=70,sp=40,sX=caixaInterfaceInferior.x+caixaInterfaceInferior.width-(btnW*2+sp)-40,sY=caixaInterfaceInferior.y+(caixaInterfaceInferior.height-(btnH*2+sp))/2;
    botoesMenu[0]=(Rectangle){sX,sY,btnW,btnH};botoesMenu[1]=(Rectangle){sX+btnW+sp,sY,btnW,btnH};botoesMenu[2]=(Rectangle){sX,sY+btnH+sp,btnW,btnH};botoesMenu[3]=(Rectangle){sX+btnW+sp,sY+btnH+sp,btnW,btnH};
    for(int i=0;i<MAX_PROJETEIS;i++)projeteis[i].active=false; for(int i=0;i<MAX_LASERS;i++)acm.lasers[i].stage=0;
}

void MudarParaTurnoInimigo(void) {
    estadoAtual=TURNO_INIMIGO; if(acaoTerminaTurno)acm.turnosTotaisPassados++;
    if(acm.turnoFinalAtivo){acm.temporizadorTurno=TEMPO_TURNO_INIMIGO*2.0f; acm.padraoAtual=ATAQUE_FINAL;}
    else{
        acm.temporizadorTurno=TEMPO_TURNO_INIMIGO;
        switch(acm.turnosTotaisPassados){
            case 1: acm.padraoAtual=RAJADA_RAPIDA; break; case 2: acm.padraoAtual=ATAQUE_LETRA_C; break;
            case 3: acm.padraoAtual=ESPIRAL_PULSANTE; break; case 4: acm.padraoAtual=CHUVA_ESTELAR_ANGULAR; break;
            case 5: acm.padraoAtual=ATAQUE_LANCA; break; case 6: acm.padraoAtual=COMBO_CAOTICO; break;
            case 7: acm.padraoAtual=CHUVA_ESTELAR_ANGULAR; break; case 8: acm.padraoAtual=VARREDURA_LASER; break;
            case 9: acm.padraoAtual=ESPIRAL_PULSANTE; break; case 10: acm.padraoAtual=ATAQUE_LANCA; break;
            case 11: acm.padraoAtual=COMBO_CAOTICO; break; case 12: acm.padraoAtual=RAJADA_RAPIDA; break;
            default: acm.padraoAtual=RAJADA_RAPIDA; break;
        }
    }
    aluno.position=(Vector2){caixaBatalha.x+caixaBatalha.width/2,caixaBatalha.y+caixaBatalha.height/2};
    for(int i=0;i<MAX_PROJETEIS;i++)projeteis[i].active=false; for(int i=0;i<MAX_LASERS;i++)acm.lasers[i].stage=0;
    acm.delayInicialTimer=1.0f; acm.estagioAtaque=0; acm.contadorAtaque=0; acm.posLetraC=(Vector2){-1000,-1000};
}

void AtualizarJogo(void) {
    switch (estadoAtual) {
        case TURNO_JOGADOR: AtualizarTurnoJogador(); break;
        case MENU_AGIR: AtualizarMenuAgir(); break;
        case EXIBINDO_TEXTO: AtualizarExibindoTexto(); break;
        case TURNO_INIMIGO: AtualizarTurnoInimigo(); break;
        case GAME_OVER: case VITORIA: if (IsKeyPressed(KEY_Z)) IniciarJogo(); break;
    }
}

void AtualizarTurnoJogador(void) {
    if (IsKeyPressed(KEY_RIGHT) && (menuSel == 0 || menuSel == 2)) menuSel++;
    if (IsKeyPressed(KEY_LEFT) && (menuSel == 1 || menuSel == 3)) menuSel--;
    if (IsKeyPressed(KEY_DOWN) && (menuSel == 0 || menuSel == 1)) menuSel += 2;
    if (IsKeyPressed(KEY_UP) && (menuSel == 2 || menuSel == 3)) menuSel -= 2;

    if (IsKeyPressed(KEY_Z)) {
        acaoTerminaTurno = false;
        estadoAtual = EXIBINDO_TEXTO;
        switch (menuSel) {
            case 0: estadoAtual = MENU_AGIR; menuAgirSel = 0; break;
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
                estadoAtual = GAME_OVER;
                break;
        }
    }
}

void AtualizarMenuAgir(void) {
    if (IsKeyPressed(KEY_DOWN) && menuAgirSel < 2) menuAgirSel++;
    if (IsKeyPressed(KEY_UP) && menuAgirSel > 0) menuAgirSel--;
    if (IsKeyPressed(KEY_X)) { estadoAtual = TURNO_JOGADOR; }

    if (IsKeyPressed(KEY_Z)) {
        acaoTerminaTurno = true;
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
        estadoAtual = EXIBINDO_TEXTO;
    }
}

void AtualizarExibindoTexto(void) {
    if (IsKeyPressed(KEY_Z)) {
        if (acaoTerminaTurno) MudarParaTurnoInimigo();
        else estadoAtual = TURNO_JOGADOR;
    }
}

void AtualizarTurnoInimigo(void) {
    if ((acm.temporizadorTurno -= GetFrameTime()) <= 0) {
        if (acm.turnoFinalAtivo) estadoAtual = VITORIA;
        else { strcpy(textoDialogo, "* O professor prepara a proxima bateria de duvidas..."); acaoTerminaTurno = false; estadoAtual = EXIBINDO_TEXTO; }
        return;
    }

    if(aluno.temporizadorInvencibilidade > 0) aluno.temporizadorInvencibilidade -= GetFrameTime(); else aluno.acabouDeDarDash = false;
    if(aluno.dashCooldown > 0) aluno.dashCooldown -= GetFrameTime();

    Vector2 moveDir = {0,0};
    if(IsKeyDown(KEY_UP)) moveDir.y = -1; if(IsKeyDown(KEY_DOWN)) moveDir.y = 1; if(IsKeyDown(KEY_LEFT)) moveDir.x = -1; if(IsKeyDown(KEY_RIGHT)) moveDir.x = 1;
    if(moveDir.x != 0 || moveDir.y != 0) {
        float len = sqrtf(moveDir.x*moveDir.x + moveDir.y*moveDir.y);
        aluno.position.x += (moveDir.x / len) * 8.0f; aluno.position.y += (moveDir.y / len) * 8.0f;
    }
    if(IsKeyPressed(KEY_LEFT_SHIFT) && aluno.dashCooldown <= 0) {
        aluno.dashCooldown = 1.0f; aluno.acabouDeDarDash = true;
        if(moveDir.x == 0 && moveDir.y == 0) moveDir.x = 1;
        float len = sqrtf(moveDir.x*moveDir.x + moveDir.y*moveDir.y);
        aluno.position.x += (moveDir.x / len) * 220.0f; aluno.position.y += (moveDir.y / len) * 220.0f;
        aluno.temporizadorInvencibilidade = 0.3f;
    }

    if(aluno.position.x < caixaBatalha.x) aluno.position.x = caixaBatalha.x; if(aluno.position.y < caixaBatalha.y) aluno.position.y = caixaBatalha.y;
    if(aluno.position.x > caixaBatalha.x + caixaBatalha.width) aluno.position.x = caixaBatalha.x + caixaBatalha.width; if(aluno.position.y > caixaBatalha.y + caixaBatalha.height) aluno.position.y = caixaBatalha.y + caixaBatalha.height;

    if (acm.delayInicialTimer > 0) {
        acm.delayInicialTimer -= GetFrameTime();
    } else {
        float modVel = acm.turnoFinalAtivo ? 1.4f : 1.1f;
        ExecutarAtaqueChefe(modVel);
        
        for (int i=0; i<MAX_PROJETEIS; i++) if (projeteis[i].active) {
            projeteis[i].position.x += projeteis[i].speed.x; projeteis[i].position.y += projeteis[i].speed.y;
            if (projeteis[i].type == ESTRELA_MAE && projeteis[i].position.y >= caixaBatalha.y+caixaBatalha.height) {projeteis[i].active = false; CriarExplosaoEstelar(projeteis[i].position, modVel);}
            if (projeteis[i].type == LANCA_AVISO) {
                projeteis[i].lifeTimer -= GetFrameTime();
                if (projeteis[i].lifeTimer <= 0) {
                    projeteis[i].active = false;
                    float ang = atan2f(aluno.position.y - projeteis[i].position.y, aluno.position.x - projeteis[i].position.x);
                    Vector2 vel = { cosf(ang) * 15.0f * modVel, sinf(ang) * 15.0f * modVel };
                    AtivarProjetil(projeteis[i].position, vel, 10.0f, BLUE, LANCA_PROJETIL, 0);
                }
            }
            if (projeteis[i].type == PROJETIL_RICOCHETE) {
                if (projeteis[i].ricochetes < 2) {
                    if (projeteis[i].position.x <= caixaBatalha.x || projeteis[i].position.x >= caixaBatalha.x + caixaBatalha.width - projeteis[i].radius) { projeteis[i].speed.x *= -1; projeteis[i].ricochetes++; }
                    if (projeteis[i].position.y <= caixaBatalha.y || projeteis[i].position.y >= caixaBatalha.y + caixaBatalha.height - projeteis[i].radius) { projeteis[i].speed.y *= -1; projeteis[i].ricochetes++; }
                }
            }
            if (!CheckCollisionPointRec(projeteis[i].position, (Rectangle){-200, -200, LARGURA_ECRA+400, ALTURA_ECRA+400}) && (projeteis[i].type != PROJETIL_RICOCHETE || projeteis[i].ricochetes >= 2) ) { projeteis[i].active = false; }
            if (CheckCollisionCircles(aluno.position, 8.0f, projeteis[i].position, projeteis[i].radius) && aluno.temporizadorInvencibilidade <= 0) {
                aluno.sanidade--; aluno.temporizadorInvencibilidade = 1.2f; aluno.acabouDeDarDash = false;
                if (aluno.sanidade <= 0) { aluno.sanidade = 0; estadoAtual = GAME_OVER; }
            }
        }
        for (int i=0; i<MAX_LASERS; i++) if (acm.lasers[i].stage > 0) {
            acm.lasers[i].timer -= GetFrameTime();
            if (acm.lasers[i].timer <= 0) {
                if (acm.lasers[i].stage == 1) { acm.lasers[i].stage = 2; acm.lasers[i].timer = 0.25f; }
                else { acm.lasers[i].stage = 0; }
            }
            if (acm.lasers[i].stage == 2 && CheckCollisionCircleRec(aluno.position, 8.0f, acm.lasers[i].rect) && aluno.temporizadorInvencibilidade <= 0) {
                aluno.sanidade--; aluno.temporizadorInvencibilidade = 1.2f; aluno.acabouDeDarDash = false;
                if (aluno.sanidade <= 0) { aluno.sanidade = 0; estadoAtual = GAME_OVER; }
            }
        }
    }
}

void DesenharJogo(void){
    BeginDrawing();
    ClearBackground(BLACK);
    DrawRectangleRec(acm.corpoVisual, MAROON);
    switch (estadoAtual) {
        case TURNO_JOGADOR: case MENU_AGIR: case EXIBINDO_TEXTO:
            DrawRectangleLinesEx(caixaInterfaceInferior, 4.0f, WHITE);
            if (estadoAtual == TURNO_JOGADOR) DesenharTurnoJogador();
            else if (estadoAtual == MENU_AGIR) DesenharMenuAgir();
            else if (estadoAtual == EXIBINDO_TEXTO) DesenharCaixaDialogo();
            break;
        case TURNO_INIMIGO: DesenharTurnoInimigo(); break;
        case GAME_OVER: DrawText("REPROVADO", LARGURA_ECRA/2-MeasureText("REPROVADO",100)/2, ALTURA_ECRA/2-50,100,GRAY); break;
        case VITORIA: DrawText("APROVADO!", LARGURA_ECRA/2-MeasureText("APROVADO!",100)/2, ALTURA_ECRA/2-50,100,GOLD); break;
    }
    DesenharInterface();
    EndDrawing();
}

void DesenharInterface(void){
    DrawText(TextFormat("Questoes Resolvidas: %d/%d", acm.questoesResolvidas, QUESTOES_TOTAIS), LARGURA_ECRA/2-250, 45, 40, WHITE);
    DrawText(TextFormat("SANIDADE: %d", aluno.sanidade), 40, ALTURA_ECRA-60, 40, WHITE);
}

void DesenharTurnoJogador(void){
    DrawText(textoDialogo, caixaInterfaceInferior.x+30, caixaInterfaceInferior.y+30, 30, WHITE);
    DrawRectangleRec(botoesMenu[0], DARKGRAY); DrawText("AGIR", botoesMenu[0].x+75, botoesMenu[0].y+15, 40, WHITE);
    DrawRectangleRec(botoesMenu[1], DARKGRAY); DrawText("ITEM", botoesMenu[1].x+75, botoesMenu[1].y+15, 40, WHITE);
    DrawRectangleRec(botoesMenu[2], DARKGRAY); DrawText("PROVA", botoesMenu[2].x+65, botoesMenu[2].y+15, 40, WHITE);
    DrawRectangleRec(botoesMenu[3], DARKGRAY); DrawText("DESISTIR", botoesMenu[3].x+35, botoesMenu[3].y+15, 40, WHITE);
    DrawCircle(botoesMenu[menuSel].x-30, botoesMenu[menuSel].y+35, 12, RED);
}

void DesenharMenuAgir(void){
    DrawRectangleRec(botoesMenu[0], DARKGRAY); DrawText("AGIR", botoesMenu[0].x+75, botoesMenu[0].y+15, 40, GRAY);
    DrawRectangleRec(botoesMenu[1], DARKGRAY); DrawText("ITEM", botoesMenu[1].x+75, botoesMenu[1].y+15, 40, GRAY);
    DrawRectangleRec(botoesMenu[2], DARKGRAY); DrawText("PROVA", botoesMenu[2].x+65, botoesMenu[2].y+15, 40, GRAY);
    DrawRectangleRec(botoesMenu[3], DARKGRAY); DrawText("DESISTIR", botoesMenu[3].x+35, botoesMenu[3].y+15, 40, GRAY);
    for(int i=0; i<3; i++){DrawText(opcoesMenuAgir[i], caixaInterfaceInferior.x+70, caixaInterfaceInferior.y+50+i*70, 40, WHITE);}
    DrawCircle(caixaInterfaceInferior.x+40, caixaInterfaceInferior.y+70+menuAgirSel*70, 10, RED);
}

void DesenharCaixaDialogo(void){
    DrawText(textoDialogo, caixaInterfaceInferior.x+30, caixaInterfaceInferior.y+30, 40, WHITE);
}

void DesenharTurnoInimigo(void){
    Color corCaixa = acm.turnoFinalAtivo ? GOLD : WHITE;
    if(acm.delayInicialTimer>0) if(fmodf(acm.delayInicialTimer*5,0.5f)>0.25f) corCaixa=RED;
    if(acm.turnoFinalAtivo && (int)(acm.temporizadorTurno*2)%2==0) DrawRectangleLinesEx(caixaBatalha, 6.0f, RED);
    else DrawRectangleLinesEx(caixaBatalha, 4.0f, corCaixa);
    
    if(acm.padraoAtual==ATAQUE_LETRA_C && acm.estagioAtaque==1) DrawRingLines(acm.posLetraC, 100, 120, 45+acm.rotacaoLetraC, 315+acm.rotacaoLetraC, 30, Fade(DARKGRAY, 0.8f));
    if(acm.padraoAtual==ATAQUE_FINAL) for(int i=0; i<MAX_LASERS; i++) if(acm.lasers[i].blasterPos.y > 0) DrawRingLines(acm.lasers[i].blasterPos, 40, 50, 45+acm.lasers[i].rotation, 315+acm.lasers[i].rotation, 20, Fade(DARKGRAY, 0.9f));
    
    if(aluno.acabouDeDarDash && aluno.temporizadorInvencibilidade > 0){ float a = (aluno.temporizadorInvencibilidade/0.3f); DrawCircleV(aluno.position, 15.0f, Fade(SKYBLUE, 0.5f*a)); }
    bool drawP=true; if(aluno.temporizadorInvencibilidade > 0 && !aluno.acabouDeDarDash) if(fmodf(aluno.temporizadorInvencibilidade, 0.2f) < 0.1f) drawP=false;
    if(drawP) DrawCircleV(aluno.position, 12.0f, RED);
    
    if(acm.delayInicialTimer <= 0){
        for(int i=0; i<MAX_PROJETEIS; i++) if(projeteis[i].active) DrawCircleV(projeteis[i].position, projeteis[i].radius, projeteis[i].color);
        for(int i=0; i<MAX_LASERS; i++) if(acm.lasers[i].stage > 0){
            if(acm.lasers[i].stage==1) DrawRectanglePro(acm.lasers[i].rect, (Vector2){0, acm.lasers[i].rect.height/2}, acm.lasers[i].rotation, Fade(YELLOW, 0.5f));
            else DrawRectanglePro(acm.lasers[i].rect, (Vector2){0, acm.lasers[i].rect.height/2}, acm.lasers[i].rotation, WHITE);
        }
    }
}

void AtivarProjetil(Vector2 p, Vector2 v, float r, Color c, TipoProjetil t, float life){
    for(int i=0; i<MAX_PROJETEIS; i++) if(!projeteis[i].active){
        projeteis[i] = (Projetil){p, v, r, life, 0, true, c, t};
        return;
    }
}

void CriarExplosaoEstelar(Vector2 p, float modVel){
    int nF = 4;
    for(int i=0; i<nF; i++){
        float a = PI + (((float)i/(nF-1))*PI);
        Vector2 v = {cosf(a)*5.0f*modVel, sinf(a)*5.0f*modVel};
        AtivarProjetil(p, v, 8.0f, Fade(WHITE, 0.8f), FRAGMENTO_ESTELAR, 0);
    }
}

void ExecutarAtaqueChefe(float modVel) {
    acm.temporizadorSubAtaque-=GetFrameTime();
    Vector2 pOrigem = {acm.corpoVisual.x+acm.corpoVisual.width/2, acm.corpoVisual.y+acm.corpoVisual.height};

    switch(acm.padraoAtual){
        case RAJADA_RAPIDA:
            if(acm.temporizadorSubAtaque>0)return; acm.temporizadorSubAtaque=0.15f;
            float angR=atan2f(aluno.position.y-pOrigem.y,aluno.position.x-pOrigem.x);
            Vector2 velR={cosf(angR)*11.0f*modVel,sinf(angR)*11.0f*modVel};
            AtivarProjetil(pOrigem,velR,10.0f,ORANGE,PROJETIL_NORMAL,0);
            break;
            
        case ATAQUE_LETRA_C:
            if(acm.temporizadorSubAtaque>0)return;
            if(acm.estagioAtaque==0){
                acm.temporizadorSubAtaque=1.0f;
                float anguloDoCentro = atan2f(aluno.position.y-(caixaBatalha.y+caixaBatalha.height/2), aluno.position.x-(caixaBatalha.x+caixaBatalha.width/2));
                acm.posLetraC.x = (caixaBatalha.x+caixaBatalha.width/2)+cosf(anguloDoCentro)*500;
                acm.posLetraC.y = (caixaBatalha.y+caixaBatalha.height/2)+sinf(anguloDoCentro)*500;
                acm.rotacaoLetraC=atan2f(aluno.position.y-acm.posLetraC.y,aluno.position.x-acm.posLetraC.y)*RAD2DEG;
                acm.estagioAtaque=1;
            } else if(acm.estagioAtaque==1){
                acm.temporizadorSubAtaque=1.8f;
                for(int i=0;i<MAX_LASERS;i++)if(acm.lasers[i].stage==0){
                    acm.lasers[i].stage=1; acm.lasers[i].timer=0.7f;
                    acm.lasers[i].rotation=acm.rotacaoLetraC;
                    acm.lasers[i].rect=(Rectangle){acm.posLetraC.x,acm.posLetraC.y,1400,70};
                    break;
                }
                acm.estagioAtaque=2;
            } else { acm.posLetraC=(Vector2){-1000,-1000}; acm.temporizadorSubAtaque=0.8f; acm.estagioAtaque=0; }
            break;

        case ESPIRAL_PULSANTE:
            if(acm.temporizadorSubAtaque>0)return; acm.temporizadorSubAtaque=0.04f;
            acm.anguloEspiral+=0.35f; float r=10.0f*(1.0f+sinf(GetTime()*20.0f)*0.4f);
            Vector2 v1={cosf(acm.anguloEspiral)*8.5f*modVel,sinf(acm.anguloEspiral)*8.5f*modVel}; AtivarProjetil(pOrigem,v1,r,VIOLET,PROJETIL_NORMAL,0);
            Vector2 v2={cosf(-acm.anguloEspiral)*8.5f*modVel,sinf(-acm.anguloEspiral)*8.5f*modVel}; AtivarProjetil(pOrigem,v2,r,SKYBLUE,PROJETIL_NORMAL,0);
            if((int)(acm.temporizadorTurno*10)%15==0){
                 float angVerde=atan2f(aluno.position.y-pOrigem.y,aluno.position.x-pOrigem.x);
                 Vector2 v3={cosf(angVerde)*7.5f*modVel,sinf(angVerde)*7.5f*modVel}; AtivarProjetil(pOrigem,v3,15.0f,GREEN,PROJETIL_NORMAL,0);
            }
            break;

        case CHUVA_ESTELAR_ANGULAR:
            if(acm.temporizadorSubAtaque>0)return; acm.temporizadorSubAtaque=0.45f;
            Vector2 pE={caixaBatalha.x+(rand()%(int)caixaBatalha.width),caixaBatalha.y-20};
            Vector2 vE={(aluno.position.x-pE.x)/80.0f,6.0f*modVel};
            AtivarProjetil(pE,vE,15.0f,YELLOW,ESTRELA_MAE,0);
            break;

        case ATAQUE_LANCA:
            if(acm.temporizadorSubAtaque>0)return;
            acm.temporizadorSubAtaque = 0.25f;
            acm.contadorAtaque++;
            Vector2 pL; int side = rand()%4;
            if(side==0)pL=(Vector2){caixaBatalha.x-20,caixaBatalha.y+rand()%(int)caixaBatalha.height}; else if(side==1)pL=(Vector2){caixaBatalha.x+caixaBatalha.width+20,caixaBatalha.y+rand()%(int)caixaBatalha.height};
            else if(side==2)pL=(Vector2){caixaBatalha.x+rand()%(int)caixaBatalha.width,caixaBatalha.y-20}; else pL=(Vector2){caixaBatalha.x+rand()%(int)caixaBatalha.width,caixaBatalha.y+caixaBatalha.height+20};
            AtivarProjetil(pL,(Vector2){0,0},10.0f,YELLOW,LANCA_AVISO,0.7f);
            break;

        case COMBO_CAOTICO:
            acm.anguloEspiral+=0.2f; Vector2 vC1={cosf(acm.anguloEspiral)*8.5f*modVel,sinf(acm.anguloEspiral)*8.5f*modVel};
            AtivarProjetil(pOrigem,vC1,8.0f,VIOLET,PROJETIL_NORMAL,0);
            if(acm.temporizadorSubAtaque<=0){
                acm.temporizadorSubAtaque=1.3f;
                for(int i=0;i<3;i++){float angBase=atan2f(aluno.position.y-pOrigem.y,aluno.position.x-pOrigem.x);float angOffset=((float)i-1.0f)*0.2f;Vector2 vel={cosf(angBase+angOffset)*10.0f*modVel,sinf(angBase+angOffset)*10.0f*modVel};AtivarProjetil(pOrigem,vel,10.0f,RED,PROJETIL_RICOCHETE,0);}
            }
            break;

        case VARREDURA_LASER:
            if(acm.temporizadorSubAtaque > 0) return;
            acm.temporizadorSubAtaque = 0.6f;
            for(int i=0; i<MAX_LASERS; i++)if(acm.lasers[i].stage==0){
                float laserX = caixaBatalha.x + 50 + (rand() % ((int)caixaBatalha.width - 100));
                acm.lasers[i].rect = (Rectangle){laserX, caixaBatalha.y, 60, caixaBatalha.height};
                acm.lasers[i].stage = 1; acm.lasers[i].timer = 0.4f;
                acm.lasers[i].rotation = 0;
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
                acm.lasers[i].blasterPos.x=centroCaixa.x+cosf(angBase+acm.anguloEspiral)*600;
                acm.lasers[i].blasterPos.y=centroCaixa.y+sinf(angBase+acm.anguloEspiral)*600;
                acm.lasers[i].rotation=atan2f(centroCaixa.y-acm.lasers[i].blasterPos.y,centroCaixa.x-acm.lasers[i].blasterPos.x)*RAD2DEG;
            }
            if(acm.temporizadorSubAtaque<=0){
                int idx=acm.contadorAtaque%8;
                acm.lasers[idx].stage=1;
                acm.lasers[idx].timer=0.6f;
                acm.lasers[idx].rect=(Rectangle){acm.lasers[idx].blasterPos.x,acm.lasers[idx].blasterPos.y,1400,80};
                acm.contadorAtaque++;
                acm.temporizadorSubAtaque=0.4f;
            }
            break;
    }
}