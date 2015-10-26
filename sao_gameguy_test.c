#include "sao_gameguy.h"

#include <stdlib.h>
#include <stdio.h>

#define SAO_GL_IMPLEMENTATION
#include "sao_gl.h"

#include <OpenGL/gl3.h>

const GLchar *vertex_shader =
    "#version 330\n"
    "layout (location = 0) in vec3 position;\n"
    "void main()\n"
    "{\n"
    "	gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
    "}\n";

const GLchar* fragment_shader =
    "#version 330\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "	color = vec4(1.0,0.0,0.0,1.0);\n"
    "}\n";

typedef struct {
    bool is_initialized;
    GLint shader_program;
    GLuint triangle_vao;
    GLuint triangle_vertex_vbo;
} GameState;

// Make a triangle spin or something. Good proof of concept.
void
game_update_and_render(ggGameMemory *memory, ggGameInput* input) {
    GameState* game_state = (GameState*)memory->persistent_storage;
    if (!game_state) {
        // Allocate memory to use by the game and store it's pointer and size in game memory
        // so that the platform layer can manage it when reloading code.
        memory->persistent_storage_size = sizeof(GameState);
        memory->persistent_storage = calloc(1, sizeof(GameState));
        game_state = (GameState*)memory->persistent_storage;

        // Set up an opengl triangle.
        GLint program = saogl_compile_shader_program(vertex_shader, fragment_shader);
        glUseProgram(program);

        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.0f,  0.5f, 0.0f
        };

        glGenVertexArrays(1, &game_state->triangle_vao);
        glBindVertexArray(game_state->triangle_vao);

        glGenBuffers(1, &game_state->triangle_vertex_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, game_state->triangle_vertex_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        int position_i = glGetAttribLocation(program, "position");
        glVertexAttribPointer(position_i, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        game_state->shader_program = program;
    }

    if (input->button.b.w.half_transition_count > 1 ||
        (input->button.b.w.ended_down &&
         input->button.b.w.half_transition_count == 1)) {
        fprintf(stderr, "You pressed w\n");
    }

    // Draw opengl triangle.
    glBindVertexArray(game_state->triangle_vao);
    glUseProgram(game_state->shader_program);
    glDrawArrays(GL_TRIANGLES, 0, 9);
}

ggGame gg_game = {
    .update_and_render = game_update_and_render
};
