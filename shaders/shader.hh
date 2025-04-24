#pragma once

#include <GL/glew.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:

    Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
  
    void use();

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;

private:
    unsigned int ID;

    void compile(const char* vertexCode, const char* fragmentCode, const char* geometryCode = nullptr);
};
