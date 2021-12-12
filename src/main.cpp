#include <Arduino.h>
#include <CustomStepper.h>                            // Подключаем библиотеку CustomStepper


/* Часы для игры по Алисе стране чудес
 * При старте система переходит в режим установки и начинает вращать стрелку
 * При нажатии клавиши стрелка останавливается и начинается процедура основной логики.
 * Логика работы основного модуля:
 *  - Только минутная стрелка
 *  - Часы идут FORWARD_M минут в правильном направлении
 *  - Часы откатываются на время в начале хода (на час + FORWARD_M минут)
 * Логика работы кнопки
 *  - Если сработало прерывание по кнопке - Возвращаем стрелку на начальное положение (steps*ROTATE_ANGLE)
 */

// Количество оборотов:
//  0.000694444 - часовая стрелка
//  0.016666667 - минутная стрелка
//  1           - секундная стрелка
// Движение при настройке - оборотов в минуту
#define RPM_SETUP     5
// Движение по часовой стрелке - оборотов в минуту
#define RPM_CCW       1
// Движение против часовой стрелки - оборотов в минуту
#define RPM_CW       12
// Градус поворота для движения стрелки на 1 минуту
#define ROTATE_ANGLE  6
// Количество минут прямого хода
#define FORWARD_M     15



CustomStepper stepper(8, 9, 10, 11);                  // Указываем пины, к которым подключен драйвер шагового двигателя
boolean ccw = true;                                   // Флаг направления хода часов
boolean buttonFlag = false;                           // Была ли нажата кнопка
boolean setupFlag = true;                             // Флажок установки
int steps=0;



void buttonTick() {

  buttonFlag = true;
  Serial.println("Button interrupt");
}

void buttonSetup() {

  setupFlag=false;
  Serial.println("Arrow fixed");
}

void setupProcedure()
{
  Serial.println("Begin setup procrdure");
  // D2 это прерывание 0
  // D3 это прерывание 1
  // обработчик - функция buttonTick
  // FALLING - при нажатии на кнопку будет сигнал 0, его и ловим
  attachInterrupt(1, buttonSetup, RISING);   
  Serial.println("Waiting button pressing..");
  while (setupFlag)
  {
    if (stepper.isDone() && setupFlag == true)               
    {
      stepper.setRPM(RPM_SETUP);                         // Устанавливаем количество оборотов в минуту
      stepper.setSPR(4075.7728395);                   // Устанавливаем колочество шагов на полный оборот. Максимальное значение 4075.7728395
      stepper.setDirection(CCW);                       // Устанавливает направление вращения (против часовой)
      stepper.rotateDegrees(ROTATE_ANGLE);  // Устанавливает вращение на заданный градус
    }      
    stepper.run();  
  }
  detachInterrupt(3);  
  Serial.println("Setup procrdure completed");


}

void setup()
{
  Serial.begin(9600);                                 // Инициализаруем консоль
  setupProcedure();                                   // Процедура установки стрелки
  // D2 это прерывание 0
  // D3 это прерывание 1
  // обработчик - функция buttonTick
  // FALLING - при нажатии на кнопку будет сигнал 0, его и ловим
  attachInterrupt(1, buttonTick, FALLING); 
}

void loop()                                           // Для работы двигателя никаких циклов быть не должно, программа сторится на if (stepper.isDone())
{
  if (stepper.isDone() && ccw == true)               // Если прямой ход и обработка закончена  - Делаем поворот на ROTATE_ANGLE градусов
  {
  stepper.setRPM(RPM_CCW);                           // Устанавливаем количество оборотов в минуту - для минутной стрелки - 
  stepper.setSPR(4075.7728395);                      // Устанавливаем колочество шагов на полный оборот. Максимальное значение 4075.7728395
    stepper.setDirection(CCW);                       // Устанавливает направление вращения (по часовой)
    stepper.rotateDegrees(ROTATE_ANGLE);             // Устанавливает вращение на заданный градус
    Serial.print("Rotate CCW begin. Step ");         // Выводим сообщение в консоль 
    steps++;                                         // Количество минут(шагов) +1
    Serial.println(steps);                                  // Выводим сообщение в консоль 
    if (steps >=FORWARD_M)                           // Если прошло FORWARD_M минут
    {
      steps=0;                                       // Сбрасываем счетчик шагов
      ccw = false;                                   // Меняем направление вращения
    }
  }

  if (stepper.isDone() && ccw == false)             // Если обратный ход и обработка закончена  - Делаем поворот на (360+ROTATE_ANGLE*FORWARD_M) градусов против часовой стрелки
  {
    stepper.setRPM(RPM_CW);                         // Устанавливаем количество оборотов в минуту
    stepper.setSPR(4075.7728395);                   // Устанавливаем колочество шагов на полный оборот. Максимальное значение 4075.7728395
    stepper.setDirection(CW);                       // Устанавливает направление вращения (против часовой)
    stepper.rotateDegrees(360+ROTATE_ANGLE*FORWARD_M);  // Устанавливает вращение на заданный градус
    Serial.println("Rotate CW begin.");             // Выводим сообщение в консоль 
    ccw = true;                                     // Меняем направление вращения
  }

  if (buttonFlag == true && ccw == true)            // Если нажата кнопка сброса в момент прямого вращения - Возвращаем стрелку на начальное положение (steps*ROTATE_ANGLE)
  {
    stepper.setRPM(RPM_CW);                         // Устанавливаем количество оборотов в минуту
    stepper.setSPR(4075.7728395);                   // Устанавливаем колочество шагов на полный оборот. Максимальное значение 4075.7728395
    stepper.setDirection(CW);                       // Устанавливает направление вращения (против часовой)
    Serial.print("Rotate reset begin from step ");  // Выводим сообщение в консоль 
    Serial.println(steps);                                  // Выводим сообщение в консоль 
    stepper.rotateDegrees((steps-1)*ROTATE_ANGLE);      // Возвращаем стрелку на начальное положение (steps*ROTATE_ANGLE)
    buttonFlag = false;
    steps=0;                                          
  }

  stepper.run();                      // Этот метод обязателен в блоке loop. Он инициирует работу двигателя, когда это необходимо
} 


