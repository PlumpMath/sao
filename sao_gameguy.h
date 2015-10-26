/* gameguy
   v2.0, changes for gouplabs

   Gameguy is my toolset for making 3d games and game demos.
   It is heavily based on the handmade hero engine capabilities and provides a reusable
   platform layer that I can use to prototype and build 3d games and demos.

   To build a new project with gameguy you build a library that exports a ggGame struct.
   That struct contains a few pieces of information.
   * An update_and_render function that takes a ggGameMemory pointer and a ggGameInput pointer.

   Then you can run the game from the command line with ./gameguy library_name.so

   Gameguy will handle setting up the window, recording input and calling
   your update_and_render function 60 times per second. It will also reload your library when
   it is recompiled and provides many debug features you can hook into for displaying and
   exploring information about the scene.
   You can put a pointer to your gamestate and it's size int he game memory and the platform
   layer can use that to loop gameplay for looped live code editing. @TODO

   In the future there may be the ability to modify settings for many of these pieces but for
   now it's very opinionated which lets me build many different demos and experiment rapidly.
   It uses SDL for window and input handling and opengl 3.3. None of it is tested on anything
   but osx.

   Future wanted features.
   - Building static releases for multiple platforms.
   - Input/Gamestate looping.
   - Tunable opengl settings (for now I just modify gameguy for each project)
   - More debug features.
   - More os features.
   - Networking features.
 */
#ifndef _sao_gameguy_h
#define _sao_gameguy_h

// This is needed for size_t and other types, @TODO ditch somehow.
#include <stdlib.h>
#include <stdbool.h> 

typedef int (*GetFileSizeFn)(const char* filename);
typedef bool (*ReadEntireFileFn)(const char* filename, char* buffer, size_t buffer_size);

typedef struct {
    GetFileSizeFn get_file_size;
    ReadEntireFileFn read_entire_file;
} ggPlatformAPI;

typedef struct {
    // Use this pointer to record any memory you want the platform layer to keep track of.
    // This memory will be saved and replayed for looped editing and debugging.
    // Put gamestate here.
    uint64_t persistent_storage_size;
    void*    persistent_storage;

    /* uint64_t transient_storage_size; */
    /* void*    transient_storage; */

    bool executable_reloaded;
    ggPlatformAPI platform_api;

    float dt;       // seconds since last frame.
    uint64_t ticks; // ms since game began.

    float display_width;
    float display_height;
    float drawable_width;
    float drawable_height;
} ggGameMemory;

// Do I want casey style buttons.
// half_transition_count, ended_down.
// mouse_moved, mouse position.

typedef struct {
    bool ended_down;
    int half_transition_count;
} ggButton;

typedef struct {
    // Mouse. (movement and clicks)
    ggButton mouse1;
    ggButton mouse2;

    int horisontal_scroll;
    int vertical_scroll;

    float mouse_x;
    float mouse_y;

    bool mouse_moved;
    float mouse_dx;
    float mouse_dy;

    // Buttons, used for game input situations.
    union {
        struct {
            ggButton w;
            ggButton a;
            ggButton s;
            ggButton d;
            ggButton q;
            ggButton e;
            ggButton up;
            ggButton down;
            ggButton left;
            ggButton right;
            ggButton space;
            ggButton ctrl;
            ggButton shift;
            ggButton n1;
            ggButton n2;
            ggButton n3;
            ggButton n4;
            ggButton n5;
            ggButton n6;
            ggButton n7;
            ggButton n8;
            ggButton n9;
            ggButton n0;
            ggButton escape;
            ggButton ret;
        } b;
        ggButton e[25];
    } button;

    // Keyboard string input.
    // @TODO
} ggGameInput;

typedef void (*UpdateAndRenderFn)(ggGameMemory *memory, ggGameInput* input);

typedef struct {
    /* int permanent_storage_size; */
    /* int temp_storage_size; */
    UpdateAndRenderFn update_and_render;
} ggGame;

// Set this in game code.
extern ggGame gg_game;

#endif

#ifdef SAO_GAMEGUY_IMPLEMENTATION

#include <SDL2/SDL.h>
#include <OpenGL/gl3.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <assert.h>

typedef struct {
    void *handle;
    ino_t id;
    ggGame* gg_game;
} CurrentGame;

