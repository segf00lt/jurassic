#define _UNITY_BUILD_

#define NOB_IMPLEMENTATION

#include "nob.h"
#include "basic.h"
#include "arena.h"
#include "context.h"
#include "str.h"
#include "array.h"
#include "os.h"


#ifdef OS_WINDOWS

#error "windows support not implemented"

#elif defined(OS_MAC)

char vim_project_file[] =
"let project_root = getcwd()\n"
"let project_build = project_root . '/nob'\n"
"let project_exe = '/jurassic'\n"
"let project_run = project_root . project_exe\n"
"let project_debug = 'open -a Visual\\ Studio\\ Code ' . project_root\n"
"\n" 
"let &makeprg = project_build\n"
"\n"
"nnoremap <F7> :call jobstart('open -a Terminal ' . project_root, { 'detach':v:true })<CR>\n"
"nnoremap <F8> :call chdir(project_root)<CR>\n"
"nnoremap <F9> :wa<CR>:make<CR>\n"
"nnoremap <F10> :call StartScratchJob(project_run)<CR>\n"
"nnoremap <F11> :call jobstart(project_debug, { 'detach':v:true })<CR>\n";

#elif defined(OS_LINUX)

char vim_project_file[] =
"let project_root = getcwd()\n"
"let project_build = project_root . '/nob'\n"
"let project_exe = '/jurassic'\n"
"let project_run = project_root . project_exe\n"
"let project_debug = 'gf2 ' . project_root . project_exe\n"
"\n"
"let &makeprg = project_build\n"
"\n"
"nnoremap <F7> :call jobstart('alacritty --working-directory ' . project_root, { 'detach':v:true })<CR>\n"
"nnoremap <F8> :call chdir(project_root)<CR>\n"
"nnoremap <F9> :wa<CR>:make<CR>\n"
"nnoremap <F10> :call StartScratchJob(project_run)<CR>\n"
"nnoremap <F11> :call jobstart(project_debug, { 'detach':v:true })<CR>\n"
"nnoremap <F12> :call jobstart('aseprite', { 'detach':v:true })<CR>\n";

#else
#error "unsupported operating system"
#endif

char vscode_launch[] =
"{\n"
"    // Use IntelliSense to learn about possible attributes.\n"
"    // Hover to view descriptions of existing attributes.\n"
"    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387\n"
"    \"version\": \"0.2.0\",\n"
"    \"configurations\": [\n"
"    {\n"
"        \"name\": \"(lldb) Launch\",\n"
"        \"type\": \"cppdbg\",\n"
"        \"request\": \"launch\",\n"
"        \"program\": \"${workspaceFolder}/jurassic\",\n"
"        \"args\": [],\n"
"        \"stopAtEntry\": false,\n"
"        \"cwd\": \"${workspaceFolder}\",\n"
"        \"environment\": [],\n"
"        \"externalConsole\": false,\n"
"        \"MIMode\": \"lldb\"\n"
"    }\n"
"]\n"
"}\n";

char vscode_settings[] =
"{\n"
"    \"cmake.ignoreCMakeListsMissing\": true,\n"
"    \"C_Cpp.errorSquiggles\": \"disabled\"\n"
"}\n";

#define CC "clang"
#define DEV_FLAGS "-g", "-O0", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-initializer-overrides", "-Wno-extra-semi", "-D_UNITY_BUILD_", "-DDEBUG"
#define RELEASE_FLAGS "-O2", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-initializer-overrides", "-Wno-extra-semi", "-D_UNITY_BUILD_"
#define WASM_FLAGS "-Os", "-O2", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-initializer-overrides", "-Wno-extra-semi", "-Wno-pthreads-mem-growth", "-D_UNITY_BUILD_"
#define TARGET "jurassic.c"
#define EXE "jurassic"
#define LDFLAGS "-lraylib", "-lm", "-lpthread"

#if defined(OS_WINDOWS)
#error "windows support not implemented"
#elif defined(OS_MAC)
#define CTAGS "/opt/homebrew/bin/ctags"
#elif defined(OS_LINUX)
#define CTAGS "/usr/local/bin/ctags"
#else
#error "unsupported operating system"
#endif

