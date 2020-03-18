// Змейка на Arduino Uno/nano с использованием LED матриц и контроллера SNES mini
// Version 0.1. By iw0rm3r, 2020. https://github.com/iw0rm3r
// --------------------БИБЛИОТЕКИ--------------------
// Faster_LedControl library by Vincent Fischer
// https://github.com/C0br4/Faster_LedControl
// --------------------------------------------------
// NintendoExtensionCtrl by David Madison
// https://github.com/dmadison/NintendoExtensionCtrl
// --------------------------------------------------
// PString by Mikal Hart
// http://arduiniana.org/libraries/PString/

#include <NintendoExtensionCtrl.h>  // поддержка I2C контроллеров Nintendo
#include <PString.h>          // печать в C-strings, вместо использования sptintf/String
#include "iwTimer.cpp"        // класс таймера на основе millis()
#include "LedControl.h"       // управление MAX7219 (ускоренная версия с HW SPI)
#include "5bite_rus_forum.h"  // файл шрифта с русскими буквами

// --------------------НАСТРОЙКИ--------------------
//#define DEBUG               // вывод по UART, ручное изменение скорости

#define BUZZ_PIN 6            // пин пищалки
#define CS_PIN 10             // пин chip select для LED матрицы
#define FREE_PIN A0           // свободный пин входа для random()

//#define INVERT_MODULES      // нужно ли инвертировать послед. модулей?
  #define INVERT_X            // нужно ли переворачивать всё по оси X?
//#define INVERT_Y            // нужно ли переворачивать всё по оси Y?
#define M7219_NUM 4           // число модулей MAX7219
#define MATRIX_BRIGHTNESS 1   // яркость матрицы (1-15)

#define SCORE_GOAL 10         // цель игры - число фруктов
#define MIN_REF_DELAY 50      // мин. задержка обновления, мс
#define MAX_REF_DELAY 1000    // макс. задержка обновления, мс
#define DEF_REF_DELAY 250     // начальная задержка обновления, мс
#define IN_FILTER_DELAY 200   // задержка фильтрации ввода, мс
#define SCROLL_DELAY 50       // задержка при прокрутке текста, мс
#define CHAR_TOTAL 255        // число символов в файле шрифта

// --------------------typedef'ы и struct'ы--------------------
typedef uint8_t fontArray[6]; // тип указателей на массивы шрифтов

// --------------------ENUMы--------------------
enum moveDirection {                        // направление движения
  STOP, UP, DOWN, LEFT, RIGHT
};
enum mode {                                 // режим работы
  INTRO, GAME, LOSE, WIN
};

// --------------------ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ--------------------
const uint8_t matrixWidth = M7219_NUM * 8;  // ширина матрицы в пикселях
const uint16_t refDelayStep = (DEF_REF_DELAY - MIN_REF_DELAY)
  / SCORE_GOAL;                             // шаг увелич. инт. обновления
bool pause = false;                         // флаг паузы
bool scrollState = false;                   // флаг прокрутки текста
bool scrollOnce;                            // флаг однократной прокрутки
int8_t headX;                               // координаты головы змейки
int8_t headY;                               // координаты головы змейки
int8_t tailX[100], tailY[100];              // массив координат хвоста
int8_t fruitX, fruitY;                      // координаты фрукта
uint8_t score;                              // счёт (число фруктов)
uint8_t tailLength;                         // длина хвоста
int16_t scrollOffset;                       // текущий сдвиг прокрутки текста
uint16_t refreshDelay;                      // интервал обновления матрицы
uint16_t scrollTxtWidth;                    // ширина текста бегущей строки
char scrollBuffer[256];                     // буфер бегущей строки
moveDirection snakeDir;                     // направление змейки
moveDirection lastFrameDir;                 // направление в пред. кадре
mode currMode = INTRO;                      // текущий режим работы
fontArray* font = _5bite_rus;               // массив шрифта

iwTimer refreshTimer(DEF_REF_DELAY);        // таймер обновления
iwTimer inputTimer(IN_FILTER_DELAY);        // таймер фильтрации ввода
iwTimer scrollTimer(SCROLL_DELAY);          // таймер задержки прокрутки текста
LedControl matrix(CS_PIN, M7219_NUM);       // LED матрица
SNESMiniController controller;              // контроллер SNES mini

void dummyFunc() { // чтобы Arduino IDE не ругался
  
}
