#include <Arduino.h>
#include <ArduinoSTL.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <LiquidCrystal.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

#define ROWS 6
#define COLUMNS 14
#define NUMBER_OF_ROOMS 13

// player scaling constants
#define STR_INC_ON_LVL_UP 2
#define HP_INC_ON_LVL_UP 5
#define XP_THRESHOLD 10

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

uint8_t current_room_index = 0;
char room_buffer[ROWS][COLUMNS];
uint8_t num_of_enemies_buffer = 0;
uint8_t current_enemy_index = 0;
bool is_joystick_down = false;
bool cleared_rooms[NUMBER_OF_ROOMS] = {false};

#define COMBAT_DELAY 5
uint8_t combat_counter = 0;

LiquidCrystal lcd = LiquidCrystal(rs, en, d4, d5, d6, d7);
char lcd_buffer[18];
Adafruit_PCD8544 nokia_screen = Adafruit_PCD8544(clk, din, d_c, ce, rst);

struct player
{
  uint8_t x = 6;
  uint8_t y = 3;
  char player_avatar = '@';
  uint8_t lvl;
  int8_t max_hp;
  int8_t hp;
  uint8_t str;
  uint8_t xp;
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

// enemy constants
#define GOBLIN_HP 2
#define GOBLIN_STR 1
#define GOBLIN_XP 2
#define GOBLIN_TICK_DELAY 5

#define SKELETON_HP 10
#define SKELETON_STR 2
#define SKELETON_XP 5
#define SKELETON_TICK_DELAY 8

#define KNIGHT_HP 15
#define KNIGHT_STR 5
#define KNIGHT_XP 10
#define KNIGHT_TICK_DELAY 15

#define DRAGON_HP 30
#define DRAGON_STR 10
#define DRAGON_XP 30
#define DRAGON_TICK_DELAY 20

#define KING_HP 100
#define KING_STR 20
#define KING_XP 100
#define KING_TICK_DELAY 255

struct enemy
{
  char *enemy_name;
  uint8_t x;
  uint8_t y;
  char enemy_avatar;
  uint8_t move_tick_delay;
  int8_t hp;
  uint8_t str;
  uint8_t xp_on_kill;
  uint8_t move_tick_delay_counter;
  void move_towards_avatar();
};

enemy enemies_in_room[3];

short enemy_move_counter = 0;

void enemy::move_towards_avatar()
{
  if (hp > 0 && !cleared_rooms[current_room_index])
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
}

void move_enemies_toward_player()
{
  if (num_of_enemies_buffer > 0)
  {
    for (uint8_t i = 0; i < num_of_enemies_buffer; i++)
    {
      if (enemies_in_room[i].move_tick_delay < enemies_in_room[i].move_tick_delay_counter)
      {
        enemies_in_room[i].move_towards_avatar();
        enemies_in_room[i].move_tick_delay_counter = 0;
      }
      else
      {
        enemies_in_room[i].move_tick_delay_counter++;
      }
    }
  }
}

struct room
{
  char room_layout[ROWS][COLUMNS];
  short left_room_index;
  short right_room_index;
  short top_room_index;
  short bottom_room_index;
  short number_of_enemies;
  enemy room_enemies[3];
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
     4,
     0},
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
     5,
     2,
     {{"goblin", 1, 1, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}, {"goblin", 5, 4, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}}},
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
     6,
     2,
     {{"goblin", 12, 2, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}, {"goblin", 11, 3, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}}},
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
     0,
     3,
     {{"goblin", 1, 1, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}, {"goblin", 6, 1, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}, {"goblin", 11, 2, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}}},
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
     0,
     3,
     {{"goblin", 2, 4, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}, {"goblin", 6, 3, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}, {"goblin", 10, 4, 'G', GOBLIN_TICK_DELAY, GOBLIN_HP, GOBLIN_STR, GOBLIN_XP}}},
    // room 5
    {
        {{'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'},
         {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
         {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
         {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
         {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
         {'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'}},
        0,
        0,
        1,
        0,
        2,
        {{"skeleton", 12, 4, 'S', SKELETON_TICK_DELAY, SKELETON_HP, SKELETON_STR, SKELETON_XP}, {"skeleton", 1, 4, 'S', SKELETON_TICK_DELAY, SKELETON_HP, SKELETON_STR, SKELETON_XP}}},
    // room 6
    {
        {{'+', '-', '-', '-', '-', '-', ' ', ' ', '-', '-', '-', '-', '-', '+'},
         {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
         {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
         {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
         {'|', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '|'},
         {'+', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '-', '+'}},
        4,
        0,
        2,
        0,
        3,
        {{"skeleton", 2, 4, 'S', SKELETON_TICK_DELAY, SKELETON_HP, SKELETON_STR, SKELETON_XP}, {"skeleton", 6, 4, 'S', SKELETON_TICK_DELAY, SKELETON_HP, SKELETON_STR, SKELETON_HP}, {"skeleton", 8, 4, 'S', SKELETON_TICK_DELAY, SKELETON_HP, SKELETON_HP, SKELETON_XP}}},
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
     1,
     1,
     {{"skeleton", 5, 2, 'S', SKELETON_TICK_DELAY, SKELETON_HP, SKELETON_STR, SKELETON_XP}}},
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
     0,
     2,
     {{"knight", 1, 1, 'K', KNIGHT_TICK_DELAY, KNIGHT_HP, KNIGHT_STR, KNIGHT_XP}, {"knight", 1, 4, 'K', KNIGHT_TICK_DELAY, KNIGHT_HP, KNIGHT_STR, KNIGHT_XP}}},
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
     0,
     2,
     {{"knight", 12, 1, 'K', KNIGHT_TICK_DELAY, KNIGHT_HP, KNIGHT_STR, KNIGHT_XP}, {"knight", 11, 4, 'K', KNIGHT_TICK_DELAY, KNIGHT_HP, KNIGHT_STR, KNIGHT_XP}}},
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
     9,
     1,
     {{"dragon", 6, 1, 'D', DRAGON_TICK_DELAY, DRAGON_HP, DRAGON_STR, DRAGON_XP}}},
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
     10,
     0},
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
     11,
     1,
     {{"king", 6, 1, '#', KING_TICK_DELAY, KING_HP, KING_STR, KING_XP}}},
};

