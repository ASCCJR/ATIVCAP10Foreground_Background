#include "funcao_atividade_.h"
#include "funcoes_neopixel.h" 

// Variáveis globais para a fila e controle de eventos
int fila[TAM_FILA];
int inicio = 0;
int fim = 0;
int quantidade = 0;
int contador = 0; // Este é o "contador de eventos" que é zerado pelo joystick

// Array para armazenar o último tempo de toque de cada botão
absolute_time_t ultimo_toque[NUM_BOTOES];

// Mapeamento dos pinos dos botões e LEDs externos (conforme definido em funcao_atividade_.h)
const uint BOTOES[NUM_BOTOES] = {BOTAO_A, BOTAO_B, BOTAO_JOYSTICK};
const uint LEDS[NUM_BOTOES]   = {LED_VERMELHO, LED_AZUL, LED_VERDE};

// Variáveis de estado e sincronização para operação multicore
volatile bool eventos_pendentes[NUM_BOTOES] = {false, false, false}; 
volatile bool estado_leds[NUM_BOTOES] = {false, false, false};     
volatile bool core1_pronto = false;                               // Flag para sinalizar que o Core 1 está inicializado

// A variável 'index_neo' é definida como global em Atividade_5.c e declarada como 'extern'
// em funcoes_neopixel.h (que é incluído aqui). Ela rastreia o NeoPixel atual

// Função de callback para interrupções dos GPIOs (botões)
void gpio_callback(uint gpio, uint32_t events) {
    for (int i = 0; i < NUM_BOTOES; i++) {
        // Verifica se o GPIO da interrupção corresponde a um dos botões configurados
        // e se o evento é uma borda de descida (botão pressionado)
        if (gpio == BOTOES[i] && (events & GPIO_IRQ_EDGE_FALL)) {
            // Envia o índice do botão pressionado para a FIFO do multicore,
            // para ser processado pelo Core 1
            multicore_fifo_push_blocking(i);
        }
    }
}

// Função para inicializar um pino GPIO
void inicializar_pino(uint pino, uint direcao, bool pull_up, bool pull_down) {
    gpio_init(pino);
    gpio_set_dir(pino, direcao);

    // Configura resistores de pull-up/pull-down se o pino for de entrada
    if (direcao == GPIO_IN) {
        if (pull_up) {
            gpio_pull_up(pino);
        } else if (pull_down) {
            gpio_pull_down(pino);
        } else {
            gpio_disable_pulls(pino); // Desabilita pulls se nenhum for especificado
        }
    }
}

