/* gameguy
   v1.0

   Gameguy is my toolset for making 3d games and game demos.
   It is heavily based on the handmade hero engine capabilities and provides a reusable platform
   layer that I can use to prototype and build 3d games and demos.

   To build a new project with gameguy you build a library that exports a ggGame struct.
   That struct contains a few pieces of information.
   * The amount of memory needed for the game (All allocated at init. @TODO let this grow)
   * An update_and_render function that takes a ggGameMemory pointer and a ggGameInput pointer.

   Then you can run the game from the command line with ./gameguy library_name.so

   Gameguy will handle setting up the window, allocating memory, recording input and calling
   your update_and_render function 60 times per second. It will also reload your library when
   it is recompiled and provides many debug features you can hook into for displaying and
   exploring information about the scene.

   In the future there may be the ability to modify settings for many of these pieces but for now
   it's very opinionated which lets me build many different demos and experiment rapidly. It uses
   SDL for window and input handling and opengl 3.3. None of it is tested on anything but osx.

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
    uint64_t permanent_storage_size;
    void*    permanent_storage;

    uint64_t transient_storage_size;
    void*    transient_storage;

    bool executable_reloaded;
    ggPlatformAPI platform_api;

    float dt;       // seconds since last frame.
    uint64_t ticks; // ms since game began.

    float display_width;
    float display_height;
    float drawable_width;
    float drawable_height;
} ggGameMemory;

typedef struct {
    
} ggGameInput;

typedef void (*UpdateAndRenderFn)(ggGameMemory *memory, ggGameInput* input);

typedef struct {
    int permanent_storage_size;
    int temp_storage_size;
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
debug_get_file_size(const char* filename)
{
    struct stat attr;
    stat(filename, &attr);
    return attr.st_size+1;
}

bool
debug_read_entire_file(const char* filename, char* buffer, size_t buffer_size)
{
    FILE* f = fopen(filename, "r");
    fread(buffer, 1, buffer_size-1, f);
    fclose(f);

    buffer[buffer_size-1] = '\0';
    
    return true;
}

bool
game_reload(CurrentGame* current_game, const char* library)
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

int
main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: must specify library filename.\n");
        exit(1);
    }
    
    const char* library_filename = argv[1];
    fprintf(stderr, "Loading Library: %s\n", library_filename);
    
    CurrentGame game = {.handle = NULL,
                        .id = 0,
                        .gg_game = NULL};
    game_reload(&game, library_filename);

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

    SDL_Window *window = SDL_CreateWindow("gameguy",
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
    game_memory.dt = target_seconds_per_frame;

    game_memory.platform_api.get_file_size = debug_get_file_size;
    game_memory.platform_api.read_entire_file = debug_read_entire_file;

    game_memory.permanent_storage_size = game.gg_game->permanent_storage_size;
    game_memory.transient_storage_size = game.gg_game->temp_storage_size;

    void* raw_memory = calloc(1,
                              game_memory.permanent_storage_size +
                              game_memory.transient_storage_size);

    game_memory.permanent_storage = raw_memory;
    game_memory.transient_storage = (char*)raw_memory + game_memory.permanent_storage_size;
    
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
        game_memory.executable_reloaded = game_reload(&game, library_filename);
        game_memory.ticks = start_time;
        game_memory.dt = dt;
        game_memory.display_width = w;
        game_memory.display_height = h;
        game_memory.drawable_width = draw_w;
        game_memory.drawable_height = draw_h;

#if 0 // @TODO sdl input
        // Handle SDL Events
        for (int i=0; i<ARRAY_COUNT(input.buttons); i++) {
            input.buttons[i].half_transition_count = 0;
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
                    input.button.mouse_1.half_transition_count++;
                    input.button.mouse_1.ended_down = true;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    input.button.mouse_1.half_transition_count++;
                    input.button.mouse_1.ended_down = false;
                }
                break;
                
            case SDL_KEYDOWN:
                if (SDLK_ESCAPE == event.key.keysym.sym) {
                    running = false;
                }

                switch(event.key.keysym.sym) {
                case(SDLK_LCTRL):
                    input.button.ctrl.half_transition_count++;
                    input.button.ctrl.ended_down = true;
                    break;
                case(SDLK_LSHIFT):
                    input.button.shift.half_transition_count++;
                    input.button.shift.ended_down = true;
                    break;
                case(SDLK_w):
                    input.button.move_up.half_transition_count++;
                    input.button.move_up.ended_down = true;
                    break;
                case(SDLK_a):
                    input.button.move_left.half_transition_count++;
                    input.button.move_left.ended_down = true;
                    break;
                case(SDLK_s):
                    input.button.move_down.half_transition_count++;
                    input.button.move_down.ended_down = true;
                    break;
                case(SDLK_d):
                    input.button.move_right.half_transition_count++;
                    input.button.move_right.ended_down = true;
                    break;
                case(SDLK_UP):
                    input.button.action_up.half_transition_count++;
                    input.button.action_up.ended_down = true;
                    break;
                case(SDLK_LEFT):
                    input.button.action_left.half_transition_count++;
                    input.button.action_left.ended_down = true;
                    break;
                case(SDLK_DOWN):
                    input.button.action_down.half_transition_count++;
                    input.button.action_down.ended_down = true;
                    break;
                case(SDLK_RIGHT):
                    input.button.action_right.half_transition_count++;
                    input.button.action_right.ended_down = true;
                    break;
                case(SDLK_q):
                    input.button.left_bumper.half_transition_count++;
                    input.button.left_bumper.ended_down = true;
                    break;
                case(SDLK_e):
                    input.button.right_bumper.half_transition_count++;
                    input.button.right_bumper.ended_down = true;
                    break;
                };
                break;
                
            case SDL_KEYUP:
                switch(event.key.keysym.sym) {
                case(SDLK_LCTRL):
                    input.button.ctrl.half_transition_count++;
                    input.button.ctrl.ended_down = false;
                    break;
                case(SDLK_LSHIFT):
                    input.button.shift.half_transition_count++;
                    input.button.shift.ended_down = false;
                    break;
                case(SDLK_w):
                    input.button.move_up.half_transition_count++;
                    input.button.move_up.ended_down = false;
                    break;
                case(SDLK_a):
                    input.button.move_left.half_transition_count++;
                    input.button.move_left.ended_down = false;
                    break;
                case(SDLK_s):
                    input.button.move_down.half_transition_count++;
                    input.button.move_down.ended_down = false;
                    break;
                case(SDLK_d):
                    input.button.move_right.half_transition_count++;
                    input.button.move_right.ended_down = false;
                    break;
                case(SDLK_UP):
                    input.button.action_up.half_transition_count++;
                    input.button.action_up.ended_down = false;
                    break;
                case(SDLK_LEFT):
                    input.button.action_left.half_transition_count++;
                    input.button.action_left.ended_down = false;
                    break;
                case(SDLK_DOWN):
                    input.button.action_down.half_transition_count++;
                    input.button.action_down.ended_down = false;
                    break;
                case(SDLK_RIGHT):
                    input.button.action_right.half_transition_count++;
                    input.button.action_right.ended_down = false;
                    break;
                case(SDLK_q):
                    input.button.left_bumper.half_transition_count++;
                    input.button.left_bumper.ended_down = false;
                    break;
                case(SDLK_e):
                    input.button.right_bumper.half_transition_count++;
                    input.button.right_bumper.ended_down = false;
                    break;
                };
                break;
            default:
                break;
            }
        }
#endif
        if (running == false) {
            break;
        }

        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

#if 0
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS) {
            io.MousePos.x = (float)mouse_x;
            io.MousePos.y = (float)mouse_y;
        } else {
            io.MousePos.x = -1;
            io.MousePos.y = -1;
        }

        io.MouseDown[0] = input.button.mouse_1.ended_down ||
            input.button.mouse_1.half_transition_count > 1;
#endif

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Run Game Tick
        game.gg_game->update_and_render(&game_memory, &input);
        
        // End Frame        
        update_time = SDL_GetTicks() - start_time;

        float time_till_vsync = target_seconds_per_frame*1000.0 - (SDL_GetTicks() - start_time);
        if (time_till_vsync > 3) {
            SDL_Delay(time_till_vsync - 2);
        }

        SDL_GL_SwapWindow(window);

        uint64_t end_counter = SDL_GetPerformanceCounter();
        last_counter = end_counter;
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#endif
