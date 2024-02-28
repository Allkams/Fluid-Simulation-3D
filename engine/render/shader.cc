// 
// Copyright 2023 Alexander Marklund (Allkams02@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand associated
// documentation files(the “Software”), to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and /or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN 
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
// THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "config.h"
#include "shader.h"

Shader::Shader(const char* vsPath, const char* fsPath) : vsPath(vsPath), fsPath(fsPath)
{
	LoadShader(vsPath, fsPath);
}

void Shader::LoadShader(const char* vsPath, const char* fsPath)
{
	std::string vsCode;
	std::string fsCode;

	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		vShaderFile.open(vsPath);
		fShaderFile.open(fsPath);

		std::stringstream vShaderStream, fShaderSteam;

		vShaderStream << vShaderFile.rdbuf();
		fShaderSteam << fShaderFile.rdbuf();

		vShaderFile.close();
		fShaderFile.close();

		vsCode = vShaderStream.str();
		fsCode = fShaderSteam.str();
	}
	catch (std::ifstream::failure e)
	{
		printf("[ Shader ] : ERROR : Shader files did not load.\n");
		assert(false);
	}

	const char* vShaderCode = vsCode.c_str();
	const char* fShaderCode = fsCode.c_str();

	GLuint vsID, fsID;
	int success;
	char info[512];

	vsID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vsID, 1, &vShaderCode, NULL);
	glCompileShader(vsID);

	glGetShaderiv(vsID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vsID, 512, NULL, info);
		printf("[ Shader ] : ERROR : Compilation failure for vertex shader. \n Error log: %s\n", info);
		assert(false);
	}

	fsID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fsID, 1, &fShaderCode, NULL);
	glCompileShader(fsID);

	glGetShaderiv(fsID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fsID, 512, NULL, info);
		printf("[ Shader ] : ERROR : Compilation failure for fragment shader. \n Error log: %s\n", info);
		assert(false);
	}

	this->ID = glCreateProgram();
	glAttachShader(ID, vsID);
	glAttachShader(ID, fsID);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, info);
		printf("[ Shader ] : ERROR : Linking failure for shader program. \n Error log: %s\n", info);
		assert(false);
	}
	
	glDeleteShader(vsID);
	glDeleteShader(fsID);
}

void Shader::ReloadShader()
{
	this->Destroy();
	this->LoadShader(this->vsPath.c_str(), this->fsPath.c_str());
}

void Shader::Destroy()
{
	glDeleteProgram(this->ID);
	this->ID = 0xFFFFFFFF;
}

void Shader::Enable()
{
	//printf("Program %i is active\n", (int)this->ID);
	glUseProgram(this->ID);
}

void Shader::Disable()
{
	//printf("Program %i is deactivated\n", (int)this->ID);
	glUseProgram(0);
}
GLuint Shader::GetProgram()
{
	return this->ID;
}


void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(this->ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{

}
void Shader::setFloat(const std::string& name, float value) const
{

	glUniform1f(glGetUniformLocation(this->ID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, glm::vec2 value) const
{
	glUniform2f(glGetUniformLocation(this->ID, name.c_str()), value.x, value.y);
}
void Shader::setVec3(const std::string& name, glm::vec3 value) const
{

}
void Shader::setVec4(const std::string& name, glm::vec4 value) const
{
	glUniform4f(glGetUniformLocation(this->ID, name.c_str()), value.x, value.y, value.z, value.w);
}

void Shader::setMat4(const std::string& name, glm::mat4 value) const
{
	glUniformMatrix4fv(glGetUniformLocation(this->ID, name.c_str()), 1, GL_FALSE, &value[0][0]);
}