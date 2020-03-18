// Файл с функциями игровой логики

void newGame() { // подготовка к новой игре
  headX = matrixWidth / 2 - 1; // координаты головы змейки
  headY = 8 / 2 - 1; // координаты головы змейки
  tailLength = 0; // длина хвоста
  snakeDir = STOP; // направление змейки
  lastFrameDir = STOP; // сбросить последнее направление
  refreshDelay = DEF_REF_DELAY; // интервал обновления матрицы
  refreshTimer.setInterval(refreshDelay);
  score = 0; // счёт (число фруктов)
  newFruit();
  renderGame(); // отобразить первый кадр
  currMode = GAME;
  #ifdef DEBUG
    Serial.println(F("Новая игра!"));
  #endif
}

void newFruit() { // "создать" новый фрукт
  bool fruitCollision;
  do {
    fruitX = random(matrixWidth);
    fruitY = random(8);
    fruitCollision = false;
    // проверить, не пересекаются ли координаты со змейкой
    if ((fruitX == headX) && (fruitY == headY)) // голова змейки
      fruitCollision = true;
    else // проверка на пиксели хвоста
      for (uint8_t tailElement = 0; tailElement < tailLength; tailElement++) {
        if((fruitX == tailX[tailElement]) && (fruitY == tailY[tailElement])) {
          fruitCollision = true;
          break; // выйти из цикла, не продолжая проверку
        }
      }
  } while(fruitCollision);
}

void gameLost() { // при проигрыше
  #ifdef DEBUG
    Serial.println(F("Игра проиграна!"));
  #endif
  tone(BUZZ_PIN, 500, 500); // пропищать
  char message[128];
  // прослойка для конкатенации
  PString psMessage(message, sizeof(message));
  psMessage.print(F("Ты проиграл. Счет: "));
  psMessage.print(score); // добавить очки к строке
  psMessage.print(F(". START - начать заново"));
  setScrollText(message, false);
  currMode = LOSE;
  delay(2000);
}

void gameWon() { // при выигрыше
  #ifdef DEBUG
    Serial.println(F("Игра выиграна!"));
  #endif
  tone(BUZZ_PIN, 1500, 500); // пропищать
  char message[128];
  // прослойка для конкатенации
  PString psMessage(message, sizeof(message));
  psMessage.print(F("Ты выиграл, набрав "));
  psMessage.print(SCORE_GOAL); // добавить цель к строке
  psMessage.print(F(" очков! START - начать заново"));
  setScrollText(message, false);
  currMode = WIN;
  delay(2000);
}

void mainLogic() { // основная игровая логика
  // условия, при которых нет смысла продолжать исполнение функции
  if (!refreshTimer.check() || snakeDir == STOP || pause) return;
  
  // обработка столкновения с хвостом
  for (uint8_t tailElement = 0; tailElement < tailLength; tailElement++) {
    if((headX == tailX[tailElement]) && (headY == tailY[tailElement])) {
      gameLost();
      return;
    }
  }
  
  // обработка поедания фрукта
  if ( (headX == fruitX) && (headY == fruitY) ) { // при столкновении
    score++; // увеличить счётчик очков
    // проверка на выигрыш
    if (score == SCORE_GOAL) { gameWon(); return; }
    
    tone(BUZZ_PIN, 1000, 250); // пропищать
    newFruit(); // "создать" новый фрукт
    tailLength++; // увеличить длину хвоста
    refreshDelay -= refDelayStep; // ускорить игру
    refreshTimer.setInterval(refreshDelay);
    #ifdef DEBUG
      Serial.print(F("Длина хвоста: "));
      Serial.println(tailLength);
      Serial.print(F("Интервал обновления, мс: "));
      Serial.println(refreshDelay);
    #endif
  }
  
  lastFrameDir = snakeDir; // сохранить направление для фильтрации ввода

  // обновить координаты хвоста для движения змейки
  for (uint8_t tailElement = tailLength - 1; tailElement != 255; tailElement--) {
    tailX[tailElement] = tailX[tailElement - 1];
    tailY[tailElement] = tailY[tailElement - 1];
  }
  tailX[0] = headX; tailY[0] = headY;
  
  // обработка движения змейки
  switch (snakeDir) {
    case UP: headY++; break;
    case DOWN: headY--; break;
    case LEFT: headX--; break;
    case RIGHT: headX++; break;
  }
  if (headY >= 8) headY = 0;
  else if (headY < 0) headY = 8 - 1;
  if (headX >= matrixWidth) headX = 0;
  else if (headX < 0) headX = matrixWidth - 1;

  renderGame();
}

