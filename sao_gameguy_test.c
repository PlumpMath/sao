#include "sao_gameguy.h"

#include <stdlib.h>
#include <stdio.h>

void
game_update_and_render(ggGameMemory *memory, ggGameInput* input) {
    if (memory->executable_reloaded) {
            fprintf(stderr, "Hello World 2\n");
    }
}

ggGame gg_game = {
    .permanent_storage_size = 512,
    .temp_storage_size = 512,
    .update_and_render = game_update_and_render
};
