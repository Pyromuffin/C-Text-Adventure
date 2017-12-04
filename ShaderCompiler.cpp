
#include <string>
#include <fstream>
#include <sstream>

#include <SFML/Graphics.hpp>

#include "ShaderCompiler.h"



GLuint CompileShader(const char* path, GLenum shaderType) 
{
	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();

	std::string str = buffer.str();
	auto cstr = str.c_str();

	auto shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &cstr, NULL);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE)
	{
		GLint logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
		std::string log;
		log.reserve(logSize);

		glGetShaderInfoLog(shader, log.capacity(), NULL, log.data());
		printf("%s\n", log.c_str());

	}

	return shader;
}

GLuint LinkShaders(GLuint vs, GLuint fs)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		//The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

		printf("%s\n", &infoLog[0]);
	}

	return program;
}
	
