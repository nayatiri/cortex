#pragma once

#include <glm/ext/vector_float3.hpp>

class Force_generator {

public:
  glm::vec3 force_to_generate;

  virtual glm::vec3 get_force(glm::vec3 position) = 0;
  virtual glm::vec3 get_force_in_time_interval(float time_interval, glm::vec3 position) = 0;
  
};

class Constant_force_generator : public Force_generator {

public:

  Constant_force_generator(glm::vec3 force);
  
  glm::vec3 get_force(glm::vec3 position);
  glm::vec3 get_force_in_time_interval(float time_interval, glm::vec3 position);
  
};

class Conditional_force_generator : public Force_generator {

public:

  Conditional_force_generator(glm::vec3 force);
  
  glm::vec3 get_force(glm::vec3 position);
  glm::vec3 get_force_in_time_interval(float time_interval,glm::vec3 position);
  
};