void renderGame() { // рендеринг игрового экрана
  for (uint8_t xCoord = 0; xCoord < matrixWidth; xCoord++) {
    for (uint8_t yCoord = 0; yCoord < 8; yCoord++) {
      bool currLedState = false;
      
      if ((xCoord == headX) && (yCoord == headY)) // голова змейки
        currLedState = true;
      else if ((xCoord == fruitX) && (yCoord == fruitY)) // фрукт
        currLedState = true;
      else // проверка на пиксели хвоста
        for (uint8_t tailElement = 0; tailElement < tailLength; tailElement++) {
          if((xCoord == tailX[tailElement]) && (yCoord == tailY[tailElement])) {
            currLedState = true;
            break; // выйти из цикла, не продолжая проверку
          }
        }

      setLedM(xCoord, yCoord, currLedState); // вывести на матрицу
    }
  }
}

#ifdef DEBUG
// ручное изменение интервала обновления (скорости игры)
void chRefDelay(int32_t diff) { 
  refreshDelay += diff;
  if (refreshDelay < MIN_REF_DELAY) refreshDelay = MAX_REF_DELAY;
  else if (refreshDelay > MAX_REF_DELAY) refreshDelay = MIN_REF_DELAY;
  refreshTimer.setInterval(refreshDelay);
  Serial.print(F("Интервал обновления изменён вручную, мс: "));
  Serial.println(refreshDelay);
}
#endif

void inputProcessing() { // ОБРАБОТКА ВВОДА
  // запрос данных с контроллера
  if (!controller.update()) {
    #ifdef DEBUG
      Serial.println(F("Контроллер не подключён!"));
    #endif
    delay(1000);
    return;
  }

  // обработка нажатий кнопок контроллера
  bool filterPassed = inputTimer.check(false); // флаг фильтра ввода
  bool buttonPressed = false;
  if (controller.buttonStart() && filterPassed) {
    if (currMode == GAME) {// в игре - пауза
      pause = !pause;
      #ifdef DEBUG
        Serial.print(F("Пауза: "));
        Serial.println(pause);
      #endif
    } else { // в других экранах - начать игру
      newGame();
    }
    buttonPressed = true;
  }
  // если игра не на паузе - обрабатывать ввод
  if (currMode == GAME && !pause) {
    if      (controller.dpadUp() && lastFrameDir != DOWN)
      snakeDir = UP;
    else if (controller.dpadDown() && lastFrameDir != UP)
      snakeDir = DOWN;
    else if (controller.dpadLeft() && lastFrameDir != RIGHT)
      snakeDir = LEFT;
    else if (controller.dpadRight() && lastFrameDir != LEFT)
      snakeDir = RIGHT;
  // в отладочном режиме доступно изменение скорости игры вручную
  #ifdef DEBUG
    else if (controller.buttonL() && filterPassed) { // + интервал обновления
      chRefDelay(100); buttonPressed = true; }
    else if (controller.buttonR() && filterPassed) { // - интервал обновления
      chRefDelay(-100); buttonPressed = true; }
  #endif
  }
  // если кнопка была нажата - зафиксировать время для фильтрации
  if (buttonPressed) inputTimer.setMillis();
}
