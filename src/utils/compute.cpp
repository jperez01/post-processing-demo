#include "compute.h"

#include <fstream>
#include <sstream>
#include <iostream>

ComputeShader::ComputeShader() {}

ComputeShader::ComputeShader(std::string computePath) {
    std::string generalPath = "../../shaders/";
    std::string fullPath = generalPath + computePath;
    std::string computeCode;
    std::ifstream computeFile;

    computeFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        computeFile.open(fullPath);
        std::stringstream computeStream;

        computeStream << computeFile.rdbuf();
        computeFile.close();

        computeCode = computeStream.str();
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    unsigned int compute;

    const char* cShaderCode = computeCode.c_str();
    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &cShaderCode, nullptr);
    glCompileShader(compute);
    checkCompileErrors(compute, "COMPUTE");

    ID = glCreateProgram();
    glAttachShader(ID, compute);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    glDeleteShader(compute);
}

void ComputeShader::use() {
    glUseProgram(ID);
}

void ComputeShader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
// ------------------------------------------------------------------------
void ComputeShader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
// ------------------------------------------------------------------------
void ComputeShader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}
// ------------------------------------------------------------------------
void ComputeShader::setVec2(const std::string &name, const glm::vec2 &value) const
{ 
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void ComputeShader::setVec2(const std::string &name, float x, float y) const
{ 
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
}
// ------------------------------------------------------------------------
void ComputeShader::setVec3(const std::string &name, const glm::vec3 &value) const
{ 
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void ComputeShader::setVec3(const std::string &name, float x, float y, float z) const
{ 
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
}
// ------------------------------------------------------------------------
void ComputeShader::setVec4(const std::string &name, const glm::vec4 &value) const
{ 
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void ComputeShader::setVec4(const std::string &name, float x, float y, float z, float w) 
{ 
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
}
// ------------------------------------------------------------------------
void ComputeShader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void ComputeShader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void ComputeShader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
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
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}