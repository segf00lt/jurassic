#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#include "basic.h"
#include "arena.h"
#include "str.h"
#include "context.h"
#include "array.h"
#include "sprite.h"
#include "stb_sprintf.h"


/*
 * macro constants
 */

#define GAME_TITLE "Jurassic"
#define TARGET_FPS 60
#define TARGET_FRAME_TIME ((float)(1.0f / (float)TARGET_FPS))

#define WINDOW_WIDTH  ((float)GetScreenWidth())
#define WINDOW_HEIGHT ((float)GetScreenHeight())

#define WINDOW_SIZE ((Vector2){ WINDOW_WIDTH, WINDOW_HEIGHT })
#define WINDOW_RECT ((Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT})

#define MAX_ENTITIES 4096
#define MAX_PARTICLES 8192
#define MAX_BULLETS_IN_BAG 8
#define MAX_PARENTS 32
#define MAX_ENTITY_LISTS 16

#define TILE_SIZE 32
#define INV_TILE_SIZE ((float)(1.0f/(float)TILE_SIZE))
#define TILE_ROWS 256
#define TILE_COLS 256
#define TILES_COUNT (TILE_ROWS*TILE_COLS)

#define BLOOD ((Color){ 255, 0, 0, 255 })

#define INITIAL_CAMERA_ZOOM ((float)3.0f)
#define DEFAULT_CAMERA_TARGET ((Vector2){ 400, 400 })


/*
 * tables
 */

#define GAME_STATES             \
  X(NONE)                       \
  X(TITLE_SCREEN)               \
  X(INTRO_SCREEN)               \
  X(GAME_OVER)                  \
  X(START_LEVEL)                \
  X(MAIN_LOOP)                  \
  X(VICTORY)                    \
  X(DEBUG_SANDBOX)              \
  X(EDITOR)                     \

#define GAME_DEBUG_FLAGS       \
  X(DEBUG_UI)                  \
  X(PLAYER_INVINCIBLE)         \
  X(DRAW_ALL_ENTITY_BOUNDS)    \
  X(SANDBOX_LOADED)            \
  X(SKIP_TRANSITIONS)          \
  X(MUTE)                      \
  X(FREE_CAM)                  \

#define GAME_FLAGS         \
  X(PAUSE)                 \
  X(CAN_PAUSE)             \
  X(PLAYER_CANNOT_SHOOT)   \
  X(HUD)                   \
  X(DRAW_IN_CAMERA)        \
  X(CAMERA_SHAKE)          \
  X(CAMERA_PULSATE)        \

#define INPUT_FLAGS         \
  X(ANY)                    \
  X(MOVE_FORWARD)           \
  X(MOVE_BACKWARD)          \
  X(MOVE_LEFT)              \
  X(MOVE_RIGHT)             \
  X(SHOOT)                  \
  X(SHOOT_HOLD)             \
  X(INTERACT)               \
  X(THROW)                  \
  X(PAUSE)                  \

#define ENTITY_KINDS           \
  X(PLAYER)                    \
  X(PARENT)                    \
  X(PLAYER_BULLET)             \
  X(RAPTOR)                    \
  X(HEALTH_PACK)               \
  X(SHOTGUN)                   \
  X(ASSAULT_RIFLE)             \
  X(GRENADE_LAUNCHER)          \
  X(FLAMETHROWER)              \
  X(BOSS)                      \

#define ENTITY_ORDERS   \
  X(FIRST)              \
  X(LAST)               \

#define ENTITY_FLAGS                 \
  X(DYNAMICS)                        \
  X(SPINNING)                        \
  X(HAS_SPRITE)                      \
  X(MANUAL_SPRITE_ORIGIN)            \
  X(APPLY_FRICTION)                  \
  X(FILL_BOUNDS)                     \
  X(HAS_GUN)                         \
  X(NOT_ON_SCREEN)                   \
  X(ON_SCREEN)                       \
  X(DIE_IF_CHILD_LIST_EMPTY)         \
  X(DIE_NOW)                         \
  X(INTERACT)                        \
  X(IS_INTERACTABLE)                 \
  X(HAS_LIFETIME)                    \
  X(APPLY_COLLISION)                 \
  X(RECEIVE_COLLISION)               \
  X(APPLY_COLLISION_DAMAGE)          \
  X(RECEIVE_COLLISION_DAMAGE)        \
  X(DAMAGE_INCREMENTS_SCORE)         \
  X(DIE_ON_APPLY_COLLISION)          \
  X(HAS_PARTICLE_EMITTER)            \
  X(CHILDREN_ON_SCREEN)              \
  X(EMIT_SPAWN_PARTICLES)            \
  X(EMIT_DEATH_PARTICLES)            \
  X(APPLY_EFFECT_TINT)               \

#define ENTITY_CONTROLS               \
  X(PLAYER)                           \
  X(FOLLOW_PARENT)                    \
  X(GUN_BEING_HELD)                   \
  X(FOLLOW_PARENT_CHAIN)              \
  X(COPY_PARENT)                      \
  X(GOTO_WAYPOINT)                    \

#define PARTICLE_EMITTERS        \
  X(SPARKS)                      \
  X(PURPLE_SPARKS)               \
  X(BLOOD_SPIT)                  \
  X(BLOOD_PUFF)                  \
  X(MASSIVE_BLOOD_PUFF)          \
  X(PINK_PUFF)                   \
  X(GREEN_PUFF)                  \
  X(BROWN_PUFF)                  \
  X(WEAPON_DIE_PUFF)             \
  X(WHITE_PUFF)                  \
  X(BIG_PLANE_EXPLOSION)         \

#define PARTICLE_FLAGS            \

#define GUN_KINDS                   \
  X(SHOTGUN)                        \
  X(ASSAULT_RIFLE)                  \
  X(GRENADE_LAUNCHER)               \
  X(FLAMETHROWER)                   \

#define GUN_FLAGS                   \
  X(MANUALLY_SET_DIR)               \
  X(LOOK_AT_PLAYER)                 \
  X(USE_POINT_BAG)                  \
  X(BURST)                          \
  X(AUTOMATIC)                      \
  X(SPIN_WITH_SINE)                 \


/*
 * macros
 */

#define entity_kind_in_mask(kind, mask) (!!((mask) & (1ull<<(kind))))


/*
 * typedefs
 */

typedef struct Game Game;
typedef struct Map_data Map_data;
typedef struct Editor Editor;
typedef struct Entity Entity;
typedef Entity* Entity_ptr;
typedef struct Particle Particle;
typedef struct Gun Gun;
typedef u64 Game_flags;
typedef u64 Game_debug_flags;
typedef u64 Particle_emitter_kind_mask;
typedef u64 Gun_kind_mask;
typedef u64 Gun_flags;
typedef u64 Input_flags;
typedef u64 Entity_flags;
typedef u64 Entity_kind_mask;
typedef u32 Particle_flags;
typedef struct Entity_handle Entity_handle;
typedef struct Entity_node Entity_node;
typedef struct Entity_list Entity_list;
typedef struct Waypoint Waypoint;
typedef struct Waypoint_list Waypoint_list;
typedef void (*Waypoint_action_proc)(Game *gp, Entity *this_entity);
typedef void (*Entity_collide_proc)(Game *gp, Entity *this_entity, Entity *other_entity);
typedef void (*Entity_interact_proc)(Game *gp, Entity *this_entity, Entity *other_entity);
typedef void (*Entity_drop_proc)(Game *gp, Entity *this_entity, Entity *other_entity);

typedef enum Game_state {
#define X(state) GAME_STATE_##state,
  GAME_STATES
#undef X
    GAME_STATE_MAX,
} Game_state;

char *Game_state_strings[GAME_STATE_MAX] = {
#define X(state) #state,
  GAME_STATES
#undef X
};

typedef enum Game_flag_index {
  GAME_FLAG_INDEX_INVALID = -1,
#define X(flag) GAME_FLAG_INDEX_##flag,
  GAME_FLAGS
#undef X
    GAME_FLAG_INDEX_MAX,
} Game_flag_index;

STATIC_ASSERT(GAME_FLAG_INDEX_MAX < 64, number_of_game_flags_is_less_than_64);

#define X(flag) const Game_flags GAME_FLAG_##flag = (Game_flags)(1ull<<GAME_FLAG_INDEX_##flag);
GAME_FLAGS
#undef X

typedef enum Game_debug_flag_index {
  GAME_DEBUG_FLAG_INDEX_INVALID = -1,
#define X(flag) GAME_DEBUG_FLAG_INDEX_##flag,
  GAME_DEBUG_FLAGS
#undef X
    GAME_DEBUG_FLAG_INDEX_MAX,
} Game_debug_flag_index;

STATIC_ASSERT(GAME_DEBUG_FLAG_INDEX_MAX < 64, number_of_game_debug_flags_is_less_than_64);

#define X(flag) const Game_debug_flags GAME_DEBUG_FLAG_##flag = (Game_debug_flags)(1ull<<GAME_DEBUG_FLAG_INDEX_##flag);
GAME_DEBUG_FLAGS
#undef X

typedef enum Input_flag_index {
  INPUT_FLAG_INDEX_INVALID = -1,
#define X(flag) INPUT_FLAG_INDEX_##flag,
  INPUT_FLAGS
#undef X
    INPUT_FLAG_INDEX_MAX,
} Input_flag_index;

STATIC_ASSERT(INPUT_FLAG_INDEX_MAX < 64, number_of_input_flags_is_less_than_64);

#define X(flag) const Input_flags INPUT_FLAG_##flag = (Input_flags)(1ull<<INPUT_FLAG_INDEX_##flag);
INPUT_FLAGS
#undef X
const Input_flags INPUT_FLAG_MOVE = INPUT_FLAG_MOVE_FORWARD | INPUT_FLAG_MOVE_LEFT | INPUT_FLAG_MOVE_RIGHT | INPUT_FLAG_MOVE_BACKWARD;

typedef enum Entity_kind {
  ENTITY_KIND_INVALID = 0,
#define X(kind) ENTITY_KIND_##kind,
  ENTITY_KINDS
#undef X
    ENTITY_KIND_MAX,
} Entity_kind;

#define X(kind) const Entity_kind_mask ENTITY_KIND_MASK_##kind = (Entity_kind_mask)(1ull<<ENTITY_KIND_##kind);
ENTITY_KINDS
#undef X

char *Entity_kind_strings[ENTITY_KIND_MAX] = {
  "",
#define X(kind) #kind,
  ENTITY_KINDS
#undef X
};

STATIC_ASSERT(ENTITY_KIND_MAX < 64, number_of_entity_kinds_is_less_than_64);

typedef enum Entity_order {
  ENTITY_ORDER_INVALID = -1,
#define X(order) ENTITY_ORDER_##order,
  ENTITY_ORDERS
#undef X
    ENTITY_ORDER_MAX,
} Entity_order;

typedef enum Entity_control {
  ENTITY_CONTROL_NONE = 0,
#define X(control) ENTITY_CONTROL_##control,
  ENTITY_CONTROLS
#undef X
    ENTITY_CONTROL_MAX
} Entity_control;

typedef enum Entity_flag_index {
  ENTITY_FLAG_INDEX_INVALID = -1,
#define X(flag) ENTITY_FLAG_INDEX_##flag,
  ENTITY_FLAGS
#undef X
    ENTITY_FLAG_INDEX_MAX,
} Entity_flag_index;

#define X(flag) const Entity_flags ENTITY_FLAG_##flag = (Entity_flags)(1ull<<ENTITY_FLAG_INDEX_##flag);
ENTITY_FLAGS
#undef X

STATIC_ASSERT(ENTITY_FLAG_INDEX_MAX < 64, number_of_entity_flags_is_less_than_64);

typedef enum Particle_flag_index {
  PARTICLE_FLAG_INDEX_INVALID = -1,
#define X(flag) PARTICLE_FLAG_INDEX_##flag,
  PARTICLE_FLAGS
#undef X
    PARTICLE_FLAG_INDEX_MAX,
} Particle_flag_index;

#define X(flag) const Particle_flags PARTICLE_FLAG_##flag = (Particle_flags)(1u<<PARTICLE_FLAG_INDEX_##flag);
PARTICLE_FLAGS
#undef X

STATIC_ASSERT(PARTICLE_FLAG_INDEX_MAX < 32, number_of_particle_flags_is_less_than_32);

typedef enum Particle_emitter {
  PARTICLE_EMITTER_INVALID = 0,
#define X(kind) PARTICLE_EMITTER_##kind,
  PARTICLE_EMITTERS
#undef X
    PARTICLE_EMITTER_MAX,
} Particle_emitter;

typedef enum Gun_kind {
  GUN_KIND_INVALID = 0,
#define X(kind) GUN_KIND_##kind,
  GUN_KINDS
#undef X
    GUN_KIND_MAX,
} Gun_kind;

