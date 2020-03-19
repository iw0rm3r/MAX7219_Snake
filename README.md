# MAX7219_Snake
**Змейка на Arduino Uno/nano с использованием LED матриц и контроллера SNES mini**

![MAX7219_Snake](https://github.com/iw0rm3r/MAX7219_Snake/raw/master/photo.jpg)
[Демонстрационное видео](https://youtu.be/-j2KcqIpfdY)

Классическая игра **"Змейка"**, реализованная на платформе **Arduino Uno/nano** с использованием LED матрицы из модулей на базе контроллера **MAX7219**. Для работы с LED матрицей используются собственные функции, составляющие «обёртку» вокруг легковесной библиотеки **Faster_LEDControl**, т.к. многоцелевые библиотеки управления LED матрицами, вроде MD_Parola, избыточны, и не позволяют уместить нужный функционал во flash-память контроллера Arduino Uno.

**Структура программы:**
1.	**Интро**;
2.	**Собственно игра**. Управление змейкой с помощью d-pad, START - пауза, кнопки L и R - замедление и ускорение игры, соответственно (только в режиме отладки, `#define DEBUG`). По мере набора очков скорость игры увеличивается. Значения минимальной/максимальной задержки обноления экрана указываются в основном файле проекта, как и целевое количество очков;
3.	**Экран проигрыша**, выводящий бегушей строкой число набранных очков;
4.	Аналогичный **экран выигрыша**.

**Используемые компоненты:**
1.	Отладочная плата **Arduino Uno/nano**;
2.	LED матрица из произвольного числа модулей на контроллерах **MAX7219**;
3.	Макетная (печатная) **плата**;
4.	**Пъезодинамик** для озвучивания событий. Лучше подключать через потенциометр для настройки громкости звука;
9.	**Контроллер Nintendo SNES mini**, подключенный по следующей схеме:
![Подключение контроллера SNES mini к Arduino](https://cdn.instructables.com/ORIG/F0N/RJQ3/IH90O28H/F0NRJQ3IH90O28H.jpg)
Контроллер SNES mini может быть легко заменён на любой другой способ ввода. Для этого потребуется внести соответствующие изменения в функцию `inputProcessing()`, удалить директиву подключения библиотеки NintendoExtensionCtrl, объект контроллера и его инициализацию в функции `setup()`.

**Как использовать?**
1.	Скачать исходники проекта с помощью `git clone https://github.com/iw0rm3r/MAX7219_Snake.git`, либо с помощью кнопки *«Clone or download»* > *«Download ZIP»*;
2.	Собрать схему, подключив компоненты к пинам Arduino, указанным в основном .ino-файле проекта, либо указав свои пины;
3.	Скомпилировать и загрузить проект в контроллер с помощью Arduino IDE, либо другой связки ПО.

**Структура проекта:**
1.	*MAX7219_Snake.ino* – основной файл проекта (используемые библиотеки, пины, настройки, глобальные переменные)
2.	*a_MAX7219_stuff.ino* – всё, что относится к выводу на LED матрицу
3.	*b_game_stuff.ino* – все функции игры;
4.	*z_setup_loop.ino* – функции setup() и loop();
5.	*iwTimer.cpp* – класс таймера на основе функции millis() для реализации обновления экрана, фильтрации ввода и т.д.;
6.	*5bite_rus_forum.h* – файл шрифта (источник указан в комментариях).