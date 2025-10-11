// main.c - Movendo um Círculo

#include "raylib.h" // Inclui a biblioteca Raylib

int main(void)
{
    // --- 1. INICIALIZAÇÃO ---

    // Configurações da janela
    const int screenWidth = 1440;
    const int screenHeight = 768;


    int plataformax1=0, plataformax2=1440;
    
    int plataforma_y1_start=10;
    int plataforma_y1_end=30;
    int plataforma_y2_start=500;
    int plataforma_y2_end=520;
    int plataforma_y3_start=990;
    int plataforma_y3_end=1010;

    Color PRETO= { 0, 0, 0, 255 };

    // Cria a janela
    InitWindow(screenWidth, screenHeight, "Movendo o Círculo!");

    // Variável para a posição do círculo.
    // Vector2 é uma estrutura da Raylib que guarda duas variáveis: 'x' e 'y'.
    // Vamos começar o círculo no meio da tela.
    Vector2 circlePosition = { (float)screenWidth / 2, (float)screenHeight / 2 };

    // Define a velocidade de movimento do círculo (em pixels por quadro)
    float moveSpeed = 5.0;

    // Define que o jogo deve tentar rodar a 60 quadros por segundo (FPS)
    SetTargetFPS(60);

    // --- 2. O LOOP PRINCIPAL ---
    // O loop continua rodando enquanto o usuário não fechar a janela
    while (!WindowShouldClose())
    {
        // --- 2.1. INPUT (Entrada) ---
        // Verifica se a tecla 'Seta para a Direita' está pressionada
        if (IsKeyDown(KEY_RIGHT))
        {
            circlePosition.x += moveSpeed; // Move o círculo para a direita
        }

        // Verifica se a tecla 'Seta para a Esquerda' está pressionada
        if (IsKeyDown(KEY_LEFT))
        {
            circlePosition.x -= moveSpeed; // Move o círculo para a esquerda
        }
        
        // Verifica se a tecla 'Seta para Baixo' está pressionada
        if (IsKeyDown(KEY_DOWN))
        {
            circlePosition.y += moveSpeed; // Move o círculo para baixo
        }

        // Verifica se a tecla 'Seta para Cima' está pressionada
        if (IsKeyDown(KEY_UP))
        {
            circlePosition.y -= moveSpeed; // Move o círculo para cima
        }

        // --- 2.2. UPDATE (Atualização) ---
        // Neste exemplo simples, a atualização já foi feita na seção de Input.
        // Em jogos mais complexos, aqui você atualizaria a lógica dos inimigos, física, etc.


        // --- 2.3. DRAW (Desenho) ---
        BeginDrawing(); // Prepara para começar a desenhar

            // Limpa o fundo da tela com uma cor.
            // DARKGRAY é uma cor pré-definida. Experimente trocar por BLUE, RED, etc.
            ClearBackground(WHITE);

            void DrawLine(int plataformax1, int plataforma_y1_start, int plataformax2, int plataforma_y1_end, Color PRETO);

            void DrawLine(int plataformax1, int plataforma_y2_start, int plataformax2, int plataforma_y2_end, Color PRETO);

            void DrawLine(int plataformax1, int plataforma_y3_start, int plataformax2, int plataforma_y3_end, Color PRETO);
              

            // Desenha o nosso círculo na tela.
            // Parâmetros: Posição (Vector2), Raio (tamanho), Cor.
            DrawCircleV(circlePosition, 50, BLACK); // Experimente trocar a cor para MAROON

        EndDrawing(); // Finaliza o desenho do quadro atual
    }

    // --- 3. FINALIZAÇÃO ---
    CloseWindow(); // Fecha a janela e libera os recursos
    return 0;
}