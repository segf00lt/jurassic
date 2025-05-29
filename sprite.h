#ifndef SPRITE_H
#define SPRITE_H


#include "basic.h"


#define SPRITE_FLAGS              \
  X(STILL)                        \
  X(REVERSE)                      \
  X(PINGPONG)                     \
  X(INFINITE_REPEAT)              \
  X(DRAW_MIRRORED_X)              \
  X(DRAW_MIRRORED_Y)              \
  X(AT_LAST_FRAME)                \


typedef enum Sprite_flag_index {
  SPRITE_FLAG_INDEX_INVALID = -1,
#define X(flag) SPRITE_FLAG_INDEX_##flag,
  SPRITE_FLAGS
#undef X
    SPRITE_FLAG_INDEX_MAX,
} Sprite_flag_index;

STATIC_ASSERT(SPRITE_FLAG_INDEX_MAX < 32, number_of_sprite_flags_is_less_than_32);

typedef u32 Sprite_flags;
#define X(flag) const Sprite_flags SPRITE_FLAG_##flag = (Sprite_flags)(1u << SPRITE_FLAG_INDEX_##flag);
SPRITE_FLAGS
#undef X

typedef struct Sprite_frame Sprite_frame;
struct Sprite_frame {
  u16 x;
  u16 y;
  u16 w;
  u16 h;
};

typedef struct Sprite_frame_slice Sprite_frame_slice;
struct Sprite_frame_slice {
  Sprite_frame *d;
  s64 count;
};

typedef struct Sprite Sprite;
struct Sprite {
  u32 id;

  Sprite_flags flags;

  union {
    s32 frame;
    s32 first_frame;
  };

  s32 last_frame;
  s32 fps;
  s32 total_frames;

  s32 cur_frame; /* this is relative to the first_frame or last_frame depending on the animation direction */
  s32 frame_counter;
  s32 repeats; /* number of times to play the sprite's animation */
};


#endif
