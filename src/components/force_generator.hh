#pragma once

#include "point.hh"
#include <glm/ext/vector_float3.hpp>
#include <memory>

class Force_generator {
public:
  glm::vec3 force_to_generate = glm::vec3(0,0,0);
  virtual glm::vec3 get_force(Point& p, float delta_time) = 0;
  
};

// constant forces of any kind, gravity, wind, etc.
class Constant_force_generator : public Force_generator {
public:
  Constant_force_generator(glm::vec3 force);
  glm::vec3 get_force(Point& p, float delta_time);
};

// conditional forces, that can have a hardcoded trigger
class Conditional_force_generator : public Force_generator {
public:

  Conditional_force_generator(glm::vec3 force);
  glm::vec3 get_force(Point& p, float delta_time);
};

// drag simulation using linear and exponential components - 0.1 exp, 0.05 lin reccommended 
class Drag_force_generator : public Force_generator {
private:
  float drag_coefficient_1 = 0.05f;
  float drag_coefficient_2 = 0.008f;
public:
  Drag_force_generator(float drag_coefficient_1, float drag_coefficient_2);
  glm::vec3 get_force(Point& p, float delta_time);
};

// weak springs (stiffness 0e-6 - 100)
class Spring_force_generator : public Force_generator {
private:
  std::shared_ptr<Point> from = nullptr;
  std::shared_ptr<Point> to = nullptr;
  float strength = 50.0f;
  float rest_length = 5.0f;
public:
  Spring_force_generator(std::shared_ptr<Point> to, std::shared_ptr<Point> from, float strength, float rest_length);
  glm::vec3 get_force(Point& p, float delta_time);
};

// bouyancy simulation for liquid, gravity affected mediae
class Bouyancy_force_generator : public Force_generator {
private:
  float medium_height = 0.0f;
  float medium_density = 1000.0f;
public:
  Bouyancy_force_generator(glm::vec3 force, float height, float density);
  glm::vec3 get_force(Point& p, float delta_time);
};

// stiff springs (stiffness 100 - inf)
class Stiff_Spring_force_generator : public Force_generator {
private:
  std::shared_ptr<Point> from = nullptr;
  std::shared_ptr<Point> to = nullptr;
  float strength = 200.0f;
  float rest_length = 5.0f;
public:
  Stiff_Spring_force_generator(std::shared_ptr<Point> to, std::shared_ptr<Point> from, float strength, float rest_length);
  glm::vec3 get_force(Point& p, float delta_time);
};

// one time force generator, used to apply an impulse to an object
class Impulse_force_generator : public Force_generator {
private:
  glm::vec3 impulse;
public:
  std::shared_ptr<Point> point = nullptr;
  bool force_applied = false;

  Impulse_force_generator(  std::shared_ptr<Point> to, glm::vec3 impulse );
  glm::vec3 get_force(Point& p, float delta_time);
};