#if defined(OS_WINDOWS)
#error "windows support not implemented"
#elif defined(OS_MAC)
#define EMCC "/opt/homebrew/bin/emcc"
#define EMAR "/opt/homebrew/bin/emar"
#elif defined(OS_LINUX)
#define EMCC "emcc"
#define EMAR "emar"
#else
#error "unsupported operating system"
#endif

#if defined(OS_WINDOWS)
#error "windows support not implemented"

#elif defined(OS_MAC)
#define STATIC_BUILD_LDFLAGS "-lm", "-framework", "IOKit", "-framework", "Cocoa", "-framework", "OpenGL"

#elif defined(OS_LINUX)

#define STATIC_BUILD_LDFLAGS "-lm"

#else
#error "unsupported operating system"
#endif

#define METAPROGRAM_EXE "metaprogram"

#if defined(OS_WINDOWS)
#error "windows support not implemented"

#elif defined(OS_MAC)

#define GAME_MODULE "jurassic.dylib"
#define GAME_MODULE_PATH "./"GAME_MODULE
#define SHARED "-dynamiclib"
#define SHARED_EXT ".dylib"

#elif defined(OS_LINUX)

#define GAME_MODULE "jurassic.so"
#define GAME_MODULE_PATH "./"GAME_MODULE
#define SHARED "-shared"
#define SHARED_EXT ".so"

#else
#error "unsupported operating system"
#endif

#define RAYLIB_DYNAMIC_LINK_OPTIONS "-L./third_party/raylib/build/shared/",  "-I./third_party/raylib/", "-lraylib", "-Wl,-rpath,./third_party/raylib/build/shared/"
#define RAYLIB_DEBUG_LINK_OPTIONS "-L./third_party/raylib/build/debug/",  "-I./third_party/raylib/", "-lraylib", "-Wl,-rpath,./third_party/raylib/build/debug/"
#define RAYLIB_STATIC_LINK_OPTIONS "-I./third_party/raylib/", "-L./third_party/raylib/build/static/", "./third_party/raylib/build/static/libraylib.a"
#define RAYLIB_STATIC_LINK_WASM_OPTIONS "-I./third_party/raylib/", "-L./third_party/raylib/build/web/", "./third_party/raylib/build/web/libraylib.web.a"

#define RAYLIB_HEADERS "third_party/raylib/raylib.h", "third_party/raylib/raymath.h", "third_party/raylib/rlgl.h"


DECL_ARR_TYPE(Nob_Proc);
DECL_ARR_TYPE(Nob_Cmd);
DECL_ARR_TYPE(Str8);
DECL_SLICE_TYPE(Str8);
DECL_SLICE_TYPE(char_ptr);
DECL_SLICE_TYPE(void_ptr);


char *_raylib_cflags_mac[] = {
  "-Wall",
  "-D_GNU_SOURCE",
  "-DPLATFORM_DESKTOP_GLFW",
  "-DGRAPHICS_API_OPENGL_33",
  "-Wno-missing-braces",
  "-Werror=pointer-arith",
  "-fno-strict-aliasing",
  "-std=c99",
  "-O1",
  "-Werror=implicit-function-declaration",
};

char *_raylib_cflags_linux[] = {
  "-Wall",
  "-D_GNU_SOURCE",
  "-DPLATFORM_DESKTOP_GLFW",
  "-DGRAPHICS_API_OPENGL_33",
  "-Wno-missing-braces",
  "-Werror=pointer-arith",
  "-fno-strict-aliasing",
  "-std=c99",
  "-fPIC",
  "-O1",
  "-Werror=implicit-function-declaration",
  "-D_GLFW_X11",
};

char *_raylib_cflags_web[] = {
  "-Wall",
  "-D_GNU_SOURCE",
  "-DPLATFORM_WEB",
  "-DGRAPHICS_API_OPENGL_ES2",
  "-Wno-missing-braces",
  "-Werror=pointer-arith",
  "-fno-strict-aliasing",
  "-std=gnu99",
  "-fPIC",
  "-Os",
  "-Os",
  "-sWASM_WORKERS=1",
  "-matomics",
  "-mbulk-memory",
  "-sUSE_PTHREADS=1",
};

