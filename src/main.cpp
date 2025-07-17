#include "lcd.hpp"
#include "button.hpp"
#include "color.hpp"
#include "util.hpp" 
#include <wiringPi.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <atomic> // Necessario para a variavel atomica
#include <iostream>
#include <thread> // Esta inclusao pode ser removida ja que a thread nao e mais usada para 'colorThread'

extern std::atomic<bool> leituraAtiva; // Declaracao para a variavel atomica do botao
extern int lcd_fd; // Necessario para fechar o LCD em caso de erro da camera

int main() {
    std::cout << "Iniciando sistema de deteccao de cores...\n";

    if (wiringPiSetup() == -1) {
        std::cerr << "Erro: WiringPiSetup falhou!" << std::endl;
        std::cerr << "Por favor, reinicie o Raspberry Pi." << std::endl;
        return 1;
    }

    setupGPIO();
    setupButtonInterrupt(); // Esta interrupcao agora sera responsavel por controlar o loop principal

    lcdInit();
    showWelcomeMessages();

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) { 
        lcdClear();
        lcdPrint("ERRO: Camera!");
        lcdSetCursor(1, 0);
        lcdPrint("Reinicie a Rasp."); // Nova mensagem para o usuario
        std::cerr << "Erro ao iniciar camera." << std::endl;
        std::cerr << "Por favor, reinicie o Raspberry Pi." << std::endl;
        
        sleep(5); 
        if (lcd_fd != -1) close(lcd_fd); // Fecha os recursos do LCD antes de sair
        return 1;
    }

    showReadyMessage();

    bool lastState = !leituraAtiva; 
    bool boot = true; // Para a mensagem inicial de "Pressione o Botao para Iniciar"

    while (true) {
        if (boot) {
            lcdClear();
            lcdSetCursor(0, 0);
            lcdPrint("Pressione o");
            lcdSetCursor(1, 0);
            lcdPrint("Botao para Iniciar");
            usleep(1000000); // 1 segundo
            boot = false;
        }

        // Verifica se o estado da leitura mudou (ativo/pausado)
        if (leituraAtiva != lastState) {
            lastState = leituraAtiva;
            lcdClear();
            lcdSetCursor(0, 0);
            lcdPrint(leituraAtiva ? "Leitura ATIVA" : "Leitura PAUSADA");
            lcdSetCursor(1, 0);
            lcdPrint(leituraAtiva ? "Detectando..." : "Aperte o Botao");
            usleep(1500000); // 1.5 segundos para mostrar a mensagem
        }

        if (leituraAtiva) {
            int h, s, v;
            // AQUI: processFrameHSV ainda recebe 'cap' e faz a captura internamente
            if (processFrameHSV(cap, h, s, v)) { 
                std::string cor = detectColorHSV(h, s, v);
                lcdClear();
                lcdSetCursor(0, 0);
                lcdPrint("Cor Detectada:");
                lcdSetCursor(1, 0);
                lcdPrint(cor);
                digitalWrite(3, HIGH); // Liga o LED
                usleep(500000); // 0.5 segundos
                digitalWrite(3, LOW);  // Desliga o LED
                usleep(500000); // 0.5 segundos
            } else {
                lcdClear();
                lcdPrint("Aguardando Frame");
                lcdSetCursor(1, 0);
                lcdPrint("da Camera...");
                digitalWrite(3, LOW); // Garante que o LED esteja desligado
                usleep(500000); // 0.5 segundos
            }
        } else {
            digitalWrite(3, LOW); // Garante que o LED esteja desligado quando pausado
            usleep(300000); // Pequena pausa para economizar CPU quando pausado
        }
    }

    lcdClear(); lcdPrint("Encerrando...");
    cap.release();
    if (lcd_fd != -1) close(lcd_fd);
    std::cout << "Programa finalizado.\n";
    return 0;
}
