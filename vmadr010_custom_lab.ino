#include <Arduino.h>

// #include <joystick_input.ino>
// #include <nokia_screen_driver.ino>
// #include <lcd_screen_driver.ino>

typedef struct task
{
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;

int delay_gcd;
const unsigned short tasksNum = 2;
task tasks[tasksNum];

void setup()
{

  Serial.begin(9600);

  // randomSeed(analogRead(0));

  unsigned char i = 0;
  tasks[i].state = SM_LCD_DRIVER_INIT;
  tasks[i].period = 200;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM_LCD_DRIVER_Tick;
  i++;
  tasks[i].state = SM_JOYSTICK_INPUT_INIT;
  tasks[i].period = 100;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM_JOYSTICK_INPUT_Tick;

  delay_gcd = 100; // GCD
  lcd.begin(16, 2);
  pinMode(joystickBtn, INPUT_PULLUP);
  nokiaScreen.setRotation(2);
  nokiaScreen.begin();
  nokiaScreen.clearDisplay();
  nokiaScreen.setContrast(50);
  nokiaScreen.setTextSize(1);
  nokiaScreen.setTextColor(BLACK);
  nokiaScreen.setCursor(0, 0);
  nokiaScreen.println("Hello world!");
  nokiaScreen.display();
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Hello, world!");
}

void loop()
{
  // put your main code here, to run repeatedly:
  unsigned char i;
  for (i = 0; i < tasksNum; ++i)
  {
    if ((millis() - tasks[i].elapsedTime) >= tasks[i].period)
    {
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = millis(); // Last time this task was ran
    }
  }
}