#define X(kind) const Gun_kind_mask GUN_KIND_MASK_##kind = (Gun_kind_mask)(1ull<<GUN_KIND_##kind);
GUN_KINDS
#undef X

STATIC_ASSERT(GUN_KIND_MAX < 64, number_of_bullet_emitter_kinds_is_less_than_64);

typedef enum Gun_flag_index {
  GUN_FLAG_INDEX_INVALID = -1,
#define X(flag) GUN_FLAG_INDEX_##flag,
  GUN_FLAGS
#undef X
    GUN_FLAG_INDEX_MAX,
} Gun_flag_index;

STATIC_ASSERT(GUN_FLAG_INDEX_MAX < 64, number_of_bullet_emitter_flags_is_less_than_64);

#define X(flag) const Gun_flags GUN_FLAG_##flag = (Gun_flags)(1ull<<GUN_FLAG_INDEX_##flag);
GUN_FLAGS
#undef X

DECL_ARR_TYPE(Entity_ptr);
DECL_SLICE_TYPE(Entity_ptr);
DECL_ARR_TYPE(Rectangle);
DECL_SLICE_TYPE(Rectangle);
DECL_SLICE_TYPE(u8);
DECL_ARR_TYPE(Entity);
DECL_SLICE_TYPE(Entity);


/*
 * struct bodies
 */

struct Particle {
  Particle_flags flags;

  f32 live;
  f32 lifetime;

  Vector2 pos;
  Vector2 vel;
  f32     radius; /* this says radius, but the particles are squares */
  f32     shrink;
  f32     friction;
  Color   begin_tint;
  Color   end_tint;
};

struct Gun {
  Gun_flags flags;

  Entity_kind_mask bullet_collision_mask;
  Entity_kind      bullet_kind;

  Gun_kind kind;

  Vector2 dir;
  f32 initial_angle;

  f32 radius;

  s32 n_arms;
  f32 arms_occupy_circle_sector_percent;

  Vector2 bullet_point_bag[MAX_BULLETS_IN_BAG];

  f32 cam_shake_magnitude;
  f32 cam_shake_duration;

  s32          n_bullets;
  f32          bullet_arm_width;
  f32          bullet_radius;
  f32          bullet_vel;
  f32          bullet_friction;
  s32          bullet_damage;
  f32          bullet_lifetime;
  Color        bullet_bounds_color;
  Color        bullet_fill_color;
  Entity_flags bullet_flags;

  Particle_emitter bullet_spawn_particle_emitter;
  Particle_emitter bullet_death_particle_emitter;

  Sprite bullet_sprite;
  f32    bullet_sprite_scale;
  f32    bullet_sprite_rotation;
  Color  bullet_sprite_tint;

  Sound sound;

  f32 burst_cooldown;
  f32 burst_timer;
  s32 burst_shots;
  s32 burst_shots_fired;

  f32 cooldown_duration;
  f32 cooldown_timer;
  b32 cooling_down;

  b16 cocked;
  s16 shoot;
};

struct Waypoint_list {
  Waypoint *first;
  Waypoint *last;
  s64 count;
  Waypoint_action_proc action;
};

struct Waypoint {
  Waypoint *next;
  Waypoint *prev;
  Vector2   pos;
  f32       radius;
  u32       tag;
};

struct Entity_list {
  Entity_node *first;
  Entity_node *last;
  s64 count;
};

struct Entity_handle {
  u64     uid;
  Entity *ep;
};

struct Entity_node {
  Entity_node *next;
  Entity_node *prev;
  union {
    Entity_handle handle;
    struct {
      u64     uid;
      Entity *ep;
    };
  };
};

struct Entity {
  b32 live;

  Entity_kind    kind;
  Entity_order   update_order;
  Entity_order   draw_order;

  Entity_control control;

  Entity_flags   flags;

  Entity *free_list_next;

  u64 uid;
  u64 tag;

  Entity_handle parent_handle;

  union {
    Entity_handle child_handle;
    Entity_handle holding_gun_handle;
  };

  Entity_list *child_list;
  Entity_list *parent_list;
  Entity_node *list_node;

  Vector2 look_dir;
  Vector2 accel;
  Vector2 vel;
  Vector2 pos;
  f32     radius;
  f32     interact_radius;
  f32     scalar_vel;
  f32     friction;
  f32     look_angle;
  f32     spin_vel;

  f32 shooting_pause_timer;
  f32 start_shooting_delay;

  b32 received_collision;
  s32 received_damage;

  s32 damage_amount;

  s32 health;

  Color bounds_color;
  Color fill_color;

  Waypoint_list waypoints;
  Waypoint     *cur_waypoint;

  Particle_emitter particle_emitter;
  Particle_emitter spawn_particle_emitter;
  Particle_emitter death_particle_emitter;

  Gun gun;

  Vector2 being_held_offset;

  Entity_kind_mask apply_collision_mask;
  Entity_collide_proc collide_proc;
  Entity_interact_proc interact_proc;
  Entity_drop_proc drop_proc;

  // TODO sprite offset
  Sprite  sprite;
  f32     sprite_scale;
  f32     sprite_rotation;
  Color   sprite_tint;
  Vector2 sprite_offset;
  Vector2 manual_sprite_origin;

  Color  effect_tint;
  f32    effect_tint_timer_vel;
  f32    effect_tint_duration;
  f32    effect_tint_timer;

  f32 invulnerability_timer;

  f32 life_time_duration;
  f32 life_timer;

  Sound spawn_sound;
  Sound hurt_sound;
  Sound die_sound;

};

struct Map_data {
  Slice_u8 tiles;
};

// TODO level editor
struct Editor {
  u8 *tiles;
  Arr_Entity static_entities;
};

struct Game {

  f32 dt;
  b32 quit;

  Game_state state;
  Game_state next_state;

  Game_flags       flags;
  Game_debug_flags debug_flags;

  Input_flags input_flags;

  u64 frame_index;

  Arena *main_arena;
  Arena *frame_arena;
  Arena *level_arena;

  Font font;

  Camera2D cam;

  struct {
    b32     on;
    f32     duration;
    f32     timer;
    f32     magnitude;
    Vector2 offset;
  } cam_shake;

  struct {
    b32 on;
    f32 duration;
    f32 timer;
    f32 magnitude;
    f32 zoom_offset;
    f32 save_zoom;
  } cam_pulsate;

  Texture2D particle_atlas;
  Texture2D sprite_atlas;
  Texture2D debug_background;

  u64 entity_uid;
  Entity *entities;
  u64 entities_allocated;
  Entity *entity_free_list;

  Particle *particles;
  u64 particles_pos;

  u32 live_entities;
  u32 live_particles;
  u32 live_enemies;

  Entity *player;
  Entity_handle player_handle;

  s16 level;

  s16 phase_index;

#if 0
  struct {
    u32 flags;
    b16 check_if_finished;
    b16 set_timer;
    s32 accumulator[4];
    f32 timer[4];
    union {
      Waypoint_list *waypoints;
    };
  } phase;
#endif

  f32 gameover_pre_delay;
  f32 gameover_type_char_timer;
  s32 gameover_chars_typed;

  s32 score;
  s32 player_health;

  Arena_scope intro_scope;

  //Sound avenger_bullet_sound;
  //Sound crab_hurt_sound;
  //Sound health_pickup_sound;
  //Sound avenger_hurt_sound;
  //Sound bomb_sound;
  //Sound powerup_sound;
  //Sound boss_die;

  Music music;
  b32 music_pos_saved;
  f32 music_pos;

  Editor editor;

};


/*
 * function headers
 */

float get_random_float(float min, float max, int steps);

Game* game_init(void);
void game_load_assets(Game *gp);
void game_unload_assets(Game *gp);
void game_update_and_draw(Game *gp);
void game_close(Game *gp);
void game_reset(Game *gp);
void game_main_loop(Game *gp);
void game_level_end(Game *gp);

void camera_shake(Game *gp, float duration, float magnitude);
void camera_pulsate(Game *gp, float duration, float magnitude);

Entity* entity_spawn(Game *gp);
void    entity_die(Game *gp, Entity *ep);

Entity_list* push_entity_list(Game *gp);
Entity_node* push_entity_list_node(Game *gp);
Entity_list* get_entity_list_by_id(Game *gp, int list_id);
Entity_node* entity_list_append(Game *gp, Entity_list *list, Entity *ep);
Entity_node* entity_list_push_front(Game *gp, Entity_list *list, Entity *ep);
b32          entity_list_remove(Game *gp, Entity_list *list, Entity *ep);

Waypoint* waypoint_list_append(Game *gp, Waypoint_list *list, Vector2 pos, float radius);
Waypoint* waypoint_list_append_tagged(Game *gp, Waypoint_list *list, Vector2 pos, float radius, u64 tag);

Entity* entity_from_uid(Game *gp, u64 uid);
Entity* entity_from_handle(Entity_handle handle);

Entity_handle handle_from_entity(Entity *ep);
b32 is_valid_handle(Entity_handle handle);

Entity* spawn_player(Game *gp);
Entity* spawn_health_pack(Game *gp);
Entity* spawn_shotgun(Game *gp);
Entity* spawn_assault_rifle(Game *gp);
Entity* spawn_parent(Game *gp);

void pickup_health_pack(Game *gp, Entity *a, Entity *b);

void pickup_shotgun(Game *gp, Entity *a, Entity *b);
void pickup_assault_rifle(Game *gp, Entity *a, Entity *b);

void drop_weapon(Game *gp, Entity *weapon, Entity *wielder);

b32 check_circle_all_inside_rec(Vector2 center, float radius, Rectangle rec);

void entity_emit_particles(Game *gp, Entity *ep);

void entity_shoot_gun(Game *gp, Entity *ep);

b32  entity_check_collision(Game *gp, Entity *a, Entity *b);

void draw_sprite(Game *gp, Entity *ep);
void sprite_update(Game *gp, Entity *ep);
Sprite_frame sprite_current_frame(Sprite sp);
b32 sprite_at_keyframe(Sprite sp, s32 keyframe);
b32 sprite_equals(Sprite a, Sprite b);

s64 tile_from_point(Vector2 p);
Vector2 point_from_tile(s64 tile);


/*
 * entity settings
 */

const Vector2 PLAYER_INITIAL_POS = { 500 , 500 };
const Vector2 DEBUG_PLAYER_INITIAL_POS = { 500 , 500 };
const float PLAYER_FRICTION = 40.0f;
const Vector2 PLAYER_LOOK_DIR = { 0, -1 };
const s32 PLAYER_HEALTH = 10;
const float PLAYER_BOUNDS_RADIUS = 10;
const float PLAYER_SPRITE_Y_OFFSET = 14;
const float PLAYER_SPRITE_SCALE = 1.0f;
const float PLAYER_ACCEL = 1.2e4;
const float PLAYER_SLOW_FACTOR = 0.5f;
const Color PLAYER_BOUNDS_COLOR = { 255, 0, 0, 255 };
const float PLAYER_LOOK_RADIUS = 200.0f;

const float PICKUP_BOUNDS_RADIUS = 50.0f;

const float WEAPON_INTERACT_RADIUS = 50;
const float WEAPON_RADIUS = 12;

const Entity_flags DEFAULT_BULLET_FLAGS =
ENTITY_FLAG_APPLY_COLLISION |
ENTITY_FLAG_APPLY_COLLISION_DAMAGE |
ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
ENTITY_FLAG_DYNAMICS |
0;

const Entity_kind_mask PLAYER_BULLET_APPLY_COLLISION_MASK =
ENTITY_KIND_MASK_RAPTOR |
ENTITY_KIND_MASK_BOSS   |
0;

const Entity_kind_mask ENEMY_KIND_MASK =
ENTITY_KIND_MASK_RAPTOR |
ENTITY_KIND_MASK_BOSS   |
ENTITY_KIND_MASK_PARENT |
0;

const float TYPING_SPEED   = 0.055f;
const float COUNTING_SPEED   = 0.0001f;

const float LEVEL_DELAY_TIME = 1.9f;
const float LEVEL_BANNER_FONT_SIZE    = 80.0f;
const float LEVEL_BANNER_FONT_SPACING = 8.0f;
const Color LEVEL_BANNER_FONT_COLOR   = BLACK;

const float GAME_OVER_FONT_SIZE    = 100.0f;
const float GAME_OVER_FONT_SPACING = 10.0f;
const Color GAME_OVER_FONT_COLOR   = RED;

const Color MY_GOLD = { 13, 88, 20, 255 };
const Color MILITARY_GREEN = { 50, 60, 57 , 255 };
const Color MUSTARD = { 234, 200, 0, 255 };