char *_raylib_debug_cflags_mac[] = {
  "-g",
  "-Wall",
  "-D_GNU_SOURCE",
  "-DPLATFORM_DESKTOP_GLFW",
  "-DGRAPHICS_API_OPENGL_33",
  "-Wno-missing-braces",
  "-Werror=pointer-arith",
  "-fno-strict-aliasing",
  "-std=c99",
  "-O0",
  "-Werror=implicit-function-declaration",
};

char *_raylib_debug_cflags_linux[] = {
  "-g",
  "-Wall",
  "-D_GNU_SOURCE",
  "-DPLATFORM_DESKTOP_GLFW",
  "-DGRAPHICS_API_OPENGL_33",
  "-Wno-missing-braces",
  "-Werror=pointer-arith",
  "-fno-strict-aliasing",
  "-std=c99",
  "-fPIC",
  "-O0",
  "-Werror=implicit-function-declaration",
  "-D_GLFW_X11",
};

char *_raylib_ldflags_mac[] = {
  "-install_name",
  "@rpath/libraylib.dylib",
  "-framework",
  "OpenGL",
  "-framework",
  "Cocoa",
  "-framework",
  "IOKit",
  "-framework",
  "CoreAudio",
  "-framework",
  "CoreVideo",
};

char *_raylib_ldflags_linux[] = {
  "-Wl,-soname,libraylib.so",
  "-lGL",
  "-lc",
  "-lm",
  "-lpthread",
  "-ldl",
  "-lrt",
};

char *_raylib_include_flags[] = {
  "-I./third_party/raylib",
  "-I./third_party/raylib/external/glfw/include",
};

Str8 _raylib_files[] = {
  str8_lit("rcore"),
  str8_lit("rshapes"),
  str8_lit("rtextures"),
  str8_lit("rtext"),
  str8_lit("utils"),
  str8_lit("rglfw"),
  str8_lit("rmodels"),
  str8_lit("raudio"),
};

Str8 _raylib_files_web[] = {
  str8_lit("rcore"),
  str8_lit("rshapes"),
  str8_lit("rtextures"),
  str8_lit("rtext"),
  str8_lit("utils"),
  str8_lit("rmodels"),
  str8_lit("raudio"),
};

const Slice(char_ptr) raylib_include_flags = { .d = _raylib_include_flags, .count = ARRLEN(_raylib_include_flags) };

const Slice(char_ptr) raylib_cflags_mac       = { .d = _raylib_cflags_mac,        .count = ARRLEN(_raylib_cflags_mac)        };
const Slice(char_ptr) raylib_debug_cflags_mac = { .d = _raylib_debug_cflags_mac,  .count = ARRLEN(_raylib_debug_cflags_mac)  };

const Slice(char_ptr) raylib_cflags_linux       = { .d = _raylib_cflags_linux,        .count = ARRLEN(_raylib_cflags_linux)        };
const Slice(char_ptr) raylib_debug_cflags_linux = { .d = _raylib_debug_cflags_linux,  .count = ARRLEN(_raylib_debug_cflags_linux)  };

const Slice(char_ptr) raylib_cflags_web       = { .d = _raylib_cflags_web,        .count = ARRLEN(_raylib_cflags_web)        };
//Slice(char_ptr) raylib_debug_cflags_web = { .d = _raylib_debug_cflags_web,  .count = ARRLEN(_raylib_debug_cflags_web)  };

const Slice(char_ptr) raylib_ldflags_mac = { .d = _raylib_ldflags_mac, .count = ARRLEN(_raylib_ldflags_mac) };
const Slice(char_ptr) raylib_ldflags_linux = { .d = _raylib_ldflags_linux, .count = ARRLEN(_raylib_ldflags_linux) };

const Slice(Str8) raylib_files = { .d = _raylib_files, .count = ARRLEN(_raylib_files) };
const Slice(Str8) raylib_files_web = { .d = _raylib_files_web, .count = ARRLEN(_raylib_files_web) };

