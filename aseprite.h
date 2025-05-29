#ifndef ASEPRITE_H
#define ASEPRITE_H


#include "basic.h"
#include "arena.h"
#include "str.h"


#define ASEPRITE_ANIM_DIRS                  \
  X(FORWARD,          forward)              \
  X(REVERSE,          reverse)              \
  X(PINGPONG,         pingpong)             \
  X(PINGPONG_REVERSE, pingpong_reverse)     \


typedef enum Aseprite_anim_dir {
  ASEPRITE_ANIM_DIR_NONE = 0,
#define X(d, ...) ASEPRITE_ANIM_DIR_##d,
  ASEPRITE_ANIM_DIRS
#undef X
    ASEPRITE_ANIM_DIR_MAX,
} Aseprite_anim_dir;

Str8 Aseprite_anim_dir_lower_strings[ASEPRITE_ANIM_DIR_MAX] = {
#define X(d, s) [ASEPRITE_ANIM_DIR_##d] = str8_lit(#s),
  ASEPRITE_ANIM_DIRS
#undef X
};

typedef struct Aseprite_atlas_frame Aseprite_atlas_frame;
struct Aseprite_atlas_frame {
  Str8      file_title;
  s64       frame_index;
  Rectangle frame;
  Rectangle sprite_source_size;
  Vector2   source_size;
  s32       duration;
  b32       rotated;
  b32       trimmed;
};

typedef struct Aseprite_frame_tag Aseprite_frame_tag;
struct Aseprite_frame_tag {
  Str8 file_title;
  Str8 tag_name;
  b32  is_keyframe;
  s64  n_repeats;
  s64  from;
  s64  to;

  Aseprite_anim_dir direction;

  Color color;
};

typedef struct Aseprite_atlas_meta Aseprite_atlas_meta;
struct Aseprite_atlas_meta {
  Str8 app;
  Str8 version;
  Str8 image;
  Str8 format;
  Str8 scale;
  Vector2 size;

  Aseprite_frame_tag *frame_tags;
  s64                 frame_tags_count;
};

typedef struct Aseprite_atlas Aseprite_atlas;
struct Aseprite_atlas {
  Aseprite_atlas_frame *frames;
  s64 *frame_indexes_relative_to_original_file;
  s64 frames_count;
  Aseprite_atlas_meta meta;
};


Rectangle aseprite_rectangle_from_json_object(JSON_value *v);
Vector2 aseprite_vector2_from_json_object(JSON_value *v);
Vector2 aseprite_vector2_wh_from_json_object(JSON_value *v);


#ifdef _UNITY_BUILD_
#define ASEPRITE_ATLAS_IMPL
#endif

#ifdef ASEPRITE_ATLAS_IMPL


Rectangle aseprite_rectangle_from_json_object(JSON_value *v) {
  Rectangle result = {0};

  JSON_value *field = v->value;

  for(; field; field = field->next) {

    if(str8_match_lit("x", field->name)) {
      result.x = (f32)field->floating;
    } else if(str8_match_lit("y", field->name)) {
      result.y = (f32)field->floating;
    } else if(str8_match_lit("w", field->name)) {
      result.width = (f32)field->floating;
    } else if(str8_match_lit("h", field->name)) {
      result.height = (f32)field->floating;
    }

  }

  return result;
}

Vector2 aseprite_vector2_wh_from_json_object(JSON_value *v) {
  Vector2 result = {0};
  JSON_value *field = v->value;

  for(; field; field = field->next) {

    if(str8_match_lit("w", field->name)) {
      result.x = (f32)field->floating;
    } else if(str8_match_lit("h", field->name)) {
      result.y = (f32)field->floating;
    }

  }

  return result;

}

Vector2 aseprite_vector2_from_json_object(JSON_value *v) {
  Vector2 result = {0};

  JSON_value *field = v->value;

  for(; field; field = field->next) {

    if(str8_match_lit("x", field->name)) {
      result.x = (f32)field->floating;
    } else if(str8_match_lit("y", field->name)) {
      result.y = (f32)field->floating;
    }

  }

  return result;

}


#endif

#endif