int
gg_debug_get_file_size(const char* filename)
{
    struct stat attr;
    stat(filename, &attr);
    return attr.st_size+1;
}

bool
gg_debug_read_entire_file(const char* filename, char* buffer, size_t buffer_size)
{
    FILE* f = fopen(filename, "r");
    fread(buffer, 1, buffer_size-1, f);
    fclose(f);

    buffer[buffer_size-1] = '\0';
    
    return true;
}

bool
gg_game_reload(CurrentGame* current_game, const char* library)
{
    bool game_reloaded = false;
    
    struct stat attr;
    if ((stat(library, &attr) == 0) && (current_game->id != attr.st_ino)) {
        fprintf(stderr, "New Library to load.\n");
        
        if (current_game->handle) {
            dlclose(current_game->handle);
        }
        
        void* handle = dlopen(library, RTLD_NOW);

        if (handle) {
            current_game->handle = handle;
            current_game->id = attr.st_ino;

            ggGame* game = (ggGame*)dlsym(handle, "gg_game");
            if (game != NULL) {
                current_game->gg_game = game;
                fprintf(stderr, "Reloaded Game Library\n");
                game_reloaded = true;
                
            } else {
                fprintf(stderr, "[error] Error loading api symbol.\n");
                dlclose(handle);
                current_game->handle = NULL;
                current_game->id = 0;
            }
            
        } else {
            fprintf(stderr, "Error loading game library.\n");
            current_game->handle = NULL;
            current_game->id = 0;
        }
    }

    return game_reloaded;
}

// @TODO: Metaprogram this or use a hash or something.
int
_gg_get_button_index(SDL_Keycode sym)
{
    int button_index;
    switch(sym) {
    case(SDLK_w):
        button_index = 0;
        break;
    case(SDLK_a):
        button_index = 1;
        break;
    case(SDLK_s):
        button_index = 2;
        break;
    case(SDLK_d):
        button_index = 3;
        break;
    case(SDLK_q):
        button_index = 4;
        break;
    case(SDLK_e):
        button_index = 5;
        break;
    case(SDLK_UP):
        button_index = 6;
        break;
    case(SDLK_DOWN):
        button_index = 7;
        break;
    case(SDLK_LEFT):
        button_index = 8;
        break;
    case(SDLK_RIGHT):
        button_index = 9;
        break;
    case(SDLK_SPACE):
        button_index = 10;
        break;
    case(SDLK_LCTRL):
        button_index = 11;
        break;
    case(SDLK_LSHIFT):
        button_index = 12;
        break;
    case(SDLK_1):
        button_index = 13;
        break;
    case(SDLK_2):
        button_index = 14;
        break;
    case(SDLK_3):
        button_index = 15;
        break;
    case(SDLK_4):
        button_index = 16;
        break;
    case(SDLK_5):
        button_index = 17;
        break;
    case(SDLK_6):
        button_index = 18;
        break;
    case(SDLK_7):
        button_index = 19;
        break;
    case(SDLK_8):
        button_index = 20;
        break;
    case(SDLK_9):
        button_index = 21;
        break;
    case(SDLK_0):
        button_index = 22;
        break;
    case(SDLK_ESCAPE):
        button_index = 23;
        break;
    case(SDLK_RETURN):
        button_index = 24;
        break;
    };
    return button_index;
}