#if defined(OS_WINDOWS)
Str8 raylib_shared_lib_name = str8_lit("raylib.dll");
#elif defined(OS_MAC)
Str8 raylib_shared_lib_name = str8_lit("libraylib.dylib");
Slice(char_ptr) raylib_cflags = raylib_cflags_mac;
Slice(char_ptr) raylib_debug_cflags = raylib_debug_cflags_mac;
Slice(char_ptr) raylib_ldflags = raylib_ldflags_mac;
#elif defined(OS_LINUX)
Str8 raylib_shared_lib_name = str8_lit("libraylib.so");
Slice(char_ptr) raylib_cflags = raylib_cflags_linux;
Slice(char_ptr) raylib_debug_cflags = raylib_debug_cflags_linux;
Slice(char_ptr) raylib_ldflags = raylib_ldflags_linux;
#else
#error unsupported OS
#endif

Str8 raylib_path = str8_lit("./third_party/raylib");

char _project_root_path[OS_PATH_LEN];
Str8 project_root_path;


/* * * * * * * * * * * * * * * * * 
 * function headers
 */

int build_raylib(void);
int bootstrap_project(void);
int build_metaprogram(void);
int run_metaprogram(void);
int build_hot_reload(void);
int build_hot_reload_cradle(void);
int build_hot_reload_no_cradle(void);
int build_release(void);
int build_wasm(void);
int build_itch(void);
int run_tags(void);
int gen_vim_project_file(void);
int gen_nob_project_file(void);
int load_nob_project_file(void);


