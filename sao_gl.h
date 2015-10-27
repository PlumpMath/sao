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

typedef int (*saogl_GetFileSizeFn)(const char* filename);
typedef bool (*saogl_ReadEntireFileFn)(const char* filename, char* buffer, size_t buffer_size);

int
saogl_build_shader_from_files(saogl_GetFileSizeFn get_file_size,
                              saogl_ReadEntireFileFn read_file,
                              const char* vertex_shader_filename,
                              const char* fragment_shader_filename);

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
        return -1;
    }

    GLint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(frag_shader);
    if (_saogl_check_shader_error(frag_shader)) {
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
        return -1;
    }

    GLint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    if (_saogl_check_program_error(program)) {
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
        glDeleteProgram(program);
        return -1;
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return program;
}


int
saogl_build_shader_from_files(saogl_GetFileSizeFn get_file_size,
                              saogl_ReadEntireFileFn read_file,
                              const char* vertex_shader_filename,
                              const char* fragment_shader_filename)
{
    // Load shaders.
    int vert_shader_size = get_file_size(vertex_shader_filename);
    if (vert_shader_size < 0) {
        fprintf(stderr, "Error: couldn't read vertex shader file\n");
        return -1;
    }
    char* vert_buf = malloc(vert_shader_size);
    read_file(vertex_shader_filename, vert_buf, vert_shader_size);
    /* fprintf(stderr, "Vertex Shader: %s", vert_buf); */

    int frag_shader_size = get_file_size(fragment_shader_filename);
    if (frag_shader_size < 0) {
        fprintf(stderr, "Error: couldn't read fragment shader file\n");
        return -1;
    }
    char* frag_buf = malloc(frag_shader_size);
    read_file(fragment_shader_filename, frag_buf, frag_shader_size);
    /* fprintf(stderr, "Fragment Shader: %s", frag_buf); */

    GLint shader = saogl_compile_shader_program(vert_buf, frag_buf);
    if (shader < 0) {
        fprintf(stderr, "Error: Couldn't compile shaders\n");
        return -1;
    }

    return shader;
}
#endif
