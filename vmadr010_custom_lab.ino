#include <Arduino.h>
#include <ArduinoSTL.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>

#define ROWS 6
#define COLUMNS 14
#define NUMBER_OF_ROOMS 13

// lcd constants
#define rs 8
#define en 7
#define d4 6
#define d5 5
#define d6 4
#define d7 3

// nokia constants
#define clk 9
#define din 13
#define d_c 12
#define ce 10
#define rst 11

LiquidCrystal lcd = LiquidCrystal(rs, en, d4, d5, d6, d7);
char lcd_buffer[18];
Adafruit_PCD8544 nokia_screen = Adafruit_PCD8544(clk, din, d_c, ce, rst);

struct player
{
  short x = 6;
  short y = 3;
  char player_avatar = '@';
  short lvl = 1;
  short hp = 10;
  short str = 1;
  short xp = 0;
  void print_player_info_on_lcd();
} player;
void player::print_player_info_on_lcd()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  sprintf(lcd_buffer, "lvl: %i", lvl);
  lcd.print(lcd_buffer);
  lcd.setCursor(0, 1);
  sprintf(lcd_buffer, " hp: %i", hp);
  lcd.print(lcd_buffer);
  lcd.setCursor(8, 0);
  sprintf(lcd_buffer, " xp: %i", xp);
  lcd.print(lcd_buffer);
  lcd.setCursor(7, 1);
  sprintf(lcd_buffer, " str: %i", str);
  lcd.print(lcd_buffer);
}

#define NUMBER_OF_ENEMIES 21

struct enemy_
{
  char *enemy_name;
  short x;
  short y;
  char enemy_avatar;
  short move_tick_delay;
  short hp;
  short str;
  short xp_on_kill;
  short room_index;
  void move_towards_avatar();
};
short enemy_move_counter = 0;

void enemy_::move_towards_avatar()
{
  if (player.y > y)
  {
    y++;
  }
  else if (player.y < y)
  {
    y--;
  }
  if (player.x > x)
  {
    x++;
  }
  else if (player.x < x)
  {
    x--;
  }
}
const enemy_ enemies[NUMBER_OF_ENEMIES] PROGMEM = {
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
    {"goblin", 1, 1, 'x', __SHRT_MAX__, 5, 1, 1, 0},
};

enemy_ enemy{
    "goblin", 1, 1, 'X', 5, 5, 1, 1, 0};

struct room
{
  char room_layout[ROWS][COLUMNS];
  short left_room_index;
  short right_room_index;
  short top_room_index;
  short bottom_room_index;
};