int build_raylib(void) {
  nob_log(NOB_INFO, "building raylib");

  Str8 raylib_build_dir = scratch_push_str8f("%S/build", raylib_path);

  Str8 raylib_static_build_dir = scratch_push_str8f("%S/static", raylib_build_dir);
  Str8 raylib_shared_build_dir = scratch_push_str8f("%S/shared", raylib_build_dir);
  Str8 raylib_debug_build_dir = scratch_push_str8f("%S/debug", raylib_build_dir);
  Str8 raylib_web_build_dir = scratch_push_str8f("%S/web", raylib_build_dir);

  // TODO os_mkdir()
  ASSERT(nob_mkdir_if_not_exists((char*)raylib_build_dir.s));
  ASSERT(nob_mkdir_if_not_exists((char*)raylib_static_build_dir.s));
  ASSERT(nob_mkdir_if_not_exists((char*)raylib_shared_build_dir.s));
  ASSERT(nob_mkdir_if_not_exists((char*)raylib_debug_build_dir.s));
  ASSERT(nob_mkdir_if_not_exists((char*)raylib_web_build_dir.s));

  Str8 raylib_static_lib_path = scratch_push_str8f("%S/libraylib.a", raylib_static_build_dir);
  Str8 raylib_shared_lib_path = scratch_push_str8f("%S/%S", raylib_shared_build_dir, raylib_shared_lib_name);
  Str8 raylib_debug_lib_path = scratch_push_str8f("%S/%S", raylib_debug_build_dir, raylib_shared_lib_name);
  Str8 raylib_web_lib_path = scratch_push_str8f("%S/libraylib.web.a", raylib_web_build_dir);

  typedef enum Build_kind {
    BUILD_KIND_STATIC,
    BUILD_KIND_SHARED,
    BUILD_KIND_DEBUG,
    BUILD_KIND_WEB,
  } Build_kind;

  typedef struct Build {
    Build_kind kind;
    char *compiler;
    Str8 dir;
    Slice_Str8 files;
    Slice_char_ptr cflags;
  } Build;

  Build builds[] = {
    { .kind = BUILD_KIND_STATIC,
      .compiler = CC, .dir = raylib_static_build_dir, .files = raylib_files, .cflags = raylib_cflags },

    { .kind = BUILD_KIND_SHARED,
      .compiler = CC, .dir = raylib_shared_build_dir, .files = raylib_files, .cflags = raylib_cflags },

    { .kind = BUILD_KIND_DEBUG,
      .compiler = CC, .dir = raylib_debug_build_dir, .files = raylib_files, .cflags = raylib_debug_cflags },

    { .kind = BUILD_KIND_WEB,
      .compiler = EMCC, .dir = raylib_web_build_dir, .files = raylib_files_web, .cflags = raylib_cflags_web },
  };

  Arr(Str8) object_files;
  arr_init(object_files, context_scratch_arena);

  Arr(Nob_Cmd) linker_cmds;
  arr_init(linker_cmds, context_scratch_arena);

  Arr(Nob_Cmd) compile_cmds;
  arr_init(compile_cmds, context_scratch_arena);

  for(int build_i = 0; build_i < ARRLEN(builds); build_i++) {

    object_files.count = 0;

    char *compiler = builds[build_i].compiler;
    Str8 build_dir = builds[build_i].dir;
    Slice(Str8) files = builds[build_i].files;
    Slice(char_ptr) cflags = builds[build_i].cflags;

    for(int i = 0; i < files.count; i++) {
      Nob_Cmd cmd = {0};

      nob_cmd_append(&cmd, compiler);

#if defined(OS_MAC)
      if(str8_match_lit("rglfw", files.d[i])) {
        nob_cmd_append(&cmd, "-x", "objective-c");
      }
#endif

      Str8 file = files.d[i];
      Str8 src_file = scratch_push_str8f("%S/%S.c", raylib_path, file);
      Str8 object_file = scratch_push_str8f("%S/%S.o", build_dir, file);

      arr_push(object_files, object_file);

      nob_cmd_append(&cmd, "-c", (char*)src_file.s, "-o", (char*)object_file.s);
      nob_da_append_many(&cmd, cflags.d, cflags.count);
      nob_da_append_many(&cmd, raylib_include_flags.d, raylib_include_flags.count);

      if(str8_match_lit("rglfw", file)) {
        nob_cmd_append(&cmd, "-U_GNU_SOURCE");
      }

      arr_push(compile_cmds, cmd);
    }

    switch(builds[build_i].kind) {
      case BUILD_KIND_STATIC:
        {
          Nob_Cmd cmd = {0};

          nob_cmd_append(&cmd, "ar", "rcs", (char*)raylib_static_lib_path.s);

          for(int i = 0; i < object_files.count; i++) {
            nob_cmd_append(&cmd, (char*)object_files.d[i].s);
          }

          arr_push(linker_cmds, cmd);

        } break;
      case BUILD_KIND_SHARED:
        {
          Nob_Cmd cmd = {0};

          nob_cmd_append(&cmd, compiler, SHARED, "-o", (char*)raylib_shared_lib_path.s);

          for(int i = 0; i < object_files.count; i++) {
            nob_cmd_append(&cmd, (char*)object_files.d[i].s);
          }

          nob_da_append_many(&cmd, raylib_ldflags.d, raylib_ldflags.count);

          arr_push(linker_cmds, cmd);

        } break;
      case BUILD_KIND_DEBUG:
        {
          Nob_Cmd cmd = {0};

          nob_cmd_append(&cmd, compiler, SHARED, "-o", (char*)raylib_debug_lib_path.s);

          for(int i = 0; i < object_files.count; i++) {
            nob_cmd_append(&cmd, (char*)object_files.d[i].s);
          }

          nob_da_append_many(&cmd, raylib_ldflags.d, raylib_ldflags.count);

          arr_push(linker_cmds, cmd);

        } break;
      case BUILD_KIND_WEB:
        {
          Nob_Cmd cmd = {0};

          nob_cmd_append(&cmd, EMAR, "rcs", (char*)raylib_web_lib_path.s);

          for(int i = 0; i < object_files.count; i++) {
            nob_cmd_append(&cmd, (char*)object_files.d[i].s);
          }

          arr_push(linker_cmds, cmd);

        } break;
    }

  }

  nob_log(NOB_INFO, "compiling all builds of raylib");
  Arr(Nob_Proc) procs;
  scratch_arr_init(procs);

  for(int i = 0; i < compile_cmds.count; i++) {
    arr_push(procs, nob_cmd_run_async(compile_cmds.d[i]));
    //ASSERT(nob_proc_wait(procs.d[i]));
  }

  for(int i = 0; i < procs.count; i++) {
    ASSERT(nob_proc_wait(procs.d[i]));
  }

  procs.count = 0;

  nob_log(NOB_INFO, "linking all builds of raylib");

  for(int i = 0; i < linker_cmds.count; i++) {
    arr_push(procs, nob_cmd_run_async(linker_cmds.d[i]));
  }

  for(int i = 0; i < procs.count; i++) {
    ASSERT(nob_proc_wait(procs.d[i]));
  }

  procs.count = 0;

  return 1;
}