void copy_enemies_to_buffer()
{
  num_of_enemies_buffer = pgm_read_word_near(&game_map[current_room_index].number_of_enemies);
  if (num_of_enemies_buffer > 0)
  {
    for (uint8_t i = 0; i < num_of_enemies_buffer; i++)
    {
      enemies_in_room[i].x = pgm_read_word_near(&game_map[current_room_index].room_enemies[i].x);
      enemies_in_room[i].y = pgm_read_word_near(&game_map[current_room_index].room_enemies[i].y);
      enemies_in_room[i].enemy_name = pgm_read_word_near(&game_map[current_room_index].room_enemies[i].enemy_name);
      enemies_in_room[i].enemy_avatar = pgm_read_word_near(&game_map[current_room_index].room_enemies[i].enemy_avatar);
      enemies_in_room[i].move_tick_delay = pgm_read_word_near(&game_map[current_room_index].room_enemies[i].move_tick_delay);
      enemies_in_room[i].hp = pgm_read_word_near(&game_map[current_room_index].room_enemies[i].hp);
      enemies_in_room[i].str = pgm_read_word_near(&game_map[current_room_index].room_enemies[i].str);
      enemies_in_room[i].xp_on_kill = pgm_read_word_near(&game_map[current_room_index].room_enemies[i].xp_on_kill);
      enemies_in_room[i].move_tick_delay_counter = 0;
    }
  }
}

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

void put_enemies_on_screen()
{
  if (num_of_enemies_buffer > 0)
  {
    for (uint8_t i = 0; i < num_of_enemies_buffer; i++)
    {
      if (enemies_in_room[i].hp > 0 && !cleared_rooms[current_room_index])
      {
        game_screen.game_screen_buffer[enemies_in_room[i].y][enemies_in_room[i].x] = enemies_in_room[i].enemy_avatar;
      }
    }
  }
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
    break;
  case SM_JOYSTICK_INPUT_LEFT:
    currInput = LEFT;
    break;
  case SM_JOYSTICK_INPUT_RIGHT:
    currInput = RIGHT;
    break;
  case SM_JOYSTICK_INPUT_UP:
    currInput = UP;
    break;
  case SM_JOYSTICK_INPUT_DOWN:
    currInput = DOWN;
    break;
  }

  return state;
}

enum SM_GAME_STATES
{
  SM_GAME_INIT,
  SM_GAME_MENU,
  SM_GAME_OVERWORLD,
  SM_GAME_COMBAT,
  SM_GAME_COMBAT_WIN,
  SM_GAME_VICTORY,
  SM_GAME_DEATH
};

void player_combat_turn()
{
  lcd.setCursor(0, 0);
  int player_dice_roll = random(6);
  if (player_dice_roll == 5)
  {
    lcd.print(F("you hit"));
    enemies_in_room[current_enemy_index].hp -= player.str;
  }
  else
  {
    lcd.print(F("you miss"));
  }
}

