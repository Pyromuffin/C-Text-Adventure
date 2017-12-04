#pragma once

#include <SFML/OpenGL.hpp>

GLuint CompileShader(const char* path, GLenum shaderType);
GLuint LinkShaders(GLuint vs, GLuint fs);

