void setup() {
  #ifdef DEBUG
    Serial.begin(9600); // вывод отладочной информации по UART
  #endif
  
  // задать режимы работы пинов
  pinMode(BUZZ_PIN, OUTPUT);
  
  // инициализация LED матрицы
  matrixShutdown(false);
  matrixIntensity(MATRIX_BRIGHTNESS);
  matrixClear();

  // запуск контроллера
  controller.begin();
  while (!controller.connect()) {
    #ifdef DEBUG
      Serial.println(F("Контроллер SNES mini не обнаружен!"));
    #endif
    delay(1000);
  }

  // инициализация генератора случайных чисел
  randomSeed(analogRead(FREE_PIN));

  // "начальный экран"
  alignText("Змейка");

  #ifdef DEBUG
    Serial.println(F("Функция setup() завершена."));
  #endif
}

void loop() {
  // обработать ввод
  inputProcessing();

  switch (currMode) {
    case (INTRO):
      // ничего не делать
      break;
    case (GAME):
      mainLogic(); // обработать логику и отрисовать кадр
      break;
    case (LOSE):
      scrollText(); // прокручивать текст
      break;
    case (WIN):
      scrollText(); // прокручивать текст
      break;
  }
}