int build_metaprogram(void) {
  nob_log(NOB_INFO, "building metaprogram");

  run_tags();

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", "metaprogram.c", "-o", METAPROGRAM_EXE, RAYLIB_STATIC_LINK_OPTIONS, STATIC_BUILD_LDFLAGS);

  if(!nob_cmd_run_sync(cmd)) return 0;

  return 1;
}

int run_metaprogram(void) {
  nob_log(NOB_INFO, "running metaprogram");

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, scratch_push_cstrf("%S/metaprogram", project_root_path));

  if(!nob_cmd_run_sync(cmd)) return 0;

  return 1;
}

int build_hot_reload_no_cradle(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in hot reload mode");

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", SHARED, "module.c", RAYLIB_DEBUG_LINK_OPTIONS, "-o", GAME_MODULE, "-lm");

  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int build_hot_reload_cradle(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in hot reload mode");

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", "-DGAME_MODULE_PATH=\""GAME_MODULE_PATH"\"", "cradle.c", RAYLIB_DEBUG_LINK_OPTIONS, "-o", EXE, "-lm");

  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int build_hot_reload(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in hot reload mode");

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", SHARED, "module.c", RAYLIB_DEBUG_LINK_OPTIONS, "-o", GAME_MODULE, "-lm");
  Nob_Proc p1 = nob_cmd_run_async_and_reset(&cmd);

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", "-DGAME_MODULE_PATH=\""GAME_MODULE_PATH"\"", "cradle.c", RAYLIB_DEBUG_LINK_OPTIONS, "-o", EXE, "-lm");

  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;
  if(!nob_proc_wait(p1)) return 0;

  return 1;
}

