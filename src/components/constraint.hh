#pragma once

#include "point.hh"
#include <memory>
struct Constraint {
  bool violated = false;
  virtual ~Constraint() = default;
};

struct Fix_length_constraint : public Constraint {
  std::shared_ptr<Point> point_a;
  std::shared_ptr<Point> point_b;
  float distance;
  Fix_length_constraint(std::shared_ptr<Point> from, std::shared_ptr<Point> to, float set_distance);
};

struct Fix_angle_constraint : public Constraint {

  std::shared_ptr<Point> end_a;
  std::shared_ptr<Point> end_b;
  std::shared_ptr<Point> hinge;
  
  float angle = 90.0f;

  Fix_angle_constraint(  std::shared_ptr<Point> end_a, std::shared_ptr<Point> end_b, std::shared_ptr<Point> hinge, float angle);

  void update_length_constraint();

  std::shared_ptr<Fix_length_constraint> create_length_constraint();
  
};