/*
 * generated code
 */

#include "sprite_data.c"

//#include "sound_data.c"


/*
 * globals
 */

bool editor_mouse_grab = false;

bool cleared_screen_on_victory = false;

bool has_started_before = false;
bool at_boss_level = false;

bool  title_screen_key_pressed = false;
int   title_screen_chars_deleted = 0;
float title_screen_type_char_timer = 0;


const float INTRO_SCREEN_FADE_TIME = 1.0f;
float intro_screen_fade_timer = 0.0f;
float intro_screen_colonel_delay = 1.0f;
int   intro_screen_cur_message = 0;
int   intro_screen_chars_typed = 0;
float intro_screen_type_char_timer = 0;
bool  intro_screen_cursor_on = false;
float intro_screen_cursor_blink_timer = 0;
float intro_screen_pre_message_delay = 0.8f;

bool  game_over_hint_on = true;
float game_over_hint_timer = 0;

float victory_screen_pre_delay = 1.0f;
float victory_screen_pre_counter_delay = 0.5f;
bool  victory_screen_finished_typing_banner = false;
bool  victory_screen_finished_incrementing_score = false;
int   victory_screen_chars_typed = 0;
int   victory_screen_type_dir = 1;
float victory_screen_show_timer = 2.5f;
int   victory_screen_target_len = STRLEN("VICTORY");
float victory_screen_type_char_timer = 0;
float victory_screen_inc_score_timer = 0;
float victory_screen_score_delay = 0.9f;
int   victory_screen_score_counter = 0;
bool  victory_screen_restart_hint_on = false;
float victory_screen_hint_timer = 0;

float boss_start_shooting_delay = 0;
float boss_shoot_pause = 0;




/* 
 * function bodies
 */

#define frame_push_array(T, n) (T*)push_array(gp->frame_arena, T, n)
#define level_push_array(T, n) (T*)push_array(gp->level_arena, T, n)

#define frame_push_struct(T) (T*)push_array(gp->frame_arena, T, 1)
#define level_push_struct(T) (T*)push_array(gp->level_arena, T, 1)

#define frame_push_struct_no_zero(T) (T*)push_array_no_zero(gp->frame_arena, T, 1)
#define level_push_struct_no_zero(T) (T*)push_array_no_zero(gp->level_arena, T, 1)

#define frame_push_array_no_zero(T, n) (T*)push_array_no_zero(gp->frame_arena, T, n)
#define level_push_array_no_zero(T, n) (T*)push_array_no_zero(gp->level_arena, T, n)

#define frame_scope_begin() scope_begin(gp->frame_arena)
#define frame_scope_end(scope) scope_end(scope)

#define level_scope_begin() scope_begin(gp->level_arena)
#define level_scope_end(scope) scope_end(scope)

#define entity_is_part_of_list(ep) (ep->list_node && ep->list_node->ep == ep)

Entity *entity_spawn(Game *gp) {
  Entity *ep = 0;

  if(gp->entity_free_list) {
    ep = gp->entity_free_list;
    gp->entity_free_list = ep->free_list_next;
  } else {
    ASSERT(gp->entities_allocated < MAX_ENTITIES);
    ep = &gp->entities[gp->entities_allocated++];
  }

  gp->entity_uid++;

  *ep =
    (Entity){
      .live = 1,
      .uid = gp->entity_uid,
    };

  return ep;
}

void entity_die(Game *gp, Entity *ep) {
  ep->free_list_next = gp->entity_free_list;
  gp->entity_free_list = ep;
  ep->live = 0;

  if(entity_is_part_of_list(ep)) {
    ASSERT(ep->parent_list);
    Entity_list *list = ep->parent_list;
    list->count--;
    dll_remove(list->first, list->last, ep->list_node);
  }

}

force_inline Entity_list* push_entity_list(Game *gp) {
  Entity_list *list = push_array(gp->level_arena, Entity_list, 1);
  return list;
}

force_inline Entity_node* push_entity_list_node(Game *gp) {
  Entity_node *e_node = push_array(gp->level_arena, Entity_node, 1);
  return e_node;
}

Entity_node* entity_list_append(Game *gp, Entity_list *list, Entity *ep) {
  ASSERT(list);

  Entity_node *e_node = push_entity_list_node(gp);

  e_node->uid = ep->uid;
  e_node->ep = ep;

  ep->parent_list = list;
  ep->list_node = e_node;

  dll_push_back(list->first, list->last, e_node);
  list->count++;

  return e_node;
}

Entity_node* entity_list_push_front(Game *gp, Entity_list *list, Entity *ep) {
  ASSERT(list);

  Entity_node *e_node = push_entity_list_node(gp);

  e_node->uid = ep->uid;
  e_node->ep = ep;

  dll_push_front(list->first, list->last, e_node);
  list->count++;

  return e_node;
}

b32 entity_list_remove(Game *gp, Entity_list *list, Entity *ep) {
  ASSERT(list);

  Entity_node *found = 0;

  for(Entity_node *node = list->first; node; node = node->next) {
    if(node->ep == ep) {
      found = node;
      break;
    }
  }

  if(found) {
    dll_remove(list->first, list->last, found);
  }

  list->count--;

  return (b32)(!!found);
}

Entity* entity_from_uid(Game *gp, u64 uid) {
  Entity *ep = 0;

  for(int i = 0; i < gp->entities_allocated; i++) {
    if(gp->entities[i].uid == uid) {
      ep = gp->entities + i;
      break;
    }
  }

  return ep;
}

Entity* entity_from_handle(Entity_handle handle) {
  Entity *ep = 0;

  if(handle.ep) {
    if(handle.ep->uid == handle.uid) {
      ep = handle.ep;
    }
  }

  return ep;
}

Entity_handle handle_from_entity(Entity *ep) {
  return (Entity_handle){ .uid = ep->uid, .ep = ep };
}

force_inline b32 is_valid_handle(Entity_handle handle) {
  return (handle.uid > 0);
}

force_inline Waypoint* waypoint_list_append(Game *gp, Waypoint_list *list, Vector2 pos, float radius) {
  return waypoint_list_append_tagged(gp, list, pos, radius, 0);
}

Waypoint* waypoint_list_append_tagged(Game *gp, Waypoint_list *list, Vector2 pos, float radius, u64 tag) {
  Waypoint *wp = push_array_no_zero(gp->level_arena, Waypoint, 1);
  wp->pos = pos;
  wp->radius = radius;
  wp->tag = tag;

  dll_push_back(list->first, list->last, wp);

  list->count++;

  return wp;
}

Entity* spawn_player(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->control = ENTITY_CONTROL_PLAYER;
  ep->kind = ENTITY_KIND_PLAYER;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_HAS_GUN |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->look_dir = PLAYER_LOOK_DIR;
#ifdef DEBUG
  ep->pos = DEBUG_PLAYER_INITIAL_POS;
#else
  ep->pos = PLAYER_INITIAL_POS;
#endif

  ep->health = PLAYER_HEALTH;

  //ep->hurt_sound = gp->avenger_hurt_sound;

  ep->sprite = SPRITE_DUDE;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = PLAYER_SPRITE_SCALE;

  ep->death_particle_emitter = PARTICLE_EMITTER_BIG_PLANE_EXPLOSION;

  //ep->bullet_emitter.kind = GUN_KIND_AVENGER;
  //ep->bullet_emitter.flags =
  //  0;

  ep->bounds_color = PLAYER_BOUNDS_COLOR;

  ep->radius = PLAYER_BOUNDS_RADIUS;
  ep->friction = PLAYER_FRICTION;

  return ep;
}

Entity* spawn_parent(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_PARENT;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_NOT_ON_SCREEN |
    ENTITY_FLAG_DIE_IF_CHILD_LIST_EMPTY |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->radius = 16.0f;
  ep->bounds_color = GREEN;

  return ep;
}

Entity* spawn_shotgun(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_SHOTGUN;

  ep->flags =
    ENTITY_FLAG_HAS_GUN |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_IS_INTERACTABLE |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    ENTITY_FLAG_HAS_SPRITE |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  // nocheckin
  ep->pos = (Vector2){ .x = 100, . y = 900 }; 

  ep->apply_collision_mask =
    ENTITY_KIND_MASK_PLAYER |
    0;

  ep->interact_proc = pickup_shotgun;
  ep->drop_proc     = drop_weapon;

  ep->gun.kind = GUN_KIND_SHOTGUN;

  ep->death_particle_emitter = PARTICLE_EMITTER_WEAPON_DIE_PUFF;

  ep->bounds_color = GREEN;
  ep->sprite = SPRITE_SHOTGUN_SIDE;
  ep->sprite_scale = 1.0f;
  ep->sprite_tint = WHITE;

  ep->interact_radius = WEAPON_INTERACT_RADIUS;
  ep->radius = WEAPON_RADIUS;

  return ep;
}

Entity* spawn_assault_rifle(Game *gp) {
  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_ASSAULT_RIFLE;

  ep->flags =
    ENTITY_FLAG_HAS_GUN |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_IS_INTERACTABLE |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    ENTITY_FLAG_HAS_SPRITE |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  // nocheckin
  ep->pos = (Vector2){ .x = 500, . y = 900 }; 

  ep->apply_collision_mask =
    ENTITY_KIND_MASK_PLAYER |
    0;

  ep->interact_proc = pickup_assault_rifle;
  ep->drop_proc     = drop_weapon;

  ep->gun.kind = GUN_KIND_ASSAULT_RIFLE;

  ep->death_particle_emitter = PARTICLE_EMITTER_WEAPON_DIE_PUFF;

  ep->bounds_color = GREEN;
  ep->sprite = SPRITE_ASSAULT_RIFLE_SIDE;
  ep->sprite_scale = 1.0f;
  ep->sprite_tint = WHITE;

  ep->interact_radius = WEAPON_INTERACT_RADIUS;
  ep->radius = WEAPON_RADIUS;

  return ep;
}

Entity* spawn_health_pack(Game *gp) {

  Entity *ep = entity_spawn(gp);

  ep->kind = ENTITY_KIND_HEALTH_PACK;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_EMIT_DEATH_PARTICLES |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = (Vector2){ .x = (float)GetRandomValue(200, WINDOW_WIDTH-200), . y = -0.4*WINDOW_HEIGHT }; 
  ep->vel = (Vector2){ .y = (float)GetRandomValue(780, 800), };
  ep->friction = 0.45f;

  ep->collide_proc = pickup_health_pack;

  ep->death_particle_emitter = PARTICLE_EMITTER_GREEN_PUFF;

  ep->apply_collision_mask =
    ENTITY_KIND_MASK_PLAYER |
    0;

  ep->bounds_color = GREEN;
  ep->sprite = SPRITE_HEALTH;
  ep->sprite_scale = 1.0f;
  ep->sprite_tint = WHITE;

  ep->radius = 40;

  return ep;
}

void pickup_assault_rifle(Game *gp, Entity *a, Entity *b) {
  ASSERT(a->kind == ENTITY_KIND_ASSAULT_RIFLE);
  ASSERT(b->kind == ENTITY_KIND_PLAYER);

  Entity *rifle = a;
  Entity *player = b;

  {
    Entity *holding = entity_from_handle(player->child_handle);

    if(holding) {
      holding->drop_proc(gp, holding, player);
    }

  }

  rifle->flags ^=
    ENTITY_FLAG_IS_INTERACTABLE |
    ENTITY_FLAG_MANUAL_SPRITE_ORIGIN |
    0;

  rifle->parent_handle = handle_from_entity(player);
  player->holding_gun_handle = handle_from_entity(rifle);

  rifle->sprite = SPRITE_ASSAULT_RIFLE_TOP;
  ///rifle->sprite_offset = (Vector2){ .x = -0.5, };
  rifle->control = ENTITY_CONTROL_GUN_BEING_HELD;
  rifle->being_held_offset = (Vector2){ .x = -6, .y = 12 };

}

void pickup_shotgun(Game *gp, Entity *a, Entity *b) {

  ASSERT(a->kind == ENTITY_KIND_SHOTGUN);
  ASSERT(b->kind == ENTITY_KIND_PLAYER);

  Entity *shotgun = a;
  Entity *player = b;

  {
    Entity *holding = entity_from_handle(player->child_handle);

    if(holding) {
      holding->drop_proc(gp, holding, player);
    }

  }

  shotgun->flags ^=
    ENTITY_FLAG_IS_INTERACTABLE |
    ENTITY_FLAG_MANUAL_SPRITE_ORIGIN |
    0;

  shotgun->parent_handle = handle_from_entity(player);
  player->holding_gun_handle = handle_from_entity(shotgun);

  shotgun->sprite = SPRITE_SHOTGUN_TOP;
  shotgun->control = ENTITY_CONTROL_GUN_BEING_HELD;
  shotgun->being_held_offset = (Vector2){ .x = -6, .y = 12 };

}

