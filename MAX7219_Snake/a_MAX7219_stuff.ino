// Файл со вспомогательными функциями для управления LED матрицей через библиотеку Faster_LedControl

// инвертировать номер модуля, если нужно
uint8_t moduleAddr(uint8_t number) {
  #ifdef INVERT_MODULES
    return (M7219_NUM - 1) - number;
  #else
    return number;
  #endif
}

// конвертировать глоб. коорд. X в адрес модуля и локальную коорд.
uint8_t xCoordToAddr(uint8_t* xCoord) {
  uint8_t module = *xCoord / 8; // находим номер модуля MAX7219
  module = moduleAddr(module); // конвертируем его в адрес
  *xCoord = *xCoord % 8; // находим номер столбца на указаном модуле
  #ifdef INVERT_X
    *xCoord = 7 - *xCoord; // переворачиваем коорд. X
  #endif
  return (module);
}

void matrixShutdown(bool state) { // задать режим всей LED матрицы
  for (uint8_t i = 0; i < M7219_NUM; i++)
    matrix.shutdown(moduleAddr(i), state);
}

void matrixIntensity(uint8_t intensity) { // задать яркость всей LED матрицы
  for (uint8_t i = 0; i < M7219_NUM; i++)
    matrix.setIntensity(moduleAddr(i), intensity);
}

void matrixClear() { // очистить всю LED матрицу
  for (uint8_t i = 0; i < M7219_NUM; i++)
    matrix.clearDisplay(moduleAddr(i));
}

// возвращает байт из текущего шрифта
byte getFontByte(uint8_t charNum, uint8_t byteNum) {
   // проверка на выход за пределы набора символов шрифта
   if (charNum > CHAR_TOTAL) return 0;
   
   byte aByte = pgm_read_byte_near( &( font[charNum][byteNum] ) );
   return aByte;
}

// вывод текста на матрицу
void setText (char text[], int xOffset = 0) {
  // очищаем колонки до текста
  if (xOffset > 0) clearColumns(0, xOffset - 1);
  
  int xCoord = xOffset; // счётчик колонок, задаётся со сдвигом по оси X
  uint16_t textLength = strlen(text);
  // перебор символов текста
  for (uint16_t charNum = 0; charNum < textLength; charNum++) {
    uint8_t aChar = text[charNum]; // взять символ из текста
    uint8_t charWidth = getFontByte(aChar, 0); // найти ширину символа
    if (charWidth == 0) continue; // пропуск символов нулевой ширины
    // вывести все байты символа последовательно
    for (uint8_t byteNum = 1; byteNum <= charWidth; byteNum++) {
      // проверка на выход за пределы матрицы
      if (xCoord >= 0 && xCoord <= matrixWidth) {
        // найти байт для столбца
        byte aByte = getFontByte(aChar, byteNum);
        setColumnM(xCoord, aByte);
      }
      xCoord++; // переход на след. байт символа
    }
    // добавить пробел между символами
    setColumnM(xCoord, 0);
    xCoord++;
  }
  
  // очищаем колонки после текста
  if (xCoord < matrixWidth) clearColumns(xCoord, matrixWidth - 1);
}

// очистка колонок матрицы
void clearColumns(uint8_t aColumn, uint8_t lastColumn) {
  for (aColumn; aColumn <= lastColumn; aColumn++)
    setColumnM(aColumn, 0);
}

uint16_t textWidth (char text[]) { // функция определения ширины текста в пикселях
  uint16_t totalWidth = 0;
  uint16_t textLength = strlen(text);
  
  // перебор символов текста
  for (uint16_t charNum = 0; charNum < textLength; charNum++) {
    uint8_t aChar = text[charNum]; // взять символ из текста
    uint8_t charWidth = getFontByte(aChar, 0); // найти ширину символа
    if (charWidth == 0) continue; // пропуск символов нулевой ширины
    totalWidth += charWidth + 1; // добавить ширину символа и пробела
  }

  return totalWidth - 1; // вернуть ширину без последнего пробела
}

void setScrollText(char *text, bool once) {
  strcpy(scrollBuffer, text);
  scrollState = true;
  scrollOnce = once;
  scrollOffset = matrixWidth + 1;
  scrollTxtWidth = textWidth(text);
}

void scrollText() { // вывод буфера на матрицу бегущей строкой
  if (!scrollState) return; // выйти из функции, если прокрутка не включена
  if (!scrollTimer.check()) return; // выйти из функции, если время крутить ещё не пришло

  // если это первая итерация прокрутки
  if ( scrollOffset == (matrixWidth + 1) ) {
    matrixClear(); // очистить матрицу
    scrollOffset--;
  // пока не достигнута последняя колонка
  } else if (scrollOffset != -scrollTxtWidth) {
    setText(scrollBuffer, scrollOffset);
    scrollOffset--;
  // если достигнута последняя колонка
  } else {
    matrixClear(); // очистить матрицу
    if (scrollOnce) scrollState = false;
    else scrollOffset = matrixWidth + 1;
  }
}

void alignText(char text[], bool center = true) { // 1 - по центру, 0 - по правому краю
  uint8_t tWidth = textWidth(text);

  // если текст шире матрицы
  if (tWidth > matrixWidth) setText(text);
  else {
    uint8_t xOffset;
    
    if (center) xOffset = (matrixWidth - tWidth) / 2;
    else xOffset = matrixWidth - tWidth;
    setText(text, xOffset);
  }
}

byte byteFlip (byte x) { // функция переворачивания byte на asm'е
  byte result;
 
    asm("mov __tmp_reg__, %[in] \n\t"
      "lsl __tmp_reg__  \n\t"   /* shift out high bit to carry */
      "ror %[out] \n\t"  /* rotate carry __tmp_reg__to low bit (eventually) */
      "lsl __tmp_reg__  \n\t"   /* 2 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 3 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 4 */
      "ror %[out] \n\t"
 
      "lsl __tmp_reg__  \n\t"   /* 5 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 6 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 7 */
      "ror %[out] \n\t"
      "lsl __tmp_reg__  \n\t"   /* 8 */
      "ror %[out] \n\t"
      : [out] "=r" (result) : [in] "r" (x));
      return(result);
}

// вывод столбца на матрицу (а не на модуль)
void setColumnM (uint8_t xCoord, byte aByte) {
  // получаем адрес модуля и локальную координату X
  uint8_t module = xCoordToAddr(&xCoord);
  #ifdef INVERT_Y
    aByte = byteFlip(aByte); // переворачиваем байт
  #endif
  matrix.setColumn(module, xCoord, aByte); // выводим байт на матрицу
}

void setLedM (uint8_t xCoord, uint8_t yCoord, bool state) { // вывод точки на матрицу (а не на модуль)
  // получаем адрес модуля и локальную координату X
  uint8_t module = xCoordToAddr(&xCoord);
  #ifdef INVERT_Y
    yCoord = 7 - yCoord; // переворачиваем координату Y
  #endif
  matrix.setLed(module, yCoord, xCoord, state); // выводим точку на матрицу
}
