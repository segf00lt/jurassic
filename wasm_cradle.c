#include <emscripten/emscripten.h>

#include "jurassic.c"

void wasm_main_loop(void *gp) {
  game_update_and_draw(gp);
}

int main(void) {
  Game *gp = game_init();

  if(!IsAudioDeviceReady()) {
    TraceLog(LOG_WARNING, "audio not initialized");
  }

  emscripten_set_main_loop_arg(wasm_main_loop, (void*)gp, TARGET_FPS, 1);

  game_close(gp);

  return 0;
}