void drop_weapon(Game *gp, Entity *weapon, Entity *wielder) {

  weapon->flags |=
    ENTITY_FLAG_IS_INTERACTABLE |
    0;

  weapon->parent_handle = (Entity_handle){0};
  wielder->child_handle = (Entity_handle){0};

  weapon->control = ENTITY_CONTROL_NONE;

}

void pickup_health_pack(Game *gp, Entity *a, Entity *b) {
  ASSERT(a->kind == ENTITY_KIND_HEALTH_PACK);
  ASSERT(b->kind == ENTITY_KIND_PLAYER);

  b->health += 5;
  if(b->health >= PLAYER_HEALTH) {
    b->health = PLAYER_HEALTH;
  }

  b->effect_tint = GREEN;

  b->flags |= ENTITY_FLAG_APPLY_EFFECT_TINT;

  b->effect_tint_duration = 0.7f;
  b->effect_tint_timer_vel = 1.0f;

  b->effect_tint_timer = b->effect_tint_duration;

  // nochecking sounds
  //SetSoundPan(gp->health_pickup_sound, Normalize(a->pos.x, WINDOW_WIDTH, 0));
  //SetSoundVolume(gp->health_pickup_sound, 0.3);
  //PlaySound(gp->health_pickup_sound);

}

void camera_shake(Game *gp, float duration, float magnitude) {

  gp->flags |= GAME_FLAG_CAMERA_SHAKE;
  gp->cam_shake.on = 0;
  gp->cam_shake.duration = duration;
  gp->cam_shake.timer = 0;
  gp->cam_shake.magnitude = magnitude;
  gp->cam_shake.offset = (Vector2){0};

}

void camera_pulsate(Game *gp, float duration, float magnitude) {

  gp->flags |= GAME_FLAG_CAMERA_PULSATE;
  gp->cam_pulsate.on = 0;
  gp->cam_pulsate.duration = duration;
  gp->cam_pulsate.timer = 0;
  gp->cam_pulsate.magnitude = magnitude;
  gp->cam_pulsate.zoom_offset = 0;
  gp->cam_pulsate.save_zoom = gp->cam.zoom;

}

b32 check_circle_all_inside_rec(Vector2 center, float radius, Rectangle rec) {
  b32 inside = 0;

  float xmin = rec.x + radius;
  float ymin = rec.y + radius;
  float xmax = rec.x + rec.width - radius;
  float ymax = rec.y + rec.height - radius;

  if(center.x > xmin && center.x < xmax && center.y > ymin && center.y < ymax) {
    inside = 1;
  }

  return inside;
}

force_inline float get_random_float(float min, float max, int steps) {
  int val = GetRandomValue(0, steps);
  float result = Remap((float)val, 0.0f, (float)steps, min, max);
  return result;
}

void entity_emit_particles(Game *gp, Entity *ep) {

  Particle_emitter emitter = ep->particle_emitter;

  Particle buf[MAX_PARTICLES];
  s32 n_particles = 0;

  switch(emitter) {
    default:
      UNREACHABLE;
    case PARTICLE_EMITTER_BIG_PLANE_EXPLOSION:
      {
        Color tints[] = {
          YELLOW, ORANGE, RED, ColorAlpha(RED, 0.8f),
        };
        int amounts[] = { 300, 200, 40 };

        int base = 0;
        for(int ti = 0; ti < ARRLEN(tints)-1; ti++) {

          n_particles += GetRandomValue(amounts[ti], amounts[ti]+20);
          ASSERT(n_particles <= MAX_PARTICLES);

          int i = base;
          for(; i < n_particles; i++) {
            Particle *p = buf + i;
            *p = (Particle){0};

            p->lifetime = get_random_float(TARGET_FRAME_TIME * 20, TARGET_FRAME_TIME * 30, 5);

            p->pos = ep->pos;
            p->vel =
              Vector2Rotate((Vector2){ 0, -1 },
                  get_random_float(0, 2*PI, 150));

            p->vel = Vector2Scale(p->vel, (float)GetRandomValue(400, 700));

            p->radius = get_random_float(2.9f, 4.7f, 15);
            p->shrink = (0.6*p->radius)/p->lifetime;

            p->friction = (float)GetRandomValue(0, 2);

            p->begin_tint = tints[ti];
            p->end_tint = tints[ti+1];

          }
          base = i;
        }

      } break;
    case PARTICLE_EMITTER_WHITE_PUFF:
      {
        n_particles = GetRandomValue(100, 110);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 20, TARGET_FRAME_TIME * 26, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(1400, 1500));

          p->radius = get_random_float(2.0f, 4.0f, 5);
          p->shrink = (0.5*p->radius)/p->lifetime;

          p->friction = (float)GetRandomValue(8, 15);

          p->begin_tint = RAYWHITE;
          p->end_tint = ColorAlpha(p->begin_tint, 0.8);

        }

      } break;
    case PARTICLE_EMITTER_WEAPON_DIE_PUFF:
      {
        n_particles = GetRandomValue(10, 15);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 30, TARGET_FRAME_TIME * 40, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(80, 90));

          p->radius = get_random_float(0.9f, 1.7f, 4);
          p->shrink = (0.46*p->radius)/p->lifetime;


          p->friction = get_random_float(0.05f, 0.1f, 4);

          p->begin_tint = (Color){ 58, 58, 58, 255 };
          p->end_tint = ColorAlpha(p->begin_tint, 0.8);

        }

      } break;
    case PARTICLE_EMITTER_BROWN_PUFF:
      {
        n_particles = GetRandomValue(10, 15);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 30, TARGET_FRAME_TIME * 40, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(80, 90));

          p->radius = get_random_float(2.9f, 3.5f, 4);
          p->shrink = (0.36*p->radius)/p->lifetime;


          p->friction = get_random_float(0.05f, 0.1f, 4);

          p->begin_tint = (Color){ 102, 57, 49, 255 };
          p->end_tint = ColorAlpha(p->begin_tint, 0.8);

        }

      } break;
    case PARTICLE_EMITTER_GREEN_PUFF:
      {
        n_particles = GetRandomValue(10, 15);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 30, TARGET_FRAME_TIME * 40, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(80, 90));

          p->radius = get_random_float(2.9f, 3.5f, 4);
          p->shrink = (0.36*p->radius)/p->lifetime;


          p->friction = get_random_float(0.05f, 0.1f, 4);

          p->begin_tint = (Color){ 0, 255, 0, 255 };
          p->end_tint = ColorAlpha(p->begin_tint, 0.8);

        }

      } break;
    case PARTICLE_EMITTER_MASSIVE_BLOOD_PUFF:
      {
        n_particles = GetRandomValue(1100, 1300);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 20, TARGET_FRAME_TIME * 30, 5);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(400, 600));

          p->radius = get_random_float(3.7f, 4.6f, 10);
          p->shrink = (0.7*p->radius)/p->lifetime;

          p->friction = (float)GetRandomValue(0, 2);

          p->begin_tint = BLOOD;
          p->end_tint = ColorAlpha(BLOOD, 0.75f);

        }

      } break;
    case PARTICLE_EMITTER_BLOOD_PUFF:
      {
        n_particles = GetRandomValue(200, 210);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 20, TARGET_FRAME_TIME * 25, 5);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate((Vector2){ 0, -1 },
                get_random_float(0, 2*PI, 150));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(500, 600));

          p->radius = get_random_float(2.7f, 3.2f, 3);
          p->shrink = (0.7*p->radius)/p->lifetime;

          p->friction = (float)GetRandomValue(0, 2);

          p->begin_tint = BLOOD;
          p->end_tint = ColorAlpha(BLOOD, 0.75f);

        }

      } break;
    case PARTICLE_EMITTER_SPARKS:
      {
        n_particles = GetRandomValue(20, 25);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 10, TARGET_FRAME_TIME * 20, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate(Vector2Normalize(Vector2Negate(ep->vel)),
                get_random_float(-PI*0.37f, PI*0.37f, 200));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(1500, 1800));

          p->radius = get_random_float(2.0f, 3.2f, 4);
          p->shrink = 12.3f;

          p->friction = (float)GetRandomValue(0, 20);

          p->begin_tint = (Color){ 255, 188, 3, 255 };
          p->end_tint = ColorAlpha(p->begin_tint, 0.83);

        }

      } break;
    case PARTICLE_EMITTER_BLOOD_SPIT:
      {
        n_particles = GetRandomValue(50, 60);
        ASSERT(n_particles <= MAX_PARTICLES);

        for(int i = 0; i < n_particles; i++) {
          Particle *p = buf + i;
          *p = (Particle){0};

          p->lifetime = get_random_float(TARGET_FRAME_TIME * 10, TARGET_FRAME_TIME * 20, 10);

          p->pos = ep->pos;
          p->vel =
            Vector2Rotate(Vector2Normalize(Vector2Negate(ep->vel)),
                get_random_float(-PI*0.1f, PI*0.1f, 200));

          p->vel = Vector2Scale(p->vel, (float)GetRandomValue(1500, 1800));

          p->radius = get_random_float(1.6f, 2.2f, 4);
          p->shrink = 12.3f;

          p->friction = (float)GetRandomValue(0, 20);

          p->begin_tint = BLOOD;
          p->end_tint = ColorAlpha(BLOOD, 0.85);

        }

      } break;
  }

  for(int i = 0; i < n_particles; i++) {
    if(gp->particles_pos >= MAX_PARTICLES) {
      gp->particles_pos = 0;
    }

    gp->particles[gp->particles_pos++] = buf[i];

  }

}

