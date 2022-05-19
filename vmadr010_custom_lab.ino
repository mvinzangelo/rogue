#include <Arduino.h>
#include <ArduinoSTL.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <LiquidCrystal.h>

// display variables

const int rs = 8, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd = LiquidCrystal(rs, en, d4, d5, d6, d7);

const int clk = 9, din = 13, d_c = 12, ce = 10, rst = 11;
Adafruit_PCD8544 nokiaScreen = Adafruit_PCD8544(clk, din, d_c, ce, rst);

// screen struct
struct game_screen_struct
{
  int rows = 6;
  int columns = 14;
  char game_screen_buffer[6][14]{
      {' ', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', ' '},
      {'|', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '|'},
      {'|', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '|'},
      {'|', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '|'},
      {'|', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '|'},
      {' ', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', ' '}};
  std::string get_screen_buffer()
  {
    std::string tmp;
    int count = 0;
    for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < columns; j++)
      {
        tmp.push_back(game_screen_buffer[i][j]);
      }
    }
    return tmp;
  }
} game_screen;

// inputs variables

const int xAxis = A0;
const int yAxis = A1;
const int joystickBtn = 2;
const int xAxis_median = 500;
const int yAxis_median = 500;

enum CODE_INPUT
{
  LEFT,
  UP,
  DOWN,
  RIGHT,
  NEUTRAL
} input;

static CODE_INPUT currInput = NEUTRAL;

// input state machine

enum SM_JOYSTICK_INPUT_States
{
  SM_JOYSTICK_INPUT_INIT,
  SM_JOYSTICK_INPUT_NEUTRAL,
  SM_JOYSTICK_INPUT_LEFT,
  SM_JOYSTICK_INPUT_RIGHT,
  SM_JOYSTICK_INPUT_UP,
  SM_JOYSTICK_INPUT_DOWN,
};

int SM_JOYSTICK_INPUT_Tick(int state)
{
  switch (state)
  {
  case SM_JOYSTICK_INPUT_INIT:
    state = SM_JOYSTICK_INPUT_NEUTRAL;
    break;
  case SM_JOYSTICK_INPUT_NEUTRAL:
    if (analogRead(xAxis) < xAxis_median - 400)
    {
      state = SM_JOYSTICK_INPUT_LEFT;
    }
    else if (analogRead(xAxis) > xAxis_median + 400)
    {
      state = SM_JOYSTICK_INPUT_RIGHT;
    }
    else if (analogRead(yAxis) > yAxis_median + 400)
    {
      state = SM_JOYSTICK_INPUT_UP;
    }
    else if (analogRead(yAxis) < yAxis_median - 400)
    {
      state = SM_JOYSTICK_INPUT_DOWN;
    }
    else
    {
      state = SM_JOYSTICK_INPUT_NEUTRAL;
    }
    break;
  case SM_JOYSTICK_INPUT_LEFT:
    if (analogRead(xAxis) > xAxis_median - 400)
    {
      state = SM_JOYSTICK_INPUT_NEUTRAL;
    }
    else
    {
      state = SM_JOYSTICK_INPUT_LEFT;
    }
    break;
  case SM_JOYSTICK_INPUT_RIGHT:
    if (analogRead(xAxis) < xAxis_median + 400)
    {
      state = SM_JOYSTICK_INPUT_NEUTRAL;
    }
    else
    {
      state = SM_JOYSTICK_INPUT_RIGHT;
    }
    break;
  case SM_JOYSTICK_INPUT_UP:
    if (analogRead(yAxis) < yAxis_median + 400)
    {
      state = SM_JOYSTICK_INPUT_NEUTRAL;
    }
    else
    {
      state = SM_JOYSTICK_INPUT_UP;
    }
    break;
  case SM_JOYSTICK_INPUT_DOWN:
    if (analogRead(yAxis) > yAxis_median - 400)
    {
      state = SM_JOYSTICK_INPUT_NEUTRAL;
    }
    else
    {
      state = SM_JOYSTICK_INPUT_DOWN;
    }
    break;
  }
  switch (state)
  {
  case SM_JOYSTICK_INPUT_INIT:
    break;
  case SM_JOYSTICK_INPUT_NEUTRAL:
    currInput = NEUTRAL;
    // Serial.println("NEUTRAL");
    break;
  case SM_JOYSTICK_INPUT_LEFT:
    currInput = LEFT;
    // Serial.println("LEFT");
    break;
  case SM_JOYSTICK_INPUT_RIGHT:
    currInput = RIGHT;
    // Serial.println("RIGHT");
    break;
  case SM_JOYSTICK_INPUT_UP:
    currInput = UP;
    // Serial.println("UP");
    break;
  case SM_JOYSTICK_INPUT_DOWN:
    currInput = DOWN;
    // Serial.println("DOWN");
    break;
  }

  return state;
}

// screen state machine

enum SM_GAME_STATES
{
  SM_GAME_OVERWORLD
};

int SM_GAME_Tick(int state)
{
  switch (state)
  {
  case SM_GAME_OVERWORLD:
    break;
  }
  switch (state)
  {
  case SM_GAME_OVERWORLD:
    break;
  }
}

typedef struct task
{
  int state;
  unsigned long period;
  unsigned long elapsedTime;
  int (*TickFct)(int);
} task;

int delay_gcd;
const unsigned short tasksNum = 1;
task tasks[tasksNum];

void setup()
{

  Serial.begin(9600);

  // randomSeed(analogRead(0));

  unsigned char i = 0;
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
  nokiaScreen.println(game_screen.get_screen_buffer().c_str());
  nokiaScreen.display();

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
