#include "color.hpp"
#include "lcd.hpp"
#include <wiringPi.h>
#include <unistd.h>
#include <iostream>
#include <atomic>

extern std::atomic<bool> leituraAtiva; // A declaracao continua necessaria

std::string detectColorHSV(int hue, int saturation, int value) {
    if (value < 30) return "PRETO";
    if (saturation < 20 && value > 220) return "BRANCO";
    if (saturation < 30 && value >= 30 && value <= 220) return "CINZA";
    if (saturation < 60 || value < 60) return "INDEFINIDO";
    if ((hue >= 0 && hue <= 8) || (hue >= 170 && hue <= 179)) return "VERMELHO";
    if (hue >= 18 && hue <= 30) return "AMARELO";
    if (hue >= 45 && hue <= 75) return "VERDE";
    if (hue >= 85 && hue <= 95) return "CIANO";
    if (hue >= 105 && hue <= 130) return "AZUL";
    if (hue >= 140 && hue <= 160) return "ROXO";
    return "INDEFINIDO";
}

bool processFrameHSV(cv::VideoCapture& cap, int& h, int& s, int& v) {
    cv::Mat frame;
    cap >> frame; // Captura um novo frame da camera

    if (frame.empty()) return false;

    cv::GaussianBlur(frame, frame, cv::Size(5, 5), 0);
    int x = frame.cols / 2, y = frame.rows / 2;
    cv::Rect roi(x - 5, y - 5, 10, 10);
    // Verificacoes de limites para o ROI para evitar crash se a camera for muito pequena ou falhar
    if (roi.x < 0 || roi.y < 0 || roi.width <= 0 || roi.height <= 0 ||
        roi.x + roi.width > frame.cols || roi.y + roi.height > frame.rows) {
        return false;
    }

    cv::Mat region = frame(roi), hsv;
    cv::cvtColor(region, hsv, cv::COLOR_BGR2HSV);
    cv::Scalar meanHSV = cv::mean(hsv);
    h = meanHSV[0]; s = meanHSV[1]; v = meanHSV[2];
    return true;
}

// Removido: void colorThread(cv::VideoCapture& cap) { ... }
// Esta funcao foi removida e sua logica movida para main.cpp
