// Класс, реализующий простейший таймер на основе функции millis()
// Version 0.2. By iw0rm3r, 2020. https://github.com/iw0rm3r

#include "Arduino.h"

class iwTimer
{
  uint32_t lastMillis;
  uint32_t interval;

  public:
  iwTimer(uint32_t anInterval) {
    lastMillis = 0;
    interval = anInterval;
  }

// проверить, не прошёл ли заданный интервал
  bool check(bool setMillis = true) {
    uint32_t currMillis = millis();
    
    if ( (currMillis - lastMillis) >= interval) {
      // если нужно - фиксировать время срабатывания
      if (setMillis) lastMillis = currMillis;
      return true;
    } else return false;
  }
  
  void reSet() { // сбросить lastMillis
    lastMillis = 0;
  }
  
  void setInterval(uint32_t newInterval) { // изменить интервал таймера
    interval = newInterval;
  }

// задать время срабатывания (для отложенной обработки)
  void setMillis() {
    lastMillis = millis();
  }
};
