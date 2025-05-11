#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader_old {
public:
  
  Shader_old(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath = "");
  
  void use();
  uint m_shader_id;
  
  void setBool(const std::string& name, bool value) const;
  void setInt(const std::string& name, int value) const;
  void setFloat(const std::string& name, float value) const;
  
private:
  unsigned int ID;
  
  void compile(const char* vertexCode, const char* fragmentCode, const char* geometryCode = nullptr);
};
