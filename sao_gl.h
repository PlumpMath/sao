/*
  My opengl helper library.
  
  A lot of this really isn't portable. It all targets opengl 3.3 and currently only works on osx.
  Should work fine with sao_gameguy.h

  - Stephen Olsen
 */
#ifndef _sao_gl_h
#define _sao_gl_h

int saogl_compile_shader_program(const char* vertex_shader_src, const char* fragment_shader_src);
// Returns an opengl shader program or 0 on error. Writes any errors to stderr.

#endif

#ifdef SAO_GL_IMPLEMENTATION

#include <stdio.h>
#include <stdint.h>
#include <OpenGL/gl3.h>

int
_saogl_check_shader_error(GLint shader)
{
    int is_ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_ok);
    if (is_ok) {
        return 0;

    } else {
        int log_length;
        char* log;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        log = (char*)malloc(log_length);
        glGetShaderInfoLog(shader, log_length, NULL, log);
        
        fprintf(stderr, "Error Compiling Shader: %s", log);
        free(log);
        return 1;
    }
}

int
_saogl_check_program_error(GLint program)
{
    int is_ok;
    glGetProgramiv(program, GL_LINK_STATUS, &is_ok);
    if (is_ok) {
        return 0;

    } else {
        int log_length;
        char* log;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        log = (char*)malloc(log_length);
        glGetProgramInfoLog(program, log_length, NULL, log);
        
        fprintf(stderr, "Error Linking Program: %s", log);
        free(log);
        return 1;
    }
}

int
saogl_compile_shader_program(const char* vertex_shader_src, const char* fragment_shader_src)
{
    GLint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vert_shader);
    if (_saogl_check_shader_error(vert_shader)) {
        glDeleteShader(vert_shader);
        return 1;
    }

    GLint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(frag_shader);
    if (_saogl_check_shader_error(frag_shader)) {
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
        return 1;
    }

    GLint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    if (_saogl_check_program_error(program)) {
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
        glDeleteProgram(program);
        return 1;
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return program;
}

#endif
