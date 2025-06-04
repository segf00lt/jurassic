#include "raylib.h"
#include "basic.h"
#include <dlfcn.h>


typedef void* (*Module_proc)(void *);


int main(void) {

  //context_init();

  void *module = dlopen(GAME_MODULE_PATH, RTLD_NOW);
  s64 module_modtime = GetFileModTime(GAME_MODULE_PATH);

  if(module) {
    TraceLog(LOG_INFO, "successfully loaded module code");
  } else {
    TraceLog(LOG_ERROR, "failed to load module code: %s", dlerror());
    return 1;
  }

  Module_proc module_init_proc            = (Module_proc)dlsym(module, "module_init");
  Module_proc module_close_proc           = (Module_proc)dlsym(module, "module_close");
  Module_proc module_main_proc            = (Module_proc)dlsym(module, "module_main");
  Module_proc module_unload_assets_proc   = (Module_proc)dlsym(module, "module_unload_assets");
  Module_proc module_load_assets_proc     = (Module_proc)dlsym(module, "module_load_assets");

  void *state = module_init_proc(0);

  while(module_main_proc(state)) {

    s64 modtime = GetFileModTime(GAME_MODULE_PATH);
    if(module_modtime != modtime) {
      TraceLog(LOG_INFO, "reloading module code");
      module_modtime = modtime;

      WaitTime(0.17f);
      if(dlclose(module)) {
        TraceLog(LOG_ERROR, "failed to load module code: %s", dlerror());
        return 1;
      }

      module = dlopen(GAME_MODULE_PATH, RTLD_NOW);

      module_init_proc  = (Module_proc)dlsym(module, "module_init");
      module_close_proc = (Module_proc)dlsym(module, "module_close");
      module_main_proc  = (Module_proc)dlsym(module, "module_main");

      module_unload_assets_proc = (Module_proc)dlsym(module, "module_unload_assets");
      module_load_assets_proc   = (Module_proc)dlsym(module, "module_load_assets");

      module_unload_assets_proc(state);
      module_load_assets_proc(state);
    }

  }

  module_close_proc(state);

  return 0;
}
