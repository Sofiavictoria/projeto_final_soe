
// ======================== Arquivo: src/button/button.cpp ========================
#include "button.hpp"
#include <wiringPi.h>
#include <atomic>
#include <iostream>
#include <unistd.h>

#define LED 3
#define BOTAO 7

std::atomic<bool> leituraAtiva(false);
static unsigned long last_interrupt_time = 0;
const long DEBOUNCE_MS = 200;

void setupGPIO() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    pinMode(BOTAO, INPUT);
    pullUpDnControl(BOTAO, PUD_UP);
}

void buttonISR() {
    unsigned long now = millis();
    if (now - last_interrupt_time > DEBOUNCE_MS) {
        leituraAtiva = !leituraAtiva;
        last_interrupt_time = now;
    }
}

void setupButtonInterrupt() {
    if (wiringPiISR(BOTAO, INT_EDGE_FALLING, &buttonISR) < 0) {
        std::cerr << "Erro ao configurar interrupcao do botao" << std::endl;
        exit(1);
    }
}