void enemy_combat_turn()
{
  lcd.setCursor(0, 1);
  int enemy_dice_roll = random(6);
  if (enemy_dice_roll == 5)
  {
    sprintf(lcd_buffer, "the %s hits", enemies_in_room[current_enemy_index].enemy_name);
    lcd.print(lcd_buffer);
    player.hp -= enemies_in_room[current_enemy_index].str;
  }
  else
  {
    sprintf(lcd_buffer, "the %s miss", enemies_in_room[current_enemy_index].enemy_name);
    lcd.print(lcd_buffer);
  }
}

void check_if_room_clear()
{
  uint8_t cnt = 0;
  for (uint8_t i = 0; i < num_of_enemies_buffer; i++)
  {
    if (enemies_in_room[i].hp <= 0)
    {
      cnt++;
    }
  }
  if (cnt == num_of_enemies_buffer)
  {
    cleared_rooms[current_room_index] = true;
  }
}

void read_player_stats_from_eeprom()
{
  player.lvl = EEPROM.read(0);
  player.max_hp = EEPROM.read(1);
  player.str = EEPROM.read(2);
  player.xp = EEPROM.read(3);
  player.hp = player.max_hp;
}

void save_player_stats_to_eeprom()
{
  EEPROM.update(0, player.lvl);
  EEPROM.update(1, player.max_hp);
  EEPROM.update(2, player.str);
  EEPROM.update(3, player.xp);
}

void reset_player_stats_on_eeprom()
{
  EEPROM.write(0, 1);
  EEPROM.write(1, 10);
  EEPROM.write(2, 1);
  EEPROM.write(3, 0);
  player.hp = player.max_hp;
}

bool is_start_selected = true;

