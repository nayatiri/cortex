#pragma once

#include "point.hh"
#include <glm/ext/vector_float3.hpp>
#include <memory>

class Force_generator {
public:
  glm::vec3 force_to_generate;
  virtual glm::vec3 get_force(Point& p, float delta_time) = 0;
  
};

class Constant_force_generator : public Force_generator {
public:
  Constant_force_generator(glm::vec3 force);
  glm::vec3 get_force(Point& p, float delta_time);
};

class Conditional_force_generator : public Force_generator {
public:

  Conditional_force_generator(glm::vec3 force);
  glm::vec3 get_force(Point& p, float delta_time);
};

class Drag_force_generator : public Force_generator {
private:
  float drag_coefficient_1 = 0.1f;
  float drag_coefficient_2 = 0.01f;
public:
  Drag_force_generator(float drag_coefficient_1, float drag_coefficient_2);
  glm::vec3 get_force(Point& p, float delta_time);
};

class Spring_force_generator : public Force_generator {
private:
  std::shared_ptr<Point> from = nullptr;
  std::shared_ptr<Point> to = nullptr;
  float strength;
  float rest_length;
public:
  Spring_force_generator(std::shared_ptr<Point> to, std::shared_ptr<Point> from, float strength, float rest_length);
  glm::vec3 get_force(Point& p, float delta_time);
};

class Bouyancy_force_generator : public Force_generator {
private:
  float medium_height;
  float medium_density;
  
public:

  Bouyancy_force_generator(glm::vec3 force, float height, float density);
  glm::vec3 get_force(Point& p, float delta_time);
};