const room game_map[NUMBER_OF_ROOMS] PROGMEM = {
    // room 0
    {{{'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'}},
     1,
     2,
     3,
     4},
    // room 1
    {{{'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'}},
     0,
     0,
     7,
     5},
    // room 2
    {{{'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'}},
     0,
     0,
     0,
     6},
    // room 3
    {{{'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'}},
     7,
     9,
     0,
     0},
    // room 4
    {{{'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'}},
     0,
     6,
     0,
     0},
    // room 5
    {{{'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'}},
     0,
     0,
     1,
     0},
    // room 6
    {{{'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'}},
     4,
     0,
     2,
     0},
    // room 7
    {{{'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'}},
     8,
     3,
     0,
     1},
    // room 8
    {{{'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'}},
     0,
     7,
     0,
     0},
    // room 9
    {{{'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'}},
     3,
     0,
     10,
     0},
    // room 10
    {{{'+', '-', '-', '-', '-', '+', ' ', ' ', '+', '-', '-', '-', '-', '+'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
      {'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'}},
     0,
     0,
     11,
     9},
    // room 11
    {{{' ', ' ', ' ', ' ', ' ', '+', ' ', ' ', '+', ' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', '|', ' ', ' ', '|', ' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', '|', ' ', ' ', '|', ' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', '|', ' ', ' ', '|', ' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', '|', ' ', ' ', '|', ' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', '|', ' ', ' ', '|', ' ', ' ', ' ', ' ', ' '}},
     0,
     0,
     12,
     10},
    // room 12 (boss room)
    {{{' ', ' ', ' ', ' ', '+', '-', '-', '-', '-', '+', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', '|', ' ', ' ', ' ', ' ', '|', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', '|', ' ', ' ', ' ', ' ', '|', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', '+', '-', ' ', ' ', '-', '+', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', '|', ' ', ' ', '|', ' ', ' ', ' ', ' ', ' '},
      {' ', ' ', ' ', ' ', ' ', '|', ' ', ' ', '|', ' ', ' ', ' ', ' ', ' '}},
     0,
     0,
     0,
     11},
};

int *curr_room_index = new int(0);
char room_buffer[ROWS][COLUMNS];

struct game_screen
{
  char game_screen_buffer[ROWS][COLUMNS];
  std::string get_screen_buffer();
  void copy_room_to_buffer(char curr[ROWS][COLUMNS]);
} game_screen;

std::string game_screen::get_screen_buffer()
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
void game_screen::copy_room_to_buffer(char curr[ROWS][COLUMNS])
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

// inputs variables

#define joystickBtn 2
#define xAxis_median 500
#define yAxis_median 500

const short xAxis = A0;
const short yAxis = A1;

enum JOYSTICK_INPUT
{
  LEFT,
  UP,
  DOWN,
  RIGHT,
  NEUTRAL
} input;

static JOYSTICK_INPUT currInput = NEUTRAL;

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

enum SM_GAME_STATES
{
  SM_GAME_INIT,
  SM_GAME_OVERWORLD,
  SM_GAME_COMBAT
};

short SM_GAME_Tick(short state)
{
  switch (state)
  {
  case SM_GAME_INIT:
    memcpy_P(&room_buffer, &game_map[*curr_room_index], sizeof(room_buffer));
    state = SM_GAME_OVERWORLD;
    break;
  case SM_GAME_OVERWORLD:
    if (enemy.y == player.y && (enemy.x == player.x + 1 || enemy.x == player.x - 1))
    {
      state = SM_GAME_COMBAT;
    }
    else if (enemy.x == player.x && (enemy.y == player.y + 1 || enemy.y == player.y - 1))
    {
      state = SM_GAME_COMBAT;
    }
    break;
  }
  switch (state)
  {
  case SM_GAME_OVERWORLD:
    memcpy_P(&room_buffer, &game_map[*curr_room_index], sizeof(room_buffer));
    switch (currInput)
    {
    case UP:
      if (player.y == 0)
      {
        memcpy_P(curr_room_index, &game_map[*curr_room_index].top_room_index, sizeof(curr_room_index));
        memcpy_P(&room_buffer, &game_map[*curr_room_index], sizeof(room_buffer));
        player.y = ROWS;
      }
      if (game_screen.game_screen_buffer[player.y - 1][player.x] == ' ')
      {
        player.y--;
      }
      break;
    case DOWN:
      if (player.y == ROWS - 1)
      {
        memcpy_P(curr_room_index, &game_map[*curr_room_index].bottom_room_index, sizeof(curr_room_index));
        memcpy_P(&room_buffer, &game_map[*curr_room_index], sizeof(room_buffer));
        player.y = 0;
      }
      else if (game_screen.game_screen_buffer[player.y + 1][player.x] == ' ')
      {
        player.y++;
      }
      break;
    case LEFT:
      if (player.x == 0)
      {
        memcpy_P(curr_room_index, &game_map[*curr_room_index].left_room_index, sizeof(curr_room_index));
        memcpy_P(&room_buffer, &game_map[*curr_room_index], sizeof(room_buffer));
        player.x = COLUMNS - 1;
      }
      else if (game_screen.game_screen_buffer[player.y][player.x - 1] == ' ')
      {
        player.x--;
      }
      break;
    case RIGHT:
      if (player.x == COLUMNS - 1)
      {
        memcpy_P(curr_room_index, &game_map[*curr_room_index].right_room_index, sizeof(curr_room_index));
        memcpy_P(&room_buffer, &game_map[*curr_room_index], sizeof(room_buffer));
        player.x = 0;
      }
      else if (game_screen.game_screen_buffer[player.y][player.x + 1] == ' ')
      {
        player.x++;
      }
      break;
    default:
      Serial.println(currInput);
      break;
    }
    enemy_move_counter++;
    if (enemy.move_tick_delay < enemy_move_counter)
    {
      enemy.move_towards_avatar();
      enemy_move_counter = 0;
    }
    game_screen.copy_room_to_buffer(room_buffer);
    game_screen.game_screen_buffer[enemy.y][enemy.x] = enemy.enemy_avatar;
    game_screen.game_screen_buffer[player.y][player.x] = player.player_avatar;
    nokia_screen.clearDisplay();
    nokia_screen.setCursor(0, 0);
    nokia_screen.println(game_screen.get_screen_buffer().c_str());
    nokia_screen.display();
    player.print_player_info_on_lcd();
    break;
  case SM_GAME_COMBAT:
    lcd.clear();
    lcd.print("IN COMBAT");
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

  nokia_screen.setRotation(2);
  nokia_screen.begin();
  nokia_screen.setContrast(50);
  nokia_screen.clearDisplay();
  nokia_screen.setTextSize(1);
  nokia_screen.setTextColor(BLACK);
}

void loop()
{
  unsigned char i;
  for (i = 0; i < tasksNum; ++i)
  {
    if ((millis() - tasks[i].elapsedTime) >= tasks[i].period)
    {
      tasks[i].state = tasks[i].TickFct(tasks[i].state);
      tasks[i].elapsedTime = millis();
    }
  }
}