void entity_shoot_gun(Game *gp, Entity *ep) {
  Gun *gun = &ep->gun;

  if(!gun->cocked) {

    gun->cocked = 1;

    switch(gun->kind) {
      default:
        break;
      case GUN_KIND_SHOTGUN:
        {
          gun->bullet_kind = ENTITY_KIND_PLAYER_BULLET;
          gun->bullet_collision_mask = PLAYER_BULLET_APPLY_COLLISION_MASK;
          gun->cooling_down = 0;
          gun->cooldown_duration = 1.0f;

          // TODO sound effects
          //gun->sound = kjsdhas;
          gun->radius = ep->radius + 5.0f;
          gun->n_arms = 6;
          gun->arms_occupy_circle_sector_percent = 0.06f;
          gun->n_bullets = 1;
          gun->bullet_arm_width = 0.0f;
          gun->bullet_radius = 3;
          gun->bullet_vel = 1000;
          gun->bullet_friction = 0.1f;
          gun->bullet_damage = 10;
          gun->bullet_bounds_color = GREEN;
          gun->bullet_sprite = SPRITE_SHOTGUN_PELLET;
          gun->bullet_sprite_tint = WHITE;
          gun->bullet_sprite_scale = 1.0f;
          gun->bullet_flags =
            ENTITY_FLAG_HAS_SPRITE |
            ENTITY_FLAG_EMIT_DEATH_PARTICLES |
            ENTITY_FLAG_APPLY_FRICTION |
            0;
          gun->cam_shake_duration = 0.18f;
          gun->cam_shake_magnitude = 2.0f;

          // nocheckin tiny sparks
          gun->bullet_death_particle_emitter = PARTICLE_EMITTER_SPARKS;

        } break;
      case GUN_KIND_ASSAULT_RIFLE:
        {

          gun->bullet_kind = ENTITY_KIND_PLAYER_BULLET;

          gun->flags |=
            GUN_FLAG_AUTOMATIC |
            0;

          gun->bullet_collision_mask = PLAYER_BULLET_APPLY_COLLISION_MASK;
          gun->cooling_down = 0;
          gun->cooldown_duration = 0.03f;

          // TODO sound effects
          //gun->sound = kjsdhas;
          gun->radius = ep->radius + 5.0f;
          gun->n_arms = 1;
          gun->arms_occupy_circle_sector_percent = 0.00f;
          gun->n_bullets = 1;
          gun->bullet_arm_width = 0.0f;
          gun->bullet_radius = 2;
          gun->bullet_vel = 1200;
          gun->bullet_friction = 0.0f;
          gun->bullet_damage = 5;
          gun->bullet_bounds_color = GREEN;
          gun->bullet_sprite = SPRITE_RIFLE_ROUND;
          gun->bullet_sprite_tint = WHITE;
          gun->bullet_sprite_scale = 1.0f;
          gun->bullet_flags =
            ENTITY_FLAG_HAS_SPRITE |
            ENTITY_FLAG_EMIT_DEATH_PARTICLES |
            ENTITY_FLAG_APPLY_FRICTION |
            0;
          gun->cam_shake_duration = 0.10f;
          gun->cam_shake_magnitude = 1.0f;

          // nocheckin tiny sparks
          gun->bullet_death_particle_emitter = PARTICLE_EMITTER_SPARKS;

        } break;
      case GUN_KIND_GRENADE_LAUNCHER:
        {
          UNIMPLEMENTED;
        } break;
      case GUN_KIND_FLAMETHROWER:
        {
          UNIMPLEMENTED;
        } break;
    }

  }

  //ASSERT(gun->bullet_kind != ENTITY_KIND_INVALID);

  if(gun->shoot) {

    if(gun->cooling_down) {
      if(gun->flags & GUN_FLAG_BURST) {

        if(gun->burst_shots_fired >= gun->burst_shots) {

          if(gun->cooldown_timer >= gun->cooldown_duration) {
            gun->cooling_down = 0;
            gun->shoot = 0;
            gun->burst_timer = 0.0f;
            gun->burst_shots_fired = 0;

          } else {
            gun->cooldown_timer += gp->dt;
          }

        } else {

          if(gun->burst_timer >= gun->burst_cooldown) {
            gun->cooling_down = 0;
            gun->burst_timer = 0;
            gun->burst_shots_fired++;
          } else {
            gun->burst_timer += gp->dt;
          }

        }

      } else {

        if(gun->cooldown_timer >= gun->cooldown_duration) {
          gun->cooling_down = 0;
          gun->shoot = 0;
        } else {
          gun->cooldown_timer += gp->dt;
        }

      }

      goto shoot_end;

    } else {
      gun->cooling_down = 1;
      gun->cooldown_timer = 0;
    }

    ASSERT(gun->n_arms > 0);
    ASSERT(gun->n_bullets > 0);

    Vector2 arm_dir;
    float arms_occupy_circle_sector_angle;
    float arm_step_angle;

    gun->dir = ep->look_dir;

    float arm_angle = 0.0;
    Matrix arm_transform;

    if(gun->n_arms == 1) {
      arms_occupy_circle_sector_angle = 0;
      arm_step_angle = 0;
      arm_transform = MatrixRotateZ(arm_angle);
      arm_dir = Vector2Transform(gun->dir, arm_transform);
    } else {
      ASSERT(gun->arms_occupy_circle_sector_percent > 0);

      arms_occupy_circle_sector_angle =
        (2*PI) * gun->arms_occupy_circle_sector_percent;
      arm_step_angle = arms_occupy_circle_sector_angle / (float)(gun->n_arms-1);
      arm_angle = -0.5*arms_occupy_circle_sector_angle;
      arm_transform = MatrixRotateZ(arm_angle);
      arm_dir = Vector2Transform(gun->dir, arm_transform);
    }

    for(int arm_i = 0; arm_i < gun->n_arms; arm_i++) {

      Vector2 step_dir = {0};
      Vector2 bullet_pos = Vector2Add(ep->pos, Vector2Scale(arm_dir, gun->radius));
      Vector2 positions[MAX_BULLETS_IN_BAG];

      if(gun->flags & GUN_FLAG_USE_POINT_BAG) {

        Vector2 dir = gun->dir;
        Vector2 dir_perp =
        {
          .x = dir.y, .y = -dir.x,
        };

        Matrix dir_transform =
        {
          .m0 = dir_perp.x, .m4 = dir.x,
          .m1 = dir_perp.y, .m5 = dir.y,
        };

        arm_transform = MatrixMultiply(arm_transform, dir_transform);

        ASSERT(gun->n_bullets < MAX_BULLETS_IN_BAG);

        for(int i = 0; i < gun->n_bullets; i++) {
          positions[i] = Vector2Add(bullet_pos, Vector2Transform(gun->bullet_point_bag[i], arm_transform));
        }

      } else {
        if(gun->n_bullets > 1) {
          ASSERT(gun->bullet_arm_width > 0);

          Vector2 arm_dir_perp = { arm_dir.y, -arm_dir.x };

          step_dir =
            Vector2Scale(arm_dir_perp, gun->bullet_arm_width/(float)(gun->n_bullets-1));

          bullet_pos =
            Vector2Add(bullet_pos, Vector2Scale(arm_dir_perp, -0.5*gun->bullet_arm_width));
        }
      }

      for(int bullet_i = 0; bullet_i < gun->n_bullets; bullet_i++) {
        Entity *bullet = entity_spawn(gp);


        bullet->kind = gun->bullet_kind;
        bullet->update_order = ENTITY_ORDER_FIRST;
        bullet->draw_order = ENTITY_ORDER_LAST;

        bullet->flags = DEFAULT_BULLET_FLAGS | gun->bullet_flags;

        bullet->bounds_color = gun->bullet_bounds_color;
        bullet->fill_color = gun->bullet_fill_color;
        bullet->look_dir = ep->look_dir;
        bullet->look_angle = ep->look_angle;

        bullet->sprite = gun->bullet_sprite;
        bullet->sprite_rotation = ep->look_angle * RAD2DEG;
        bullet->sprite_scale = gun->bullet_sprite_scale;
        bullet->sprite_tint = gun->bullet_sprite_tint;

        bullet->scalar_vel = gun->bullet_vel;
        bullet->vel = Vector2Scale(arm_dir, gun->bullet_vel);

        camera_shake(gp, gun->cam_shake_duration, gun->cam_shake_magnitude);

        if(IsSoundValid(gun->sound)) {
          SetSoundPan(gun->sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
          SetSoundVolume(gun->sound, 0.2);
          SetSoundPitch(gun->sound, get_random_float(0.98, 1.01, 4));
          PlaySound(gun->sound);
        }

        bullet->friction = gun->bullet_friction;
        //bullet->vel = Vector2Add(bullet->vel, Vector2Scale(gun->dir, Vector2DotProduct(gun->dir, ep->vel)));

        bullet->radius = gun->bullet_radius;

        bullet->apply_collision_mask = ep->gun.bullet_collision_mask;
        bullet->damage_amount = gun->bullet_damage;

        bullet->spawn_particle_emitter = gun->bullet_spawn_particle_emitter;
        bullet->death_particle_emitter = gun->bullet_death_particle_emitter;

        if(gun->bullet_flags & ENTITY_FLAG_HAS_LIFETIME) {
          bullet->life_time_duration = gun->bullet_lifetime;
        }

        if(gun->flags & GUN_FLAG_USE_POINT_BAG) {
          bullet->pos = positions[bullet_i];
        } else {
          bullet->pos = bullet_pos;
          bullet_pos = Vector2Add(bullet_pos, step_dir);
        }

      } /* bullet loop */

      arm_dir = Vector2Rotate(arm_dir, arm_step_angle);

    } /* arm loop */

shoot_end:;
  }

}

force_inline b32 entity_check_collision(Game *gp, Entity *a, Entity *b) {
  b32 result = 0;

  float sqr_min_dist = SQUARE(a->radius + b->radius);

  if(Vector2DistanceSqr(a->pos, b->pos) < sqr_min_dist) {
    result = 1;
  }

  return result;
}

void sprite_update(Game *gp, Entity *ep) {
  ep->sprite_rotation = ep->look_angle * RAD2DEG;

  Sprite *sp = &ep->sprite;
  if(!(sp->flags & SPRITE_FLAG_STILL)) {

    ASSERT(sp->fps > 0);

    if(sp->cur_frame < sp->total_frames) {
      sp->frame_counter++;

      if(sp->frame_counter >= (TARGET_FPS / sp->fps)) {
        sp->frame_counter = 0;
        sp->cur_frame++;
      }

    }

    if(sp->cur_frame >= sp->total_frames) {
      if(sp->flags & SPRITE_FLAG_INFINITE_REPEAT) {
        if(sp->flags & SPRITE_FLAG_PINGPONG) {
          sp->cur_frame--;
          sp->flags ^= SPRITE_FLAG_REVERSE;
        } else {
          sp->cur_frame = 0;
        }
      } else {
        sp->cur_frame--;
        sp->flags |= SPRITE_FLAG_AT_LAST_FRAME | SPRITE_FLAG_STILL;
        ASSERT(sp->cur_frame >= 0 && sp->cur_frame < sp->total_frames);
      }
    }

  }

}

Sprite_frame sprite_current_frame(Sprite sp) {
  s32 abs_cur_frame = sp.first_frame + sp.cur_frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    abs_cur_frame = sp.last_frame - sp.cur_frame;
  }

  Sprite_frame frame = __sprite_frames[abs_cur_frame];

  return frame;
}

b32 sprite_at_keyframe(Sprite sp, s32 keyframe) {
  b32 result = 0;

  s32 abs_cur_frame = sp.first_frame + sp.cur_frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    abs_cur_frame = sp.last_frame - sp.cur_frame;
  }

  if(abs_cur_frame == keyframe) {
    result = 1;
  }

  return result;
}

b32 sprite_equals(Sprite a, Sprite b) {
  return !!(a.id == b.id);
}

void draw_sprite(Game *gp, Entity *ep) {
  Sprite sp = ep->sprite;
  Vector2 pos = Vector2Add(ep->pos, ep->sprite_offset);
  f32 scale = ep->sprite_scale;
  f32 rotation = ep->sprite_rotation;
  Color tint = ep->sprite_tint;

  if(ep->flags & ENTITY_FLAG_APPLY_EFFECT_TINT) {
    tint = ColorLerp(ep->sprite_tint, ep->effect_tint, Normalize(ep->effect_tint_timer, 0, ep->effect_tint_duration));
  }

  ASSERT(sp.cur_frame >= 0 && sp.cur_frame < sp.total_frames);

  Sprite_frame frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    frame = __sprite_frames[sp.last_frame - sp.cur_frame];
  } else {
    frame = __sprite_frames[sp.first_frame + sp.cur_frame];
  }

  Rectangle source_rec =
  {
    .x = (float)frame.x,
    .y = (float)frame.y,
    .width = (float)frame.w,
    .height = (float)frame.h,
  };

  Rectangle dest_rec =
  {
    //.x = (pos.x - scale*0.5f*source_rec.width),
    //.y = (pos.y - scale*0.5f*source_rec.height),
    // because of rotation or origin idk??
    .x = pos.x,
    .y = pos.y,
    .width = scale*source_rec.width,
    .height = scale*source_rec.height,
  };

  if(sp.flags & SPRITE_FLAG_DRAW_MIRRORED_X) {
    source_rec.width *= -1;
  }

  if(sp.flags & SPRITE_FLAG_DRAW_MIRRORED_Y) {
    source_rec.height *= -1;
  }

  Vector2 origin = { dest_rec.width*0.5f, dest_rec.height*0.5f };

  if(ep->flags & ENTITY_FLAG_MANUAL_SPRITE_ORIGIN) {
  }

  //DrawRectanglePro(dest_rec, origin, rotation, SKYBLUE);
  DrawTexturePro(gp->sprite_atlas, source_rec, dest_rec, origin, rotation, tint);
}

Game* game_init(void) {

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1000, 800, GAME_TITLE);
  InitAudioDevice();
  //DisableCursor();

  //SetMasterVolume(GetMasterVolume() * 0.5);

  SetTargetFPS(TARGET_FPS);
  SetTextLineSpacing(10);
  SetTraceLogLevel(LOG_DEBUG);
  SetExitKey(0);

  Game *gp = os_alloc(sizeof(Game));
  memory_set(gp, 0, sizeof(Game));

  gp->main_arena  = arena_alloc(.size = MB(3));
  gp->level_arena = arena_alloc(.size = KB(16));
  gp->frame_arena = arena_alloc(.size = KB(8));

  gp->entities  = push_array_no_zero(gp->main_arena, Entity, MAX_ENTITIES);
  gp->particles = push_array_no_zero(gp->main_arena, Particle, MAX_PARTICLES);

  gp->editor.tiles = push_array(gp->main_arena, u8, TILES_COUNT);
  arr_init_ex(gp->editor.static_entities, gp->main_arena, 64);

  gp->cam =
    (Camera2D) {
      .zoom = INITIAL_CAMERA_ZOOM,
    };

  game_load_assets(gp);

  //PlayMusicStream(gp->music);

  game_reset(gp);

  return gp;
}

void game_load_assets(Game *gp) {
  gp->font = GetFontDefault();

  /* sprites */
  gp->debug_background = LoadTexture("./lizardman.png");
  //gp->background_texture = LoadTexture("./sprites/the_sea.png");
  gp->sprite_atlas = LoadTexture("./aseprite/atlas.png");
  SetTextureFilter(gp->sprite_atlas, TEXTURE_FILTER_POINT);

  /* sounds */

  /* music */

  if(gp->music_pos_saved) {
    gp->music_pos_saved = 0;
    SeekMusicStream(gp->music, gp->music_pos);
    PlayMusicStream(gp->music);
  }

}