// Função executada no Core 1 para tratar os eventos dos botões e controlar os LEDs
void tratar_eventos_leds() {
    // Sinaliza para o Core 0 que o Core 1 está pronto e rodando
    core1_pronto = true;

    while (true) {
        // Aguarda bloqueando até que um dado (índice do botão) seja recebido da FIFO do multicore
        uint32_t id1 = multicore_fifo_pop_blocking();

        // Pausa para debounce do botão
        sleep_ms(DEBOUNCE_MS);

        // Confirma se o botão ainda está pressionado após o debounce
        if (!gpio_get(BOTOES[id1])) {
            // Lógica para ignorar a ação se múltiplos botões estiverem pressionados simultaneamente
            bool outro_pressionado = false;
            for (int i = 0; i < NUM_BOTOES; i++) {
                if (i != id1 && !gpio_get(BOTOES[i])) { // Verifica se outro botão (diferente do id1) está pressionado
                    outro_pressionado = true;
                    break;
                }
            }
            if (outro_pressionado) {
                // Se outro botão estiver pressionado, espera o botão id1 ser solto e ignora este evento
                while (!gpio_get(BOTOES[id1])) tight_loop_contents();
                continue; // Volta para o início do loop while(true)
            }

            // REQUISITO: "Manter toda lógica já existente de controle da fila com 
            // os BOTÕES A e B e o uso da Matriz NeoPixel"
            // A lógica abaixo para id1 == 0 (Botão A) e id1 == 1 (Botão B)
            // foi mantida conforme o original, controlando a fila e os NeoPixels
            if (id1 == 0 && index_neo < LED_COUNT) {  // Ação do BOTÃO A 
                uint8_t r = numero_aleatorio(1, 255);
                uint8_t g = numero_aleatorio(1, 255);
                uint8_t b = numero_aleatorio(1, 255);
                npAcendeLED(index_neo, r, g, b); // Acende o NeoPixel atual com cor aleatória
                index_neo++;                     // Avança para o próximo NeoPixel

                // Adiciona um novo valor à fila se houver espaço
                if (quantidade < TAM_FILA) {
                    fila[fim] = contador++; // Usa 'contador' para o valor e o incrementa
                    fim = (fim + 1) % TAM_FILA; // Avança o ponteiro de fim da fila (circular)
                    quantidade++;               // Incrementa o número de elementos na fila
                    imprimir_fila();            // Imprime o estado atual da fila
                }

            } else if (id1 == 1 && index_neo > 0) {   // Ação do BOTÃO B 
                index_neo--;                     // Recua para o NeoPixel anterior
                npAcendeLED(index_neo, 0, 0, 0); // Apaga o NeoPixel na nova posição de index_neo

                // Remove um valor do início da fila se ela não estiver vazia
                if (quantidade > 0) {
                    int valor = fila[inicio]; // Lê o valor (embora 'valor' não seja usado posteriormente aqui)
                    inicio = (inicio + 1) % TAM_FILA; // Avança o ponteiro de início da fila (circular)
                    quantidade--;                   // Decrementa o número de elementos na fila
                    imprimir_fila();                // Imprime o estado atual da fila
                }
            }
            // REQUISITO: "Inserir uma nova lógica de Controle no BOTÃO do JOYSTICK"
            // O bloco "else if (id1 == 2)" implementa esta nova lógica
            else if (id1 == 2) { // Ação do BOTÃO DO JOYSTICK 

                // REQUISITO Joystick: "AO PRESSIONAR O BOTÃO DO JOYSTICK, O SISTEMA ZERA O CONTADOR DE EVENTOS..."
                // REQUISITO Joystick: "...Os contadores que monitoram a fila e de eventos deverão ser ajustados de modo que tenhamos a ideia de uma nova fila que está sendo executada pela primeira vez"
                contador = 0; // Zera o 'contador de eventos' (variável 'contador') que fornece os valores para a fila

                // REQUISITO Joystick: "...Simulando uma fila vazia que está pronta para iniciar"
                // REQUISITO Joystick: "...Os contadores que monitoram a fila... deverão ser ajustados..."
                inicio = 0;     // Reseta o ponteiro de início da fila
                fim = 0;        // Reseta o ponteiro de fim da fila
                quantidade = 0; // Zera a quantidade de elementos na fila
                                // Estas três linhas preparam a fila como se fosse uma "nova fila"

                // REQUISITO Joystick: "...E APAGA TODA A MATRIZ DE NEOPIXEL"
                index_neo = 0;  // Reseta o índice que rastreia o LED NeoPixel atual, para apagar desde o início
                npClear();      // Preenche o buffer interno dos NeoPixels com zeros
                npWrite();      // Envia os dados do buffer para a fita NeoPixel, efetivamente apagando todos os LEDs
                
                imprimir_fila(); // Mostra no console que a fila foi zerada
            }

            // Atualiza os LEDs RGB externos com base no estado da fita NeoPixel
            // (Esta lógica foi mantida conforme o código original da atividade que estava no arquivo compactado)
            gpio_put(LED_VERMELHO, (index_neo == LED_COUNT)); // Acende LED Vermelho se todos os NeoPixels estiverem acesos
            gpio_put(LED_AZUL,     (index_neo == 0));        // Acende LED Azul se nenhum NeoPixel estiver aceso
            gpio_put(LED_VERDE,    0);                       // LED Verde permanece desligado

            // Espera o botão ser solto antes de processar o próximo evento
            while (!gpio_get(BOTOES[id1])) {
                tight_loop_contents(); // Otimização para loops de espera ativos
            }
        }
    }
}

// Função para imprimir o conteúdo atual da fila no console serial
void imprimir_fila() {
    printf("Fila [tam=%d]: ", quantidade);
    int i = inicio;
    for (int c = 0; c < quantidade; c++) {
        printf("%d ", fila[i]);
        i = (i + 1) % TAM_FILA; // Move para o próximo elemento na fila circular
    }
    printf("\n"); // Nova linha ao final da impressão da fila
}