int build_release(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in release mode");

  ASSERT(nob_mkdir_if_not_exists("build"));
  ASSERT(nob_mkdir_if_not_exists("./build/release"));

  nob_cmd_append(&cmd, CC, RELEASE_FLAGS, "static_cradle.c", RAYLIB_STATIC_LINK_OPTIONS, "-o", "./build/release/"EXE, STATIC_BUILD_LDFLAGS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int build_wasm(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building for WASM");

  ASSERT(nob_mkdir_if_not_exists("build"));
  ASSERT(nob_mkdir_if_not_exists("./build/wasm"));

  char *target = "wasm_cradle.c";
  nob_cmd_append(&cmd, EMCC, WASM_FLAGS, "--preload-file", "./aseprite/atlas.png", "--preload-file", "./sprites/islands.png", "--preload-file", "./sounds/", target, RAYLIB_STATIC_LINK_WASM_OPTIONS, RAYLIB_STATIC_LINK_WASM_OPTIONS, "-sEXPORTED_RUNTIME_METHODS=ccall", "-sUSE_GLFW=3", "-sFORCE_FILESYSTEM=1", "-sMODULARIZE=1", "-sWASM_WORKERS=1", "-sUSE_PTHREADS=1", "-sWASM=1", "-sEXPORT_ES6=1", "-sGL_ENABLE_GET_PROC_ADDRESS", "-sINVOKE_RUN=0", "-sNO_EXIT_RUNTIME=1", "-sMINIFY_HTML=0", "-o", "./build/wasm/jurassic.js", "-lpthread");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int build_itch(void) {
  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building for itch.io");

  ASSERT(nob_mkdir_if_not_exists("build"));
  ASSERT(nob_mkdir_if_not_exists("./build/itch"));

  char *target = "wasm_cradle.c";
  nob_cmd_append(&cmd, EMCC, WASM_FLAGS, "--preload-file", "./aseprite/atlas.png", "--preload-file", "./sprites/the_sea.png", "--preload-file", "./sounds/", target, RAYLIB_STATIC_LINK_WASM_OPTIONS, RAYLIB_STATIC_LINK_WASM_OPTIONS, "-sEXPORTED_RUNTIME_METHODS=ccall,HEAPF32", "-sUSE_GLFW=3", "-sFORCE_FILESYSTEM=1", "-sMODULARIZE=1", "-sWASM_WORKERS=1", "-sUSE_PTHREADS=1", "-sWASM=1", "-sEXPORT_ES6=1", "--shell-file", "itch_shell.html", "-sGL_ENABLE_GET_PROC_ADDRESS", "-sINVOKE_RUN=1", "-sNO_EXIT_RUNTIME=1", "-sMINIFY_HTML=0", "-sASYNCIFY", "-o", "./build/itch/index.html", "-pthread", "-sALLOW_MEMORY_GROWTH",scratch_push_cstrf("-sSTACK_SIZE=%lu", MB(10)));
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "sh", "-c", "zip ./build/itch/flight_22.zip ./build/itch/*");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}


int run_tags(void) {
  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd, CTAGS, "-w", "--sort=yes", "--langmap=c:.c.h", "--languages=c", "--c-kinds=+zfxm", "--extras=-q", "--fields=+n", "--exclude=third_party", "-R");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, CTAGS, "-w", "--sort=yes", "--langmap=c:.c.h", "--languages=c", "--c-kinds=+zpxm", "--extras=-q", "--fields=+n", "-a", RAYLIB_HEADERS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int gen_vim_project_file(void) {
  nob_log(NOB_INFO, "generating vim project file");
  Str8 path_str = scratch_push_str8f("%S/.project.vim", project_root_path);
  ASSERT(nob_write_entire_file((char*)path_str.s, vim_project_file, memory_strlen(vim_project_file)));
  return 1;
}

int gen_nob_vim_project_file(void) {
  nob_log(NOB_INFO, "generating nob project file");
  Str8 root_path = scratch_push_str8f("root = %S\n", os_get_current_dir());

  ASSERT(nob_write_entire_file(".project.nob", root_path.s, root_path.len));

  return 1;
}

int bootstrap_project(void) {
  nob_log(NOB_INFO, "bootstrapping project");

  gen_nob_vim_project_file();

  load_nob_project_file();

  gen_vim_project_file();

  { /* gen vscode project stuff */

    ASSERT(nob_mkdir_if_not_exists(".vscode"));

    ASSERT(nob_write_entire_file(".vscode/launch.json", vscode_launch, STRLEN(vscode_launch)));
    ASSERT(nob_write_entire_file(".vscode/settings.json", vscode_settings, STRLEN(vscode_settings)));

  } /* gen vscode project stuff */

  if(!build_raylib()) return 0;
  if(!build_metaprogram()) return 0;
  if(!run_metaprogram()) return 0;
  if(!run_tags()) return 0;
  if(!build_hot_reload()) return 0;

  return 1;
}

// TODO make nob work in subdirs of the project dir
int load_nob_project_file(void) {
  Nob_String_Builder sb = {0};

  Str8 cur_dir = os_get_current_dir();

  for(int i = 0; i < 4; i++) {
    if(!nob_read_entire_file(".project.nob", &sb)) {
      if(!nob_set_current_dir("..")) {
        nob_log(NOB_ERROR, "no .project.nob file found, please run bootstrap_project() before trying to build");
        return 1;
      }
    } else {
      os_set_current_dir(cur_dir);
      break;
    }
  }

  Nob_String_View sv = nob_sb_to_sv(sb);

  for(size_t i = 0; i < sv.count; i++) {
    if(sv.data[i] == '=') {
      Nob_String_View before = { .count = i, .data = sv.data };
      Nob_String_View after  = { .count = sv.count - (i+1), .data = sv.data + (i+1) };
      before = nob_sv_trim(before);
      after = nob_sv_trim(after);

      if(nob_sv_eq(before, nob_sv_lit("root"))) {
        project_root_path = (Str8){ .s = (u8*)after.data, .len = after.count };
      } else {
        nob_log(NOB_WARNING, "unknown option %s in .project.nob", before.data);
      }

    }
  }

  nob_log(NOB_INFO, "loaded .project.nob");

  return 1;
}

int main(int argc, char **argv) {

  context_init();

#if 0
  {

    NOB_GO_REBUILD_URSELF(argc, argv);

    bootstrap_project();

    return 0;
  }
#endif

  load_nob_project_file();

  ASSERT(os_set_current_dir(project_root_path));

  NOB_GO_REBUILD_URSELF(argc, argv);

  //if(!build_raylib()) return 1;
  //if(!build_metaprogram()) return 1;

  run_metaprogram();
  run_tags();

  //if(!build_release()) return 1;
  //if(!build_wasm()) return 1;
  //if(!build_itch()) return 1;
  if(!build_hot_reload_no_cradle()) return 1;
  //if(!build_hot_reload()) return 1;


  return 0;
}