void game_unload_assets(Game *gp) {

  //UnloadRenderTexture(gp->render_texture);
  //UnloadTexture(gp->sprite_atlas);
  //UnloadTexture(gp->background_texture);

  //gp->music_pos_saved = 1;
  //gp->music_pos = GetMusicTimePlayed(gp->music);
  //StopMusicStream(gp->music);
  //UnloadMusicStream(gp->music);

  //UnloadSound(gp->avenger_bullet_sound);
  //UnloadSound(gp->crab_hurt_sound);
  //UnloadSound(gp->health_pickup_sound);
  //UnloadSound(gp->avenger_hurt_sound);
  //UnloadSound(gp->bomb_sound);
  //UnloadSound(gp->powerup_sound);

}

void game_close(Game *gp) {
  game_unload_assets(gp);

  CloseWindow();
  CloseAudioDevice();
}

void game_reset(Game *gp) {

  gp->state = GAME_STATE_NONE;
  gp->next_state = GAME_STATE_NONE;

  gp->flags = 0;
  gp->cam.target = DEFAULT_CAMERA_TARGET;

  gp->player = 0;
  gp->player_handle = (Entity_handle){0};
  gp->entity_free_list = 0;
  gp->entities_allocated = 0;
  gp->live_entities = 0;

  gp->particles_pos = 0;
  gp->live_particles = 0;

  memory_set(gp->entities, 0, sizeof(Entity) * MAX_ENTITIES);
  memory_set(gp->particles, 0, sizeof(Particle) * MAX_PARTICLES);

  gp->frame_index = 0;

  arena_clear(gp->main_arena);
  arena_clear(gp->frame_arena);
  arena_clear(gp->level_arena);

  //memory_set(&gp->phase, 0, sizeof(gp->phase));
  gp->phase_index = 0;
  gp->level = 0;

  gp->score = 0;

  //SeekMusicStream(gp->music, 0);

  gp->gameover_pre_delay = 0;
  gp->gameover_type_char_timer = 0;
  gp->gameover_chars_typed = 0;

}

void game_level_end(Game *gp) {
  arena_clear(gp->level_arena);
  gp->level++;
  gp->next_state = GAME_STATE_START_LEVEL;
  gp->flags |= GAME_FLAG_PLAYER_CANNOT_SHOOT;
}

s64 tile_from_point(Vector2 p) {

  s64 col = Clamp((s64)(p.x * INV_TILE_SIZE), 0, TILE_COLS);
  s64 row = Clamp((s64)(p.y * INV_TILE_SIZE), 0, TILE_ROWS);

  s64 tile = col + row * TILE_COLS;

  return tile;

}

Vector2 point_from_tile(s64 tile) {

  s64 col = tile % TILE_ROWS;
  s64 row = tile / TILE_ROWS;

  Vector2 p =
  {
    .x = TILE_SIZE * (float)col,
    .y = TILE_SIZE * (float)row,
  };

  return p;
}

void game_main_loop(Game *gp) {
  /*
   * NOTE gameplay ideas
   *
   * Time limit to clear the level, postvoid style.
   * Justification: the USA is going to nuke the island you're on to get rid of the dinossaur-human hybrids.
   *
   * The theme is "unpredictable"
   *
   * Weapons you pick up could be jammed, forcing you to look for another quickly or throw them at your enemies.
   *
   * Parts of the map with supplies could randomly become inaccessable on a given run
   *
   * The core idea is that the same strategy will not work every run, and you won' know
   * that ahead of time, so the meta level of the game is learning to adapt quickly when
   * what you tried doesn't work. Like in martial arts.
   *
   * It is never unpredictable to the point of being cheap, but enough to force you to think fast
   *
   * There is a chance that your weapon will fire itself when you throw it. This will deal reduced damage if it hits you.
   * How should we telegraph it though??
   *
   * Some rooms could randomly have enemies instead of supplies.
   *
   * Certain doors require keys.
   *
   * Some special rooms could be locked, and maybe the key randomly spawns in one of a handfull of rooms.
   * These rooms wouldn't be essential to completing a run, just extra stuff.
   *
   */
  UNIMPLEMENTED;
}

void game_update_and_draw(Game *gp) {

  if(IsMusicStreamPlaying(gp->music)) {
    if(GetMusicTimePlayed(gp->music) >= 160.58f) {
      SeekMusicStream(gp->music, 32.630f);
    }

    SetMusicVolume(gp->music, 0.10f);
    UpdateMusicStream(gp->music);
  }

#ifdef DEBUG
  gp->dt = Clamp(1.0f/50.0f, 1.0f/TARGET_FPS, GetFrameTime());
#else
  gp->dt = Clamp(1.0f/10.0f, 1.0f/TARGET_FPS, GetFrameTime());
#endif

  gp->next_state = gp->state;

  {
    Entity *player = entity_from_handle(gp->player_handle);
    if(player) {
      gp->player = player;
    }
  }

  if(WindowShouldClose()) {
    gp->quit = 1;
    return;
  }

  { /* get input */
    gp->input_flags = 0;

    if(IsKeyDown(KEY_W)) {
      gp->input_flags |= INPUT_FLAG_MOVE_FORWARD;
    }

    if(IsKeyDown(KEY_A)) {
      gp->input_flags |= INPUT_FLAG_MOVE_LEFT;
    }

    if(IsKeyDown(KEY_S)) {
      gp->input_flags |= INPUT_FLAG_MOVE_BACKWARD;
    }

    if(IsKeyDown(KEY_D)) {
      gp->input_flags |= INPUT_FLAG_MOVE_RIGHT;
    }

    if(IsKeyPressed(KEY_E)) {
      gp->input_flags |= INPUT_FLAG_INTERACT;
    }

    // TODO throwing weapons
    if(IsKeyPressed(KEY_F)) {
      gp->input_flags |= INPUT_FLAG_THROW;
    }

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      gp->input_flags |= INPUT_FLAG_SHOOT;
    }

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      gp->input_flags |= INPUT_FLAG_SHOOT_HOLD;
    }

    if(IsKeyPressed(KEY_ESCAPE)) {
      if(gp->flags & GAME_FLAG_CAN_PAUSE) {
        gp->input_flags |= INPUT_FLAG_PAUSE;
      }
    }

#ifdef DEBUG

    if(IsKeyPressed(KEY_F1)) {
      if(gp->debug_flags & GAME_DEBUG_FLAG_MUTE) {
        SetMasterVolume(1);
      } else {
        SetMasterVolume(0);
      }
      gp->debug_flags ^= GAME_DEBUG_FLAG_MUTE;
    }

    if(IsKeyPressed(KEY_F5)) {
      game_reset(gp);
    }

    if(IsKeyPressed(KEY_F11)) {
      gp->debug_flags  ^= GAME_DEBUG_FLAG_DEBUG_UI;
    }

    if(IsKeyPressed(KEY_F10)) {
      gp->debug_flags ^= GAME_DEBUG_FLAG_DRAW_ALL_ENTITY_BOUNDS;
    }

    if(IsKeyPressed(KEY_F7)) {
      gp->debug_flags  ^= GAME_DEBUG_FLAG_PLAYER_INVINCIBLE;
    }

#endif

    int key = GetCharPressed();
    if(key != 0) {
      gp->input_flags |= INPUT_FLAG_ANY;
    }

  } /* get input */


  if(gp->input_flags & INPUT_FLAG_PAUSE) {
    gp->flags ^= GAME_FLAG_PAUSE;
  }

  { /* update */

    if(is_valid_handle(gp->player_handle)) {
      if(!gp->player) {
        gp->next_state = GAME_STATE_GAME_OVER;
      }
    }

    if(gp->flags & GAME_FLAG_PAUSE) {
      // TODO pause
      //if(gp->input_flags & INPUT_FLAG_ANY) {
      //  ResumeSound(gp->avenger_bullet_sound);
      //  ResumeSound(gp->crab_hurt_sound);
      //  ResumeSound(gp->health_pickup_sound);
      //  gp->flags ^= GAME_FLAG_PAUSE;
      //} else {
      //  PauseSound(gp->avenger_bullet_sound);
      //  PauseSound(gp->crab_hurt_sound);
      //  PauseSound(gp->health_pickup_sound);
      //  goto update_end;
      //}
    }

    if(gp->flags & GAME_FLAG_CAMERA_SHAKE) {
      if(!gp->cam_shake.on) {
        gp->cam_shake.on = 1;
      }

      if(gp->cam_shake.timer >= gp->cam_shake.duration) {
        gp->flags &= ~GAME_FLAG_CAMERA_SHAKE;
      } else {
        gp->cam_shake.timer += gp->dt;

        // Generate small random offset
        float progress = gp->cam_shake.timer / gp->cam_shake.duration;
        float intensity = gp->cam_shake.magnitude * (1.0f - progress);

        gp->cam_shake.offset.x = ((float)GetRandomValue(-100, 100) / 100.0f) * intensity;
        gp->cam_shake.offset.y = ((float)GetRandomValue(-100, 100) / 100.0f) * intensity;
      }

    }

    if(gp->flags & GAME_FLAG_CAMERA_PULSATE) {
      if(!gp->cam_pulsate.on) {
        gp->cam_pulsate.on = 1;
      }

      if(gp->cam_pulsate.timer >= gp->cam_pulsate.duration) {
        gp->flags &= ~GAME_FLAG_CAMERA_PULSATE;
        gp->cam.zoom = gp->cam_pulsate.save_zoom;
      } else {
        gp->cam_pulsate.timer += gp->dt;

        // Generate small random offset
        float progress = gp->cam_pulsate.timer / gp->cam_pulsate.duration;
        gp->cam_pulsate.zoom_offset = gp->cam_pulsate.magnitude * (1.0f - progress);
      }

    }


    switch(gp->state) {
      default:
        UNREACHABLE;
      case GAME_STATE_NONE:
        {
          /* start settings */

          SetMusicVolume(gp->music, 1.0);

          {
#ifdef DEBUG

            gp->debug_flags |=
              GAME_DEBUG_FLAG_DEBUG_UI |
              GAME_DEBUG_FLAG_MUTE |
              GAME_DEBUG_FLAG_SKIP_TRANSITIONS |
              //GAME_DEBUG_FLAG_PLAYER_INVINCIBLE |
              0;

            gp->next_state = GAME_STATE_DEBUG_SANDBOX;
            //gp->next_state = GAME_STATE_GAME_OVER;
            //gp->next_state = GAME_STATE_SPAWN_PLAYER;
            //gp->next_state = GAME_STATE_TITLE_SCREEN;
#else

            gp->level = 0;
            gp->phase_index = 0;

            if(!has_started_before) {
              gp->next_state = GAME_STATE_TITLE_SCREEN;
              has_started_before = true;
            } else {
              gp->next_state = GAME_STATE_SPAWN_PLAYER;
            }
#endif

            { /* init globals */

              cleared_screen_on_victory = false;

              at_boss_level = false;

              title_screen_key_pressed = false;
              title_screen_chars_deleted = 0;
              title_screen_type_char_timer = 0;

              intro_screen_fade_timer = 0;
              intro_screen_colonel_delay = 1.0f;
              intro_screen_cur_message = 0;
              intro_screen_chars_typed = 0;
              intro_screen_type_char_timer = 0;
              intro_screen_pre_message_delay = 0.8f;

              victory_screen_pre_delay = 1.0f;
              victory_screen_pre_counter_delay = 0.5f;
              victory_screen_finished_typing_banner = false;
              victory_screen_finished_incrementing_score = false;
              victory_screen_chars_typed = 0;
              victory_screen_type_dir = 1;
              victory_screen_show_timer = 2.5f;
              victory_screen_target_len = STRLEN("VICTORY");
              victory_screen_type_char_timer = 0;
              victory_screen_inc_score_timer = 0;
              victory_screen_score_delay = 1.4f;
              victory_screen_score_counter = 0;
              victory_screen_restart_hint_on = true;
              victory_screen_hint_timer = 0;

              boss_start_shooting_delay = 0;
              boss_shoot_pause = 0;

            } /* init globals */

            //memory_set(&gp->phase, 0, sizeof(gp->phase));

          }

          goto update_end;

        } break;
      case GAME_STATE_TITLE_SCREEN:
        {

          TODO("title screen");

        } break;
      case GAME_STATE_INTRO_SCREEN:
        {

          TODO("intro screen");

        } break;
      case GAME_STATE_MAIN_LOOP:

        TODO("main loop");

        game_main_loop(gp);
        if(gp->next_state == GAME_STATE_VICTORY) {
          goto update_end;
        }
        break;
      case GAME_STATE_VICTORY:
        {

          TODO("victory screen");

        } break;
      case GAME_STATE_GAME_OVER:
        {

          TODO("game over screen");

        } break;
#ifdef DEBUG
      case GAME_STATE_DEBUG_SANDBOX:
        {

          if(!gp->player) {
            Entity *player = spawn_player(gp);
            gp->player_handle = handle_from_entity(player);

            gp->flags |=
              GAME_FLAG_DRAW_IN_CAMERA |
              0;

            spawn_shotgun(gp);
            spawn_assault_rifle(gp);

          } else {

            gp->next_state = GAME_STATE_EDITOR;

          }

        } break;
      case GAME_STATE_EDITOR:
        {

          gp->debug_flags |=
            GAME_DEBUG_FLAG_FREE_CAM |
            0;


          if(IsKeyDown(KEY_LEFT_SHIFT)) {
            float offset_x = 45*GetMouseWheelMoveV().y;
            gp->cam.target.x += offset_x;
          } else if(IsKeyDown(KEY_LEFT_CONTROL)) {
            float offset_y = 45*GetMouseWheelMoveV().y;
            gp->cam.target.y += offset_y;
          } else {
            float adjust_zoom = 0.4f*GetMouseWheelMoveV().y;

            if(gp->cam.zoom + adjust_zoom > 0.0) {
              gp->cam.zoom += adjust_zoom;
            }
          }

          if(IsKeyPressed(KEY_EQUAL)) {
            gp->cam.zoom = INITIAL_CAMERA_ZOOM;
          }

          if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 p = GetMousePositionWorld2D(gp->cam);
            s64 tile = tile_from_point(p);
            ASSERT(tile >= 0 && tile <= TILES_COUNT);
            gp->editor.tiles[tile] = 1;
          } else if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 p = GetMousePositionWorld2D(gp->cam);
            s64 tile = tile_from_point(p);
            ASSERT(tile >= 0 && tile <= TILES_COUNT);
            gp->editor.tiles[tile] = 0;
          } else if(IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
            gp->cam.target =
              Vector2Subtract(gp->cam.target,
                  Vector2Scale(GetMouseDelta(), 0.4f));
          }

          goto update_end;

        } break;
