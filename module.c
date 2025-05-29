#include "jurassic.c"

void *module_init(void*);
void *module_close(void*);
void *module_main(void*);
void *module_load_assets(void*);
void *module_unload_assets(void*);

void *module_init(void* _) {

  return (void*)game_init();

}

void *module_close(void* gp) {

  game_close(gp);
  return 0;

}

void *module_main(void* _gp) {

  Game *gp = _gp;
  game_update_and_draw(gp);

  if(gp->quit) {
    return (void*)0;
  } else {
    return (void*)1;
  }

}

void *module_load_assets(void* gp) {

  game_load_assets((Game*)gp);
  return 0;

}

void *module_unload_assets(void* gp) {

  game_unload_assets((Game*)gp);
  return 0;

}