int
main(int argc, char* argv[]) {
#ifndef SAO_GAMEGUY_STATIC_LINK
    #ifndef SAO_GAMEGUY_LIBRARY_NAME
    if (argc != 2) {
        fprintf(stderr, "Error: must specify library filename.\n");
        exit(1);
    }
    const char* library_filename = argv[1];
    #else
    const char* library_filename = SAO_GAMEGUY_LIBRARY_NAME;
    #endif
    fprintf(stderr, "Loading Library: %s\n", library_filename);

    CurrentGame game = {.handle = NULL,
                        .id = 0,
                        .gg_game = NULL};
    gg_game_reload(&game, library_filename);
#endif

    float game_update_hz = 60;
    float target_seconds_per_frame = 1.0f / game_update_hz;

    // Init Stuff
    if (SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Error initializing SDL: %s\ns", SDL_GetError());
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    #ifdef SAO_GAMEGUY_WINDOW_NAME
    char* window_name = SAO_GAMEGUY_WINDOW_NAME;
    #else
    char* window_name = "gameguy";
    #endif

    SDL_Window *window = SDL_CreateWindow(window_name,
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1920,
                                          1080,
                                          SDL_WINDOW_OPENGL |
                                          SDL_WINDOW_RESIZABLE |
                                          SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
    }

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    fprintf(stderr, "Drawable Resolution: %d x %d\n", width, height);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        fprintf(stderr, "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    ggGameMemory game_memory = {};
    game_memory.persistent_storage_size = 0;
    game_memory.persistent_storage = NULL;
    
    game_memory.dt = target_seconds_per_frame;

    game_memory.platform_api.get_file_size = gg_debug_get_file_size;
    game_memory.platform_api.read_entire_file = gg_debug_read_entire_file;
    
    ggGameInput input = {};

    uint32_t update_time = 0;
    uint32_t frame_time = 0;

    uint64_t last_counter = SDL_GetPerformanceCounter();
    uint32_t last_start_time = SDL_GetTicks();

    bool running = true;

    while (running) {
        uint32_t start_time = SDL_GetTicks();
        frame_time = start_time - last_start_time;
        float dt = (float)frame_time / 1000.0;
        last_start_time = start_time;

        int w, h;
	SDL_GetWindowSize(window, &w, &h);

        int draw_w, draw_h;
        SDL_GL_GetDrawableSize(window, &draw_w, &draw_h);

        // Reload Game
#ifndef SAO_GAMEGUY_STATIC_LINK
        game_memory.executable_reloaded = gg_game_reload(&game, library_filename);
#endif
        game_memory.ticks = start_time;
        game_memory.dt = dt;
        game_memory.display_width = w;
        game_memory.display_height = h;
        game_memory.drawable_width = draw_w;
        game_memory.drawable_height = draw_h;

        /* Handle SDL Events */
        for (int i=0; i<sizeof(input.button.e)/sizeof(input.button.e[0]); i++) {
            input.button.e[i].half_transition_count = 0;
        }

        input.horisontal_scroll = 0;
        input.vertical_scroll = 0;
        
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // handle event
            switch(event.type) {
            case SDL_QUIT:
                running = false;
                break;
                
            case SDL_MOUSEWHEEL:
                input.horisontal_scroll += event.wheel.x;
                input.vertical_scroll += event.wheel.y;
                break;
                
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    input.mouse1.half_transition_count++;
                    input.mouse1.ended_down = true;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    input.mouse1.half_transition_count++;
                    input.mouse1.ended_down = false;
                }
                break;

            case SDL_KEYDOWN: {
                if (SDLK_ESCAPE == event.key.keysym.sym) {
                    running = false;
                }

                int button_index = _gg_get_button_index(event.key.keysym.sym);
                input.button.e[button_index].half_transition_count++;
                input.button.e[button_index].ended_down = true;
            } break;

            case SDL_KEYUP: {
                int button_index = _gg_get_button_index(event.key.keysym.sym);
                input.button.e[button_index].half_transition_count++;
                input.button.e[button_index].ended_down = false;
            } break;
                
            default:
                break;
            };
        }
        
        if (running == false) {
            break;
        }

        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS) {
            if (mouse_x != input.mouse_x ||
                mouse_y != input.mouse_y) {
                input.mouse_moved = true;
                input.mouse_dx = mouse_x - input.mouse_x;
                input.mouse_dy = mouse_y - input.mouse_y;
                
            }
            input.mouse_x = mouse_x;
            input.mouse_y = mouse_y;

        } else {
            input.mouse_x = -1;
            input.mouse_y = -1;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Run Game Tick
        #ifdef SAO_GAMEGUY_STATIC_LINK
        gg_game.update_and_render(&game_memory, &input);
        #else
        game.gg_game->update_and_render(&game_memory, &input);
        #endif
        
        // End Frame        
        update_time = SDL_GetTicks() - start_time;

        float time_till_vsync = target_seconds_per_frame*1000.0 - (SDL_GetTicks()-start_time);
        if (time_till_vsync > 3) {
            SDL_Delay(time_till_vsync - 2);
        }

        SDL_GL_SwapWindow(window);

        uint64_t end_counter = SDL_GetPerformanceCounter();
        last_counter = end_counter;
    }

    fprintf(stderr, "Closing\n");
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#endif
