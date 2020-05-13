#ifndef _VR_3D_SHADER_H_
#define _VR_3D_SHADER_H_
//����C++��׼ͷ�ļ�
#include <iostream>
#include <vector>
#include <fstream>		//for ofstream

// ����GLEW�� ���徲̬����
#define GLEW_STATIC
#include "GL/glew.h"

struct ShaderFile
{
	GLenum shaderType;
	const char* filePath;
	ShaderFile(GLenum type, const char* path)
		:shaderType(type), filePath(path) {}
};

class vrshader
{
public:
	~vrshader()
	{
		if (this->programId)
		{
			glDeleteProgram(this->programId);
		}
	}
	vrshader(const char* vertexPath, const char* fragPath, bool ispath = true) :programId(0), isPath(ispath)
	{
		std::vector<ShaderFile> fileVec;
		fileVec.push_back(ShaderFile(GL_VERTEX_SHADER, vertexPath));
		fileVec.push_back(ShaderFile(GL_FRAGMENT_SHADER, fragPath));
		loadFromFile(fileVec);
	}
	vrshader(const char* vertexPath, const char* fragPath, const char* geometryPath) :programId(0)
	{
		std::vector<ShaderFile> fileVec;
		fileVec.push_back(ShaderFile(GL_VERTEX_SHADER, vertexPath));
		fileVec.push_back(ShaderFile(GL_FRAGMENT_SHADER, fragPath));
		fileVec.push_back(ShaderFile(GL_GEOMETRY_SHADER, geometryPath));
		loadFromFile(fileVec);
	}

	// Uses the current shader
	void vr_shader_bind()
	{
		glUseProgram(programId);
	}

	GLint vr_get_attribute_location(const GLchar* name)
	{
		return glGetAttribLocation(programId, name);
	}

	GLint vr_get_uniform_location(const GLchar* name)
	{
		return glGetUniformLocation(programId, name);
	}

	void vr_shader_update_uniform_1i(const GLchar* name, GLint i)
	{
		GLint location = glGetUniformLocation(programId, name);
		glUniform1i(location, i); // ��������ԪΪi��
	}

	void vr_shader_update_uniform_4f(const GLchar* name, GLfloat f0, GLfloat f1, GLfloat f2, GLfloat f3)
	{
		GLint location = glGetUniformLocation(programId, name);
		glUniform4f(location, f0, f1, f2, f3);
	}

	void vr_shader_update_uniform_1f(const GLchar* name, GLfloat i)
	{
		GLint location = glGetUniformLocation(programId, name);
		glUniform1f(location, i);
	}

	void vr_shader_upload_matrix(const GLchar* name, const GLfloat* value)
	{
		GLint transformLoc = glGetUniformLocation(programId, name);
		glUniformMatrix4fv(transformLoc, //uniform��λ��ֵ
			1,			//����OpenGL���ǽ�Ҫ���Ͷ��ٸ�����
			GL_FALSE,	//�Ƿ�ϣ�������ǵľ�������û�(Transpose)��Ҳ����˵�������Ǿ�����к���
			value);		//�����ľ�������
	}

protected:
private:
	GLuint programId;
	bool isPath;

private:
	/*
	* ���ļ����ض����ƬԪ��ɫ��
	* ���ݲ���Ϊ [(��ɫ���ļ����ͣ���ɫ���ļ�·��)+]
	*/
	void loadFromFile(std::vector<ShaderFile>& shaderFileVec)
	{
		std::vector<GLuint> shaderObjectIdVec;
		std::vector<std::string> sourceVec;
		size_t shaderCount = shaderFileVec.size();
		// ��ȡ�ļ�Դ����
		for (size_t i = 0; i < shaderCount; ++i)
		{
			std::string shaderSource;
			if (!loadShaderSource(shaderFileVec[i].filePath, shaderSource))
			{
				std::cout << "Error::Shader could not load file:" << shaderFileVec[i].filePath << std::endl;
				return;
			}
			sourceVec.push_back(shaderSource);
		}
		bool bSuccess = true;
		// ����shader object
		for (size_t i = 0; i < shaderCount; ++i)
		{
			GLuint shaderId = glCreateShader(shaderFileVec[i].shaderType);
			const char* c_str = sourceVec[i].c_str();
			glShaderSource(shaderId, 1, &c_str, NULL);
			glCompileShader(shaderId);
			GLint compileStatus = 0;
			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus); // ������״̬
			if (compileStatus == GL_FALSE) // ��ȡ���󱨸�
			{
				GLint maxLength = 0;
				glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> errLog(maxLength);
				glGetShaderInfoLog(shaderId, maxLength, &maxLength, &errLog[0]);
				std::cout << "Error::Shader file [" << shaderFileVec[i].filePath << " ] compiled failed,"
					<< &errLog[0] << std::endl;
				bSuccess = false;
			}
			shaderObjectIdVec.push_back(shaderId);
		}
		// ����shader program
		if (bSuccess)
		{
			this->programId = glCreateProgram();
			for (size_t i = 0; i < shaderCount; ++i)
			{
				glAttachShader(this->programId, shaderObjectIdVec[i]);
			}
			glLinkProgram(this->programId);
			GLint linkStatus;
			glGetProgramiv(this->programId, GL_LINK_STATUS, &linkStatus);
			if (linkStatus == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetProgramiv(this->programId, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> errLog(maxLength);
				glGetProgramInfoLog(this->programId, maxLength, &maxLength, &errLog[0]);
				std::cout << "Error::shader link failed," << &errLog[0] << std::endl;
			}
		}
		// ������ɺ�detach ���ͷ�shader object
		for (size_t i = 0; i < shaderCount; ++i)
		{
			if (this->programId != 0)
			{
				glDetachShader(this->programId, shaderObjectIdVec[i]);
			}
			glDeleteShader(shaderObjectIdVec[i]);
		}
	}

	/*
	* ��ȡ��ɫ������Դ��
	*/
	bool loadShaderSource(const char* filePath, std::string& source)
	{
		source.clear();
		if (isPath) 
		{
			std::ifstream in_stream(filePath);
			if (!in_stream)
			{
				return false;
			}
			source.assign(std::istreambuf_iterator<char>(in_stream),
				std::istreambuf_iterator<char>()); // �ļ��������������ַ���
		}
		else
		{
			source.assign(filePath);
		}
		
		return true;
	}

};

#endif // !_VR_3D_SHADER_H_


