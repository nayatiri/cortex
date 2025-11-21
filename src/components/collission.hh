#pragma once

#include "point.hh"
#include <glm/ext/vector_float3.hpp>
#include <memory>

struct Collission {
public:
  bool resolved = false;
  
  virtual void resolve_collission() = 0;
  
};

struct Point_Point_Collission : public Collission {
  
  std::shared_ptr<Point> point_a;
  std::shared_ptr<Point> point_b;

  glm::vec3 contact_normal = {0,0,0};

  float restitution_coefficient = 1.0f; // how much energy is lost on contact, for now 1 aka no energy lost, 0 is perfectly sticky

  Point_Point_Collission( std::shared_ptr<Point> point_a, std::shared_ptr<Point> point_b, glm::vec3 contact_normal);

  void resolve_collission();
};

struct Point_Plane_Collission : public Collission {

  float restitution_coefficient = 1.0f; // how much energy is lost on contact, for now 1 aka no energy lost, 0 is perfectly sticky
  
  Point_Plane_Collission();

  void resolve_collission();
  
};
