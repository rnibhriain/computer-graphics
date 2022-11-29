#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "maths_funcs.h"


class Shader
{
public:
	// the program ID
	unsigned int ID;

	// constructor reads and builds the shader
	Shader();
	Shader(const char* vertexPath, const char* fragmentPath);
	// use/activate the shader
	void use();
	// utility uniform functions
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec3(const std::string& name, vec3 value) const;
	void setMat4(const std::string& name, const mat4& mat) const;
private:
	char* readShaderSource(const char* shaderFile);
	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
	GLuint CompileShader(const char* vertexPath, const char* fragmentPath);
};

#endif