short SM_GAME_Tick(short state)
{
  switch (state)
  {
  case SM_GAME_INIT:
    state = SM_GAME_MENU;
  case SM_GAME_MENU:
    Serial.println(player.lvl);
    if (is_start_selected && !digitalRead(joystickBtn))
    {
      current_room_index = 0;
      player.x = 6;
      player.y = 3;
      for (uint8_t i = 0; i < NUMBER_OF_ROOMS; i++)
      {
        cleared_rooms[i] = false;
      }
      memcpy_P(&room_buffer, &game_map[current_room_index], sizeof(room_buffer));
      num_of_enemies_buffer = 0;
      state = SM_GAME_OVERWORLD;
    }
    else if (!is_start_selected && !digitalRead(joystickBtn))
    {
      reset_player_stats_on_eeprom();
      read_player_stats_from_eeprom();
    }
    else
    {
      state = SM_GAME_MENU;
    }
    break;
  case SM_GAME_OVERWORLD:
    if (num_of_enemies_buffer > 0)
    {
      for (uint8_t i = 0; i < num_of_enemies_buffer; i++)
      {
        if (enemies_in_room[i].hp > 0 && !cleared_rooms[current_room_index])
        {
          if (enemies_in_room[i].y == player.y && (enemies_in_room[i].x == player.x + 1 || enemies_in_room[i].x == player.x - 1))
          {
            state = SM_GAME_COMBAT;
            current_enemy_index = i;
            lcd.clear();
            player_combat_turn();
            enemy_combat_turn();
          }
          else if (enemies_in_room[i].x == player.x && (enemies_in_room[i].y == player.y + 1 || enemies_in_room[i].y == player.y - 1))
          {
            state = SM_GAME_COMBAT;
            current_enemy_index = i;
            lcd.clear();
            player_combat_turn();
            enemy_combat_turn();
          }
        }
      }
    }
    break;
  case SM_GAME_COMBAT:
    if (player.hp <= 0)
    {
      save_player_stats_to_eeprom();
      state = SM_GAME_DEATH;
    }
    else if (enemies_in_room[current_enemy_index].hp <= 0)
    {
      state = SM_GAME_COMBAT_WIN;
    }
    break;
  case SM_GAME_COMBAT_WIN:
    if (!digitalRead(joystickBtn))
    {
      player.xp += enemies_in_room[current_enemy_index].xp_on_kill;
      if (player.xp >= XP_THRESHOLD)
      {
        player.lvl++;
        player.str += STR_INC_ON_LVL_UP;
        player.max_hp += HP_INC_ON_LVL_UP;
        player.hp = player.max_hp;
        if (player.xp > XP_THRESHOLD)
        {
          uint8_t overflow = player.xp % XP_THRESHOLD;
          player.xp = overflow;
        }
        else
        {
          player.xp = 0;
        }
      }
      check_if_room_clear();
      if (cleared_rooms[NUMBER_OF_ROOMS - 1])
      {
        save_player_stats_to_eeprom();
        state = SM_GAME_VICTORY;
      }
      else
      {
        state = SM_GAME_OVERWORLD;
      }
    }
    break;
  case SM_GAME_DEATH:
    if (!digitalRead(joystickBtn))
    {
      read_player_stats_from_eeprom();
      state = SM_GAME_MENU;
    }
    break;
  case SM_GAME_VICTORY:
    if (!digitalRead(joystickBtn))
    {
      read_player_stats_from_eeprom();
      state = SM_GAME_MENU;
    }
  }

  switch (state)
  {
  case SM_GAME_MENU:
    switch (currInput)
    {
    case UP:
      is_start_selected = !is_start_selected;
      break;
    case DOWN:
      is_start_selected = !is_start_selected;
      break;
    default:
      break;
    }
    player.print_player_info_on_lcd();
    nokia_screen.clearDisplay();
    nokia_screen.setCursor(30, 5);
    nokia_screen.println(F("ROGUE"));
    if (is_start_selected)
    {
      nokia_screen.setCursor(0, 23);
      nokia_screen.println(F("> start"));
      nokia_screen.setCursor(0, 35);
      nokia_screen.println(F("reset"));
    }
    else
    {
      nokia_screen.setCursor(0, 23);
      nokia_screen.println(F("start"));
      nokia_screen.setCursor(0, 35);
      nokia_screen.println(F("> reset"));
    }
    nokia_screen.display();
    break;
  case SM_GAME_OVERWORLD:
    memcpy_P(&room_buffer, &game_map[current_room_index], sizeof(room_buffer));
    switch (currInput)
    {
    case UP:
      if (player.y == 0)
      {
        current_room_index = pgm_read_word_near(&game_map[current_room_index].top_room_index);
        memcpy_P(&room_buffer, &game_map[current_room_index], sizeof(room_buffer));
        copy_enemies_to_buffer();
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
        current_room_index = pgm_read_word_near(&game_map[current_room_index].bottom_room_index);
        memcpy_P(&room_buffer, &game_map[current_room_index], sizeof(room_buffer));
        copy_enemies_to_buffer();
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
        current_room_index = pgm_read_word_near(&game_map[current_room_index].left_room_index);
        memcpy_P(&room_buffer, &game_map[current_room_index], sizeof(room_buffer));
        copy_enemies_to_buffer();
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
        current_room_index = pgm_read_word_near(&game_map[current_room_index].right_room_index);
        memcpy_P(&room_buffer, &game_map[current_room_index], sizeof(room_buffer));
        copy_enemies_to_buffer();
        player.x = 0;
      }
      else if (game_screen.game_screen_buffer[player.y][player.x + 1] == ' ')
      {
        player.x++;
      }
      break;
    default:
      break;
    }
    move_enemies_toward_player();
    game_screen.copy_room_to_buffer(room_buffer);
    put_enemies_on_screen();
    game_screen.game_screen_buffer[player.y][player.x] = player.player_avatar;
    nokia_screen.clearDisplay();
    nokia_screen.setCursor(0, 0);
    nokia_screen.println(game_screen.get_screen_buffer().c_str());
    nokia_screen.display();
    player.print_player_info_on_lcd();
    break;
  case SM_GAME_COMBAT:

    if (combat_counter >= COMBAT_DELAY)
    {
      lcd.clear();
      player_combat_turn();
      enemy_combat_turn();
      combat_counter = 0;
    }
    else if (combat_counter == COMBAT_DELAY - 1)
    {
      lcd.clear();
    }
    combat_counter++;
    break;
  case SM_GAME_COMBAT_WIN:
    lcd.clear();
    lcd.print(F("you defeated the"));
    lcd.setCursor(0, 1);
    lcd.print(enemies_in_room[current_enemy_index].enemy_name);
    break;
  case SM_GAME_DEATH:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("you died."));
    lcd.setCursor(0, 1);
    lcd.print(F("press to restart"));
    break;
  case SM_GAME_VICTORY:
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("you won!"));
    lcd.setCursor(0, 1);
    lcd.print(F("press to restart"));
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

  randomSeed(analogRead(0));

  unsigned char i = 0;
  tasks[i].state = SM_JOYSTICK_INPUT_INIT;
  tasks[i].period = 100;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM_JOYSTICK_INPUT_Tick;
  i++;
  tasks[i].state = SM_GAME_MENU;
  tasks[i].period = 200;
  tasks[i].elapsedTime = 0;
  tasks[i].TickFct = &SM_GAME_Tick;

  delay_gcd = 100; // GCD
  lcd.begin(16, 2);
  pinMode(joystickBtn, INPUT_PULLUP);

  nokia_screen.setRotation(2);
  nokia_screen.begin();
  nokia_screen.setContrast(70);
  nokia_screen.clearDisplay();
  nokia_screen.setTextSize(1);
  nokia_screen.setTextColor(BLACK);

  // reset_player_stats_on_eeprom();
  read_player_stats_from_eeprom();
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
