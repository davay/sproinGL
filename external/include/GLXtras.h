// GLXtras.h - GLSL convenience routines

#ifndef GL_XTRAS_HDR
#define GL_XTRAS_HDR

#include "glad.h"
#include "VecMat.h"

// Print Info
int PrintGLErrors();
void PrintVersionInfo();
void PrintExtensions();
void PrintProgramLog(int programID);
void PrintProgramAttributes(int programID);
void PrintProgramUniforms(int programID);

// Shader Compilation
GLuint CompileShaderViaFile(const char *filename, GLint type);
GLuint CompileShaderViaCode(const char **code, GLint type);

// Program Linking
GLuint LinkProgramViaCode(const char **vertexCode, const char **pixelCode);
GLuint LinkProgramViaCode(const char **vertexCode, const char **tessellationControlCode, const char **tessellationEvalCode, const char **geometryCode, const char **pixelCode);
GLuint LinkProgram(GLuint vshader, GLuint pshader);
GLuint LinkProgram(GLuint vshader, GLuint tcshader, GLuint teshader, GLuint gshader, GLuint pshader);
GLuint LinkProgramViaFile(const char *vertexShaderFile, const char *pixelShaderFile);

int CurrentProgram();

// Uniform Access
bool SetUniform(int program, const char *name, int val, bool report = true);
// bool SetUniform(int program, const char *name, GLuint val, bool report = true);
bool SetUniformv(int program, const char *name, int count, int *v, bool report = true);
bool SetUniform(int program, const char *name, float val, bool report = true);
bool SetUniformv(int program, const char *name, int count, float *v, bool report = true);
bool SetUniform(int program, const char *name, vec2 v, bool report = true);
bool SetUniform(int program, const char *name, vec3 v, bool report = true);
bool SetUniform(int program, const char *name, vec4 v, bool report = true);
bool SetUniform(int program, const char *name, vec3 *v, bool report = true);
bool SetUniform(int program, const char *name, vec4 *v, bool report = true);
bool SetUniform3(int program, const char *name, float *v, bool report = true);
bool SetUniform3v(int program, const char *name, int count, float *v, bool report = true);
bool SetUniform4v(int program, const char *name, int count, float *v, bool report = true);
bool SetUniform(int program, const char *name, mat4 m, bool report = true);
	// if no such named uniform and report, print error message

// Attribute Access
int EnableVertexAttribute(int program, const char *name);
	// find named attribute and enable
void DisableVertexAttribute(int program, const char *name);
	// find named attribute and disable
void VertexAttribPointer(int program, const char *name, GLint ncomponents, GLsizei stride, const GLvoid *offset);
	// find and set named attribute, with given number of components, stride between entries, offset into array
	// this calls glAttribPointer with type = GL_FLOAT and normalize = GL_FALSE

#endif // GL_XTRAS_HDR
