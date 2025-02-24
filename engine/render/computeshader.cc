#include "config.h"
#include "ComputeShader.h"

namespace Render
{
	ComputeShader::ComputeShader()
	{}
	ComputeShader::ComputeShader(const char* computePath)
	{
		std::string  computeCode;
		std::ifstream cShaderFile;

		cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			cShaderFile.open(computePath);
			std::stringstream cShaderStream;
			cShaderStream << cShaderFile.rdbuf();
			cShaderFile.close();
			computeCode = cShaderStream.str();
		}
		catch (std::ifstream::failure& e)
		{
			printf("[ Compute Shader ] : ERROR : Shader files did not load.\n");
		}
		const char* cShaderCode = computeCode.c_str();

		unsigned int compute;

		compute = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute, 1, &cShaderCode, NULL);
		glCompileShader(compute);
		checkCompileErrors(compute, "COMPUTE");

		ID = glCreateProgram();
		glAttachShader(ID, compute);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");
		glDeleteShader(compute);
	}


	void ComputeShader::checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: Compute " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
}