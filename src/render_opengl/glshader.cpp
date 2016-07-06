// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#include <iostream>
#include <algorithm>
#include <map>
#include "gl.h"
#include "gl_debug.h"
#include "../rage.h"
#include "glshader.h"

using namespace std;


/**
* Creates and compile an OpenGL "shader" object
* @return GLuint
**/
GLuint GLShader::createShader(const char* code, GLenum type)
{
	GLint success;
	const char* strings[2];
	int lengths[2];

	GLuint shader = glCreateShader(type);
	if (shader == 0) {
		return 0;
	}

	strings[0] = "#version 130\n";
	lengths[0] = strlen(strings[0]);

	strings[1] = code;
	lengths[1] = strlen(strings[1]);

	glShaderSource(shader, 2, strings, lengths);

	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (! success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(shader, 1024, NULL, InfoLog);
		string log = string(InfoLog);
		log = replaceString(log, "\n", "\n\t");
		log = trimString(log);
		GL_LOG("Error compiling shader:\n\t%s", log.c_str());
		return 0;
	}

	return shader;
}


/**
* Create and link a shader program from two compiled shaders
* Does NOT delete the linked shaders - calling code needs to do this
* @return True on success, false on failure
**/
bool GLShader::createProgFromShaders(GLuint vertex, GLuint fragment)
{
	GLint success;

	// Create program object
	GLuint program = glCreateProgram();
	if (program == 0) {
		return false;
	}

	glAttachShader(program, vertex);
	glAttachShader(program, fragment);

	// Bind attribs
	glBindAttribLocation(program, ATTRIB_POSITION, "vPosition");
	glBindAttribLocation(program, ATTRIB_NORMAL, "vNormal");
	glBindAttribLocation(program, ATTRIB_TEXUV + 0, "vTexUV0");
	glBindAttribLocation(program, ATTRIB_TEXUV + 1, "vTexUV1");
	glBindAttribLocation(program, ATTRIB_BONEID, "vBoneIDs");
	glBindAttribLocation(program, ATTRIB_BONEWEIGHT, "vBoneWeights");
	glBindAttribLocation(program, ATTRIB_TEXTCOORD, "vCoord");
	glBindAttribLocation(program, ATTRIB_COLOR, "vColor");
	glBindAttribLocation(program, ATTRIB_TANGENT, "vTangent");
	glBindAttribLocation(program, ATTRIB_BITANGENT, "vBitangent");

	// Link
	glLinkProgram(program);

	// Check link worked
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (! success) {
		GLchar infolog[1024];
		glGetProgramInfoLog(program, 1024, NULL, infolog);
		GL_LOG("Error linking program\n%s", infolog);
		return false;
	}

	CHECK_OPENGL_ERROR;

	this->program = program;
	return true;
}


/**
* Creates and link a shader program from two shader code strings
* @return True on success, false on failure
**/
bool GLShader::createProgFromStrings(const char* vertex, const char* fragment)
{
	GLuint sVertex, sFragment;
	bool result;

	sVertex = this->createShader(vertex, GL_VERTEX_SHADER);
	if (sVertex == 0) {
		GL_LOG("Invalid vertex shader (program %u)", program);
		return false;
	}

	sFragment = this->createShader(fragment, GL_FRAGMENT_SHADER);
	if (sFragment == 0) {
		glDeleteShader(sVertex);
		GL_LOG("Invalid fragment shader (program %u)", program);
		return false;
	}

	result = this->createProgFromShaders(sVertex, sFragment);

	glDeleteShader(sVertex);
	glDeleteShader(sFragment);

	return result;
}


/**
* Return the shader program id
**/
GLuint GLShader::p()
{
	return this->program;
}


/**
* Return a uniform location id
**/
GLuint GLShader::uniform(const char* name)
{
	map<const char*, GLuint>::iterator it = this->uniforms.find(name);
	if (it != this->uniforms.end()) {
		return it->second;
	}

	GLuint loc = glGetUniformLocation(this->program, name);

	this->uniforms.insert(std::pair<const char*, GLuint>(name, loc));

	return loc;
}
