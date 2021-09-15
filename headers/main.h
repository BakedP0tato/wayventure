#pragma once
#include <libtcod.h>
#include <SDL2/SDL.h>
const auto modkeys = KMOD_CTRL | KMOD_SHIFT | KMOD_ALT;
const std::string TILESET_FILE = "res/alphabet.png";
const tcod::TilesetPtr tileset=tcod::load_tilesheet(TILESET_FILE, {16,16}, tcod::CHARMAP_CP437);

const std::string username = "Wayfarer"; // Please don't put weird characters and keep the length under 30 characters. The program will grumble if you don't follow my instructions.

// Tweak these colors if your eyes go ouch. You can play around with the colors and relaunch the program when you're ready.
static constexpr TCOD_ColorRGB BLACK{10,10,10}; // def_background
static constexpr TCOD_ColorRGB WHITE{208,208,208}; // def_foreground
static constexpr TCOD_ColorRGB CYAN{0,255,255}; // player_char (exclusive)
static constexpr TCOD_ColorRGB LIGHT_RED{255,50,50}; // enemy (exclusive)
static constexpr TCOD_ColorRGB GREEN{50,153,50};
static constexpr TCOD_ColorRGB BLUE{0,79,152};
static constexpr TCOD_ColorRGB PURPLE{165,0,165};
static constexpr TCOD_ColorRGB YELLOW{255,255,0};
static constexpr TCOD_ColorRGB DARK_RED{225,2,2}; // artifact (exclusive)
static constexpr TCOD_ColorRGB LIGHT_BLUE{173,216,230}; // water (exclusive)
static constexpr TCOD_ColorRGB MAGENTA{255,0,255}; // NPC (exclusive)

// Don't touch any of these. For god's sake.
const std::string empty_line = "                                                                                ";
constexpr char RARITY_COMMON = 'c';
constexpr char RARITY_UNCOMMON = 'u';
constexpr char RARITY_RARE = 'r';
constexpr char RARITY_EPIC = 'e';
constexpr char RARITY_LEGENDARY = 'l';
constexpr char RARITY_ARTIFACT = 'a';
constexpr char TYPE_HELMET = 'h';
constexpr char TYPE_CHESTPLATE = 'c';
constexpr char TYPE_GREAVES = 'g';
constexpr char TYPE_BOOTS = 'b';
constexpr char TYPE_SHIELD = 's';
constexpr char TYPE_WEAPON = 'w';
constexpr char DUNGEON_ENEMY = 'e';
constexpr char DUNGEON_ROOM_BOSS = 'b';
constexpr char DUNGEON_LEVEL_BOSS = 'x';
constexpr char DUNGEON_FINAL_BOSS = 'f';

void clear_and_draw_dialog(tcod::ConsolePtr &main_win, tcod::ContextPtr &context, std::string dialog);
int SDL_getch(tcod::ConsolePtr &main_win, tcod::ContextPtr &context);
void SDL_wclear_main_win(tcod::ConsolePtr &main_win, tcod::ContextPtr &context);
void SDL_wclear_dialog_bar(tcod::ConsolePtr &main_win, tcod::ContextPtr &context);
void SDL_wclear_stats_bar(tcod::ConsolePtr &main_win, tcod::ContextPtr &context);
