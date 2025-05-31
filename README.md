# ATIVCAP10Foreground_Background
# **Controle Interativo de LEDs NeoPixel e Fila com Raspberry Pi Pico**  

## **Visão Geral**  
Este projeto demonstra o controle interativo de uma fita de LEDs NeoPixel (WS2812B) e o gerenciamento de uma fila de software utilizando um **Raspberry Pi Pico**. O sistema empreende uma abordagem **multicore**, onde:  
- **Core 0**: Detecta interrupções dos botões e envia eventos para o Core 1.  
- **Core 1**: Processa os eventos, atualiza os LEDs NeoPixel e gerencia a fila.  
---

## **Funcionalidades Principais**  

### **1. Controle Sequencial de LEDs NeoPixel**  
| Botão       | GPIO | Ação |  
|-------------|------|------|  
| **Botão A** | 5    | Acende o próximo LED com uma cor aleatória (até 25 LEDs). |  
| **Botão B** | 6    | Apaga o último LED aceso. |  

### **2. Gerenciamento de Fila de Software**  
- **Botão A**: Adiciona um valor incremental (contador de eventos) ao final da fila.  
- **Botão B**: Remove o valor do início da fila.  
- **Estado da fila** (tamanho e elementos) é exibido no console serial após cada operação.  

### **3. Botão de Reset (Joystick - GPIO 22)**  
- **Ao pressionar**:  
  - Apaga **todos os LEDs NeoPixel**.  
  - **Zera o contador de eventos**.  
  - **Esvazia a fila**, simulando um reinício.  

### **4. LEDs Indicadores Externos**  
| LED   | GPIO | Estado Indicado |  
|-------|------|------------------|  
| **Vermelho** | 13   | Todos os 25 LEDs acesos. |  
| **Azul**     | 12   | Todos os LEDs apagados. |  
| **Verde**    | 11   | (Não utilizado no código atual) |  

### **5. Processamento Multicore (RP2040)**  
- **Core 0**:  
  - Configura interrupções nos botões (GPIO 5, 6, 22).  
  - Envia eventos para o Core 1 via **FIFO multicore**.  
- **Core 1**:  
  - Realiza **debounce** dos botões.  
  - Controla LEDs NeoPixel, fila e LEDs RGB externos.  

---

## **Estrutura do Código**  
| Arquivo | Descrição |  
|---------|-----------|  
| **Atividade_5.c** | Função `main()`, inicializa sistema e interrupções. |  
| **funcao_atividade_.c/h** | Lógica da fila, interrupções e tratamento de eventos. |  
| **funcoes_neopixel.c/h** | Controle da fita NeoPixel via PIO. |  
| **ws2818b.pio** | Máquina de estados para comunicação com os NeoPixels. |  
| **CMakeLists.txt** | Configuração do build com CMake. |  

# **Atendimento aos Requisitos Solicitados**

## **Requisito 1**  
**"Manter toda lógica já existente de controle da fila com os BOTÕES A e B e o uso da Matriz NeoPixel."**  

✅ **Cumprimento**:  
- A funcionalidade original dos botões **A (GPIO 5)** e **B (GPIO 6)** foi **preservada integralmente**.  
- **Botão A**:  
  - Acende o próximo LED NeoPixel na sequência com uma cor aleatória.  
  - Adiciona um valor incremental (`contador`) ao final da fila.  
- **Botão B**:  
  - Apaga o último LED aceso.  
  - Remove o valor do início da fila.  
- As estruturas de controle `if (id1 == 0)` (Botão A) e `else if (id1 == 1)` (Botão B) em `funcao_atividade_.c` **não sofreram alterações** em sua lógica fundamental.  

---

## **Requisito 2**  
**"Inserir uma nova lógica de Controle no BOTÃO do JOYSTICK:"**  

✅ **Cumprimento**:  
- Foi adicionado um novo bloco `else if (id1 == 2)` na função `tratar_eventos_leds` (em `funcao_atividade_.c`).  
- Esse bloco é executado **exclusivamente** quando o botão do joystick (**GPIO 22**) é pressionado.  

---

### **Requisito 2.1**  
**"AO PRESSIONAR O BOTÃO DO JOYSTICK, O SISTEMA ZERA O CONTADOR DE EVENTOS E APAGA TODA A MATRIZ DE NEOPIXEL."**  

✅ **Cumprimento**:  
1. **Zera o contador de eventos**:  
   ```c
   contador = 0;
   ```  
2. **Apaga todos os LEDs NeoPixel**:  
   - Reseta o índice do NeoPixel (`index_neo = 0`).  
   - Limpa o buffer dos LEDs (`npClear()`).  
   - Envia o comando para a fita física (`npWrite()`).  

---

### **Requisito 2.2**  
**"Simulando uma fila vazia que está pronta para iniciar. Os contadores que monitoram a fila e de eventos deverão ser ajustados de modo que tenhamos a ideia de uma nova fila que está sendo executada pela primeira vez."**  

✅ **Cumprimento**:  
- **Reseta a fila de software**:  
  ```c
  inicio = 0;
  fim = 0;
  quantidade = 0;
  ```  
- **Zera o contador de eventos** (`contador = 0`), reiniciando a numeração dos itens na fila.  
- **Esvazia a fila**, simulando um estado inicial como se o sistema estivesse sendo executado pela primeira vez.  

---

## **Observação**  
Todas as modificações foram implementadas **sem afetar o funcionamento original** dos botões A e B, atendendo estritamente aos requisitos solicitados pelo professor. O botão do joystick agora funciona como um **reset completo** do sistema, reiniciando tanto os LEDs NeoPixel quanto a fila de software.

---

## **Propósito**  
Projeto desenvolvido para fins educacionais durante a residência em **Sistemas Embarcados** pelo **Embarcatech**.  

---  