#endif
    }

    gp->live_entities = 0;
    gp->live_enemies = 0;

    for(Entity_order order = ENTITY_ORDER_FIRST; order < ENTITY_ORDER_MAX; order++) {

      for(int i = 0; i < gp->entities_allocated; i++)
      { /* update_entities */

        Entity *ep = &gp->entities[i];

        if(ep->live && ep->update_order == order)
        { /* entity_update */

          gp->live_entities++;

          if(entity_kind_in_mask(ep->kind, ENEMY_KIND_MASK)) {
            gp->live_enemies++;
          }

          b8 applied_collision = 0;
          b8 is_on_screen = CheckCollisionCircleRec(ep->pos, ep->radius, WINDOW_RECT);
          b8 is_fully_on_screen = check_circle_all_inside_rec(ep->pos, ep->radius, WINDOW_RECT);

          switch(ep->control) {
            default:
              UNREACHABLE;
            case ENTITY_CONTROL_NONE:
              break;
            case ENTITY_CONTROL_PLAYER:
              {

                { /* mouse look */

                  Vector2 center = { WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f };
                  Vector2 mouse_pos = GetMousePosition();
                  Vector2 look_dir = Vector2Subtract(mouse_pos, center);
                  float len = Vector2Length(look_dir);

                  if(len > 0.001f) {
                    look_dir = Vector2Scale(look_dir, 1.0f/len);

                    ep->look_dir = look_dir;
                    ep->look_angle = -atan2f(look_dir.x, look_dir.y);
                    ep->sprite_rotation = ep->look_angle * RAD2DEG;
                  }

                } /* mouse look */

                ep->accel = (Vector2){0};

                if(gp->input_flags & INPUT_FLAG_MOVE) {

                  if(gp->input_flags & INPUT_FLAG_MOVE_LEFT) {
                    ep->accel.x = -1;
                  }

                  if(gp->input_flags & INPUT_FLAG_MOVE_RIGHT) {
                    ep->accel.x += 1;
                  }

                  if(gp->input_flags & INPUT_FLAG_MOVE_FORWARD) {
                    ep->accel.y = -1;
                  }

                  if(gp->input_flags & INPUT_FLAG_MOVE_BACKWARD) {
                    ep->accel.y += 1;
                  }

                  ep->accel = Vector2Normalize(ep->accel);
                  ep->accel = Vector2Scale(ep->accel, PLAYER_ACCEL);

                } else {
                  ep->vel = (Vector2){0};
                }

                if(ep->received_damage > 0) {
                  // TODO camera shake
                  ep->effect_tint = BLOOD;

                  if(ep->invulnerability_timer > 0) {
                    ep->health += ep->received_damage;
                  } else {
                    ep->invulnerability_timer = 1.2f;
                  }

                }

                if(ep->invulnerability_timer != 0) {
                  if(ep->invulnerability_timer > 0) {
                    ep->invulnerability_timer -= gp->dt;

                    if(ep->effect_tint_timer <= 0) {
                      ep->flags |= ENTITY_FLAG_APPLY_EFFECT_TINT;

                      ep->effect_tint_duration = 0.1f;
                      ep->effect_tint_timer_vel = 1.0f;

                      ep->effect_tint_timer = ep->effect_tint_duration;
                    }

                  } else {
                    ep->invulnerability_timer = 0;
                    ep->effect_tint = BLANK;
                  }
                }

                Entity *gun = entity_from_handle(ep->holding_gun_handle);
                if(gun) {

                  if(gun->gun.flags & GUN_FLAG_AUTOMATIC) {
                    if(gp->input_flags & INPUT_FLAG_SHOOT_HOLD) {
                      gun->gun.shoot = 1;
                    }
                  } else {
                    if(gp->input_flags & INPUT_FLAG_SHOOT) {

                      gun->gun.shoot = 1;

                      //camera_shake(gp, 0.15, 1.8f);
                      //camera_shake(gp, 0.1f, 1.0f);
                      //camera_pulsate(gp, 0.18f, 0.16f);

                      //if(!(gp->flags & GAME_FLAG_PLAYER_CANNOT_SHOOT)) {
                      //  Entity *gun = entity_from_handle(ep->holding_gun_handle);
                      //  if(gun) {
                      //    gun->gun.shoot = 1;
                      //  }
                      //}
                    }
                  } 

                }

                if(gp->input_flags & INPUT_FLAG_INTERACT) {
                  ep->flags |= ENTITY_FLAG_INTERACT;
                }

                if(gp->input_flags & INPUT_FLAG_THROW) {

                  Entity *holding = entity_from_handle(ep->holding_gun_handle);

                  if(holding) {
                    holding->flags ^=
                      ENTITY_FLAG_DYNAMICS |
                      ENTITY_FLAG_SPINNING |
                      ENTITY_FLAG_APPLY_FRICTION |
                      ENTITY_FLAG_APPLY_COLLISION_DAMAGE |
                      ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
                      ENTITY_FLAG_HAS_LIFETIME |
                      ENTITY_FLAG_IS_INTERACTABLE |
                      0;

                    holding->control = ENTITY_CONTROL_NONE;
                    holding->parent_handle = (Entity_handle){0};
                    holding->vel = Vector2Scale(ep->look_dir, 1100);
                    holding->spin_vel = PI*6.5f;
                    holding->friction = 1.0f;
                    holding->damage_amount = holding->gun.bullet_damage << 2;
                    holding->life_time_duration = 0.23f;
                    holding->apply_collision_mask =
                      ENTITY_KIND_MASK_RAPTOR |
                      0;
                  }

                }

                if(gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) {
                  ep->health = PLAYER_HEALTH;
                  ep->received_damage = 0;
                }

              } break;
            case ENTITY_CONTROL_GUN_BEING_HELD:
              {
                Entity *parent = entity_from_handle(ep->parent_handle);

                ep->look_dir = parent->look_dir;
                ep->look_angle = parent->look_angle;

                float angle = parent->look_angle;
                ep->sprite_rotation = parent->sprite_rotation;
                ep->manual_sprite_origin = parent->pos;
                Vector2 offset = Vector2Rotate(ep->being_held_offset, angle);
                ep->pos = Vector2Add(parent->pos, offset);

              } break;
            case ENTITY_CONTROL_COPY_PARENT:
              {
                Entity *parent = entity_from_handle(ep->parent_handle);
                ASSERT(parent);

                ep->vel = parent->vel;

              } break;
            case ENTITY_CONTROL_FOLLOW_PARENT:
              {
                Entity *parent = entity_from_handle(ep->parent_handle);
                ASSERT(parent);

                Vector2 dir = Vector2Normalize(Vector2Subtract(parent->pos, ep->pos));
                ep->vel = Vector2Scale(dir, ep->scalar_vel);

              } break;
            case ENTITY_CONTROL_GOTO_WAYPOINT:
              {
                ASSERT(ep->waypoints.first && ep->waypoints.last);
                Waypoint *wp = ep->cur_waypoint;
                if(!wp) {
                  wp = ep->waypoints.first;
                }

                Vector2 dir = Vector2Subtract(wp->pos, ep->pos);
                float dir_len_sqr = Vector2LengthSqr(dir);

                if(dir_len_sqr < SQUARE(wp->radius)) {
                  if(!wp->next) {
                    if(ep->waypoints.action) {
                      ep->waypoints.action(gp, ep);
                    }
                  } else {
                    ep->cur_waypoint = wp->next;
                  }
                } else {
                  float dir_len = sqrtf(dir_len_sqr);
                  dir = Vector2Scale(dir, ep->scalar_vel/dir_len);
                  ep->vel = dir;
                }

              } break;
          }

          /* entity flags stuff */

          if(ep->flags & ENTITY_FLAG_APPLY_FRICTION) {
            ep->vel = Vector2Subtract(ep->vel, Vector2Scale(ep->vel, ep->friction*gp->dt));
          }

          if(ep->flags & ENTITY_FLAG_DYNAMICS) {
            Vector2 a_times_t = Vector2Scale(ep->accel, gp->dt);
            ep->vel = Vector2Add(ep->vel, a_times_t);
            ep->pos = Vector2Add(ep->pos, Vector2Add(Vector2Scale(ep->vel, gp->dt), Vector2Scale(a_times_t, 0.5*gp->dt)));
          }

          if(ep->flags & ENTITY_FLAG_SPINNING) {
            float turn = ep->spin_vel * gp->dt;
            ep->look_angle += turn;
            //ep->look_dir = Vector2Rotate(ep->look_dir, turn);
          }

          if(ep->flags & ENTITY_FLAG_HAS_GUN) {
            if(gp->state != GAME_STATE_GAME_OVER) {
              entity_shoot_gun(gp, ep);
            }
          }

          if(ep->flags & ENTITY_FLAG_INTERACT) {

            ep->flags &= ~ENTITY_FLAG_INTERACT;

            for(int i = 0; i < gp->entities_allocated; i++) {
              Entity *interacting = &gp->entities[i];

              if(ep != interacting && interacting->live) {

                if(interacting->flags & ENTITY_FLAG_IS_INTERACTABLE) {
                  bool is_interacting =
                    CheckCollisionPointCircle(ep->pos, interacting->pos, interacting->interact_radius);
                  if(is_interacting) {

                    interacting->interact_proc(gp, interacting, ep);
                    break;

                  }
                }

              }

            }

          }

          if(ep->flags & ENTITY_FLAG_APPLY_COLLISION) {

            for(int i = 0; i < gp->entities_allocated; i++) {
              Entity *colliding = &gp->entities[i];

              if(ep != colliding && colliding->live) {

                if(entity_kind_in_mask(colliding->kind, ep->apply_collision_mask)) {
                  if(entity_check_collision(gp, ep, colliding)) {
                    applied_collision = 1;
                    colliding->received_collision = 1;

                    if(ep->collide_proc) {
                      ep->collide_proc(gp, ep, colliding);
                    }

                    if(ep->flags & ENTITY_FLAG_APPLY_COLLISION_DAMAGE) {
                      colliding->received_damage += ep->damage_amount;
                    }

                  }

                }

              }

            }

          }

          if(ep->flags & ENTITY_FLAG_DIE_IF_CHILD_LIST_EMPTY) {
            ASSERT(ep->child_list);
            if(ep->child_list->count <= 0) {
              ep->flags |= ENTITY_FLAG_DIE_NOW;
            }
          }

          if(ep->flags & ENTITY_FLAG_DIE_ON_APPLY_COLLISION) {
            if(applied_collision) {
              ep->flags |= ENTITY_FLAG_DIE_NOW;
            }
          }

          if(ep->flags & ENTITY_FLAG_APPLY_EFFECT_TINT) {
            if(ep->effect_tint_timer < 0) {
              ep->effect_tint_timer = 0;
              ep->flags &= ~ENTITY_FLAG_APPLY_EFFECT_TINT;
            } else {
              ep->effect_tint_timer -= ep->effect_tint_timer_vel*gp->dt;
            }
          }

          if(ep->flags & ENTITY_FLAG_RECEIVE_COLLISION) {
            if(!is_fully_on_screen) {
              ep->received_collision = 0;
              ep->received_damage = 0;
            } else {
              if(ep->received_collision) {
                ep->received_collision = 0;

                if(ep->flags & ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE) {

                  if(ep->received_damage > 0) {
                    if(IsSoundValid(ep->hurt_sound)) {
                      SetSoundPan(ep->hurt_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
                      if(ep->kind == ENTITY_KIND_PLAYER) {
                        SetSoundVolume(ep->hurt_sound, 0.17f);
                      } else {
                        SetSoundVolume(ep->hurt_sound, 0.5f);
                      }
                      PlaySound(ep->hurt_sound);
                    }
                  }

                  ep->health -= ep->received_damage;

                  if(ep->flags & ENTITY_FLAG_DAMAGE_INCREMENTS_SCORE) {
                    gp->score += ep->received_damage;
                  }

                  ep->received_damage = 0;

                  if(ep->kind != ENTITY_KIND_PLAYER) {
                    ep->flags |= ENTITY_FLAG_APPLY_EFFECT_TINT;

                    ep->effect_tint = BLOOD;

                    if(ep->effect_tint_duration == 0) {
                      ep->effect_tint_duration = 0.02f;
                    }
                    if(ep->effect_tint_timer_vel == 0) {
                      ep->effect_tint_timer_vel = 1.0f;
                    }

                    ep->effect_tint_timer = ep->effect_tint_duration;
                  }

                  if(ep->health <= 0) {
                    ep->flags |= ENTITY_FLAG_DIE_NOW;
                  }

                }

              }
            }
          }

          if(ep->flags & ENTITY_FLAG_HAS_SPRITE) {
            sprite_update(gp, ep);
          }

          if(ep->flags & ENTITY_FLAG_NOT_ON_SCREEN) {
            if(is_on_screen) {
              ep->flags ^= ENTITY_FLAG_NOT_ON_SCREEN | ENTITY_FLAG_ON_SCREEN;
            }
          }

          if(ep->flags & ENTITY_FLAG_HAS_LIFETIME) {
            ASSERT(ep->life_time_duration > 0);

            if(ep->life_timer >= ep->life_time_duration) {
              ep->life_timer = 0;
              ep->flags |= ENTITY_FLAG_DIE_NOW;
            } else {
              ep->life_timer += gp->dt;
            }

          }

          if(ep->flags & ENTITY_FLAG_EMIT_SPAWN_PARTICLES) {
            ep->flags &= ~ENTITY_FLAG_EMIT_SPAWN_PARTICLES;

            if(is_on_screen) {

              //if(IsSoundValid(ep->spawn_sound)) {
              //  PlaySound(ep->spawn_sound);
              //}

              Particle_emitter tmp = ep->particle_emitter;
              ep->particle_emitter = ep->spawn_particle_emitter;
              entity_emit_particles(gp, ep);
              ep->particle_emitter = tmp;
            }

          }

          if(ep->flags & ENTITY_FLAG_DIE_NOW) {

            if(is_on_screen) {
              if(ep->flags & ENTITY_FLAG_EMIT_DEATH_PARTICLES) {
                Particle_emitter tmp = ep->particle_emitter;
                ep->particle_emitter = ep->death_particle_emitter;
                entity_emit_particles(gp, ep);
                ep->particle_emitter = tmp;
              }

            }

            entity_die(gp, ep);
            goto entity_update_end;
          }

entity_update_end:;
        } /* entity_update */

      } /* update_entities */

    }

    gp->live_particles = 0;

    for(int i = 0; i < MAX_PARTICLES; i++) {
      Particle *p = &gp->particles[i];

      if(p->live >= p->lifetime) {
        p->live = 0;
        p->lifetime = 0;
        continue;
      }

      gp->live_particles++;

      { /* particle_update */

        if(!CheckCollisionCircleRec(p->pos, p->radius, WINDOW_RECT)) {
          p->live = 0;
          p->lifetime = 0;
          gp->live_particles--;
          continue;
        }

        p->pos = Vector2Add(p->pos, Vector2Scale(p->vel, gp->dt));
        p->vel = Vector2Subtract(p->vel, Vector2Scale(p->vel, p->friction*gp->dt));

        if(p->radius > 0) {
          p->radius -= p->shrink * gp->dt;
        }

        p->live += gp->dt;

      } /* particle_update */

    }

update_end:;
  } /* update */

  defer_loop(BeginDrawing(), EndDrawing())
  { /* draw to screen */
    ClearBackground(BLACK);

    gp->cam.offset =
      (Vector2) {
        (float)GetScreenWidth()*0.5f,
        (float)GetScreenHeight()*0.5f,
      };

    if(!(gp->debug_flags & GAME_DEBUG_FLAG_FREE_CAM)) {

      if(gp->player) {
        gp->cam.target = gp->player->pos;

        if(gp->flags & GAME_FLAG_CAMERA_SHAKE) {
          gp->cam.target = Vector2Add(gp->cam.target, gp->cam_shake.offset);
        }

        if(gp->flags & GAME_FLAG_CAMERA_PULSATE) {
          gp->cam.zoom = gp->cam_pulsate.save_zoom - gp->cam_pulsate.zoom_offset; 
        }

      }

    }

    if(gp->flags & GAME_FLAG_DRAW_IN_CAMERA) defer_loop(BeginMode2D(gp->cam), EndMode2D())
    { /* draw in camera */

      {
        Rectangle rec =
        {
          .width = gp->debug_background.width,
          .height = gp->debug_background.height,
        };

        Vector2 pos = 
        {
          .x = 200, .y = 300,
        };

        DrawTextureRec(gp->debug_background, rec, pos, WHITE);
      }

      if(gp->state == GAME_STATE_EDITOR) {

        Editor *editor = &gp->editor;

        for(s64 i = 0; i < TILES_COUNT; i++) {
          if(editor->tiles[i]) {
            Vector2 tile_point = point_from_tile(i);

            {
              Rectangle rec =
              {
                .x = tile_point.x,
                .y = tile_point.y,
                .width = TILE_SIZE,
                .height = TILE_SIZE,
              };

              DrawRectangleRec(rec, SKYBLUE);
            }

          }
        }

        Vector2 p = GetMousePositionWorld2D(gp->cam);

        Vector2 tile_point = point_from_tile(tile_from_point(p));

        {
          Rectangle rec =
          {
            .x = tile_point.x,
            .y = tile_point.y,
            .width = TILE_SIZE,
            .height = TILE_SIZE,
          };

          DrawRectangleRec(rec, ColorAlpha(ORANGE, 0.7));
        }

        DrawGrid2D(1000, TILE_SIZE);

      }

      for(Entity_order order = ENTITY_ORDER_FIRST; order < ENTITY_ORDER_MAX; order++) {
        for(int i = 0; i < gp->entities_allocated; i++)
        { /* entity_draw */

          Entity *ep = &gp->entities[i];

          if(!ep->live || ep->draw_order != order) continue;

          if(ep->flags & ENTITY_FLAG_FILL_BOUNDS) {
            Color tint = ep->fill_color;
            DrawCircleV(ep->pos, ep->radius, tint);
          }

          if(ep->flags & ENTITY_FLAG_HAS_SPRITE) {
            Vector2 pos = ep->pos;

            // TODO sprite offset
            draw_sprite(gp, ep);

            ep->pos = pos;
          }

          if(gp->debug_flags & GAME_DEBUG_FLAG_DRAW_ALL_ENTITY_BOUNDS) {
            Color bounds_color = ep->bounds_color;
            bounds_color.a = 150;
            DrawCircleLinesV(ep->pos, ep->radius, ep->bounds_color);

            if(ep->interact_radius > 0) {
              DrawCircleLinesV(ep->pos, ep->interact_radius, YELLOW);
            }

            DrawCircleV(ep->pos, 4.0f, bounds_color);

            float look_len = Vector2Length(ep->look_dir);
            if(look_len > 0.0001f) {
              Vector2 look = Vector2Scale(ep->look_dir, (ep->radius+5.0f)/look_len);
              DrawLineEx(ep->pos, Vector2Add(ep->pos, look), 1.0f, ep->bounds_color);
            }
          }

        } /* entity_draw */
      }

      for(int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &gp->particles[i];

        if(p->live >= p->lifetime) continue;

        { /* particle_draw */

          Rectangle rec = { p->pos.x - p->radius, p->pos.y - p->radius, TIMES2(p->radius), TIMES2(p->radius) };

          Color tint = ColorLerp(p->begin_tint, p->end_tint, Normalize(p->live, 0, p->lifetime));

          DrawRectangleRec(rec, tint);

        } /* particle_draw */

      }

    } /* draw in camera */

    if(gp->flags & GAME_FLAG_HUD) {
      // TODO HUD
    }

    if(gp->state == GAME_STATE_START_LEVEL) {
    } else if(gp->state == GAME_STATE_GAME_OVER) {
    } else if(gp->state == GAME_STATE_TITLE_SCREEN) {
    } else if(gp->state == GAME_STATE_INTRO_SCREEN) {
    } else if(gp->state == GAME_STATE_VICTORY) {
    }

    if(gp->flags & GAME_FLAG_PAUSE) {
      DrawRectangleRec(WINDOW_RECT, (Color){ .a = 100 });

      char *paused_msg = "PAUSED";
      char *hint_msg = "press any key to resume";
      int pause_font_size = 90;
      int hint_font_size = 20;

      Color pause_color = MUSTARD;
      Color hint_color = LIGHTGRAY;

      static b8 hint_on = 1;
      static f32 hint_blink_time = 0.0f;

      Vector2 pause_pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.4f };
      Vector2 pause_size = MeasureTextEx(gp->font, paused_msg, pause_font_size, pause_font_size/10);

      Vector2 hint_pos = { WINDOW_WIDTH * 0.5, WINDOW_HEIGHT * 0.5f };
      Vector2 hint_size = MeasureTextEx(gp->font, hint_msg, hint_font_size, hint_font_size/10);

      DrawTextEx(
          gp->font,
          paused_msg,
          Vector2Subtract(pause_pos, Vector2Scale(pause_size, 0.5)),
          pause_font_size,
          pause_font_size/10,
          pause_color);

      if(hint_on) {
        DrawTextEx(
            gp->font,
            hint_msg,
            Vector2Subtract(hint_pos, Vector2Scale(hint_size, 0.5)),
            hint_font_size,
            hint_font_size/10,
            hint_color);
      }

      if((hint_on && hint_blink_time >= 0.75f) || (!hint_on && hint_blink_time >= 0.5f)) {
        hint_on = !hint_on;
        hint_blink_time = 0;
      } else {
        hint_blink_time += gp->dt;
      }

    }


