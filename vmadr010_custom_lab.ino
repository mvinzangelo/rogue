#include <Arduino.h>
#include <ArduinoSTL.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <LiquidCrystal.h>

#define ROWS 6
#define COLUMNS 14

// display variables
const short rs = 8, en = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd = LiquidCrystal(rs, en, d4, d5, d6, d7);

const short clk = 9, din = 13, d_c = 12, ce = 10, rst = 11;
Adafruit_PCD8544 nokiaScreen = Adafruit_PCD8544(clk, din, d_c, ce, rst);

// struct player
struct player
{
  short x = 4;
  short y = 3;
  char player_avatar = '@';
} player;

// room class
enum DOOR_LOCATION
{
  DOOR_TOP,
  DOOR_BOTTOM,
  DOOR_LEFT,
  DOOR_RIGHT
};
struct room
{
  char room_layout[ROWS][COLUMNS];
  room *adjacent_rooms[4];
};

// rooms
struct room room_1
{
  {
    {' ', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', ' '},
        {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
        {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
        {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
        {' ', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', ' '},
  }
};
struct room room_2
{
  {
    {' ', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', ' '},
        {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
        {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
        {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
        {' ', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', ' '},
  }
};

void set_adjacent_rooms()
{
  room_1.adjacent_rooms[DOOR_LEFT] = &room_2;
  room_2.adjacent_rooms[DOOR_RIGHT] = &room_1;
}

room *curr_room = &room_1;

// game screen global struct
struct game_screen
{
  char game_screen_buffer[ROWS][COLUMNS];
  // methods
  std::string get_screen_buffer()
  {
    std::string tmp;
    short count = 0;
    for (short i = 0; i < ROWS; i++)
    {
      for (short j = 0; j < COLUMNS; j++)
      {
        tmp.push_back(game_screen_buffer[i][j]);
      }
    }
    return tmp;
  }
  void copy_room_shorto_buffer(char curr[ROWS][COLUMNS])
  {
    for (short i = 0; i < ROWS; i++)
    {
      for (short j = 0; j < COLUMNS; j++)
      {
        game_screen_buffer[i][j] = curr[i][j];
      }
    }
    return;
  }
} game_screen;

// inputs variables

const short xAxis = A0;
const short yAxis = A1;
const short joystickBtn = 2;
const short xAxis_median = 500;
const short yAxis_median = 500;

enum JOYSTICK_INPUT
{
  LEFT,
  UP,
  DOWN,
  RIGHT,
  NEUTRAL
} input;

static JOYSTICK_INPUT currInput = NEUTRAL;

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

short SM_JOYSTICK_INPUT_Tick(short state)
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

short SM_GAME_Tick(short state)
{
  switch (state)
  {
  case SM_GAME_OVERWORLD:
    break;
  }
  switch (state)
  {
  case SM_GAME_OVERWORLD:
    switch (currInput)
    {
    case UP:
      if (curr_room->room_layout[player.y - 1][player.x] != '-')
      {
        player.y--;
      }
      break;
    case DOWN:
      if (curr_room->room_layout[player.y + 1][player.x] != '-')
      {
        player.y++;
      }
      break;
    case LEFT:
      if (curr_room->room_layout[player.y][player.x - 1] != '|')
      {
        player.x--;
      }
      break;
    case RIGHT:
      if (curr_room->room_layout[player.y][player.x + 1] != '|')
      {
        player.x++;
      }
      break;
    default:
      Serial.println(currInput);
      break;
    }
    game_screen.copy_room_shorto_buffer(room_1.room_layout);
    game_screen.game_screen_buffer[player.y][player.x] = player.player_avatar;
    nokiaScreen.clearDisplay();
    nokiaScreen.setCursor(0, 0);
    nokiaScreen.println(game_screen.get_screen_buffer().c_str());
    nokiaScreen.display();
    break;
  }
  return state;
}

typedef struct task
{
  short state;
  unsigned long period;
  unsigned long elapsedTime;
  short (*TickFct)(short);
} task;

short delay_gcd;
const unsigned short tasksNum = 2;
task tasks[tasksNum];

void setup()
{
  Serial.begin(9600);

  set_adjacent_rooms();

  // randomSeed(analogRead(0));

  unsigned char i = 0;
  tasks[i].state = SM_JOYSTICK_INPUT_INIT;
  tasks[i].period = 100;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM_JOYSTICK_INPUT_Tick;
  i++;
  tasks[i].state = SM_GAME_OVERWORLD;
  tasks[i].period = 200;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM_GAME_Tick;

  delay_gcd = 100; // GCD
  lcd.begin(16, 2);
  pinMode(joystickBtn, INPUT_PULLUP);

  nokiaScreen.setRotation(2);
  nokiaScreen.begin();
  nokiaScreen.setContrast(50);
  nokiaScreen.clearDisplay();
  nokiaScreen.setTextSize(1);
  nokiaScreen.setTextColor(BLACK);

  lcd.clear();
  lcd.println("Hello, world!");
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
