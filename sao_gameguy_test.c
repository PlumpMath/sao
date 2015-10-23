#include "sao_gameguy.h"

#include <stdlib.h>
#include <stdio.h>

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
    GameState* game_state = (GameState*)memory;
    if (!game_state->is_initialized) {
        GLint program = glCreateProgram();
        GLint vert_shader = glCreateShader(GL_VERTEX_SHADER);
        GLint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vert_shader, 1, &vertex_shader, 0);
        glShaderSource(frag_shader, 1, &fragment_shader, 0);
        glCompileShader(vert_shader);
        glCompileShader(frag_shader);
        glAttachShader(program, vert_shader);
        glAttachShader(program, frag_shader);
        glLinkProgram(program);

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

    glBindVertexArray(game_state->triangle_vao);
    glUseProgram(game_state->shader_program);
    glDrawArrays(GL_TRIANGLES, 0, 9);

}

ggGame gg_game = {
    .permanent_storage_size = sizeof(GameState),
    .temp_storage_size = 0,
    .update_and_render = game_update_and_render
};