#ifdef DEBUG
    if(gp->debug_flags & GAME_DEBUG_FLAG_DEBUG_UI) { /* debug overlay */
      char *debug_text = frame_push_array(char, 256);
      char *debug_text_fmt =
        "sound: %s\n"
        "player_is_invincible: %s\n"
        "frame time: %.7f\n"
        "live entities count: %i\n"
        "live enemies count: %i\n"
        "live particles count: %i\n"
        "most entities allocated: %li\n"
        "particle_pos: %i\n"
        "screen width: %i\n"
        "screen height: %i\n"
        "render width: %i\n"
        "render height: %i\n"
        "player pos: { x = %.1f, y = %.1f }\n"
        "level: %i\n"
        "phase: %i\n"
        "game state: %s";
      stbsp_sprintf(debug_text,
          debug_text_fmt,
          (gp->debug_flags & GAME_DEBUG_FLAG_MUTE) ? "off" : "on",
          (gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) ? "on" : "off",
          gp->dt,
          gp->live_entities,
          gp->live_enemies,
          gp->live_particles,
          gp->entities_allocated,
          gp->particles_pos,
          GetScreenWidth(),
          GetScreenHeight(),
          GetRenderWidth(),
          GetRenderHeight(),
          gp->player ? gp->player->pos.x : 0,
          gp->player ? gp->player->pos.y : 0,
          gp->level+1,
          gp->phase_index+1,
          Game_state_strings[gp->state]);
      Vector2 debug_text_size = MeasureTextEx(gp->font, debug_text, 18, 1.0);
      DrawText(debug_text, 10, GetScreenHeight() - debug_text_size.y - 10, 18, GREEN);
    } /* debug overlay */
#endif

  } /* draw to screen */


  gp->state = gp->next_state;

  gp->frame_index++;

  arena_clear(gp->frame_arena);

}
