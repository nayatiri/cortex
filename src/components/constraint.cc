#include "constraint.hh"
#include <cmath>
#include <glm/trigonometric.hpp>
#include <memory>

Fix_length_constraint::Fix_length_constraint(std::shared_ptr<Point> from,
                                             std::shared_ptr<Point> to,
                                             float set_distance)
    : point_a(from), point_b(to), distance(set_distance) {};

Fix_angle_constraint::Fix_angle_constraint(std::shared_ptr<Point> end_a,
                                           std::shared_ptr<Point> end_b,
                                           std::shared_ptr<Point> hinge,
                                           float angle)
    : end_a(end_a), end_b(end_b), hinge(hinge), angle(angle) {};

std::shared_ptr<Fix_length_constraint> Fix_angle_constraint::create_length_constraint() {
  
  float dist_h_a = hinge->get_distance_to_other_point(end_a);
  float dist_h_b = hinge->get_distance_to_other_point(end_b);
  
  //ghetto xd
  float need_distance = sqrt( (dist_h_a * dist_h_a) + (dist_h_b * dist_h_b) - 2*dist_h_a*dist_h_b*cos(glm::radians(angle)) );
  
  return std::make_shared<Fix_length_constraint>(end_a,end_b,need_distance);
  
}

void Fix_angle_constraint::update_length_constraint() {

  float dist_h_a = hinge->get_distance_to_other_point(end_a);
  float dist_h_b = hinge->get_distance_to_other_point(end_b);
  
  //ghetto xd
  float need_distance = sqrt( (dist_h_a * dist_h_a) + (dist_h_b * dist_h_b) - 2*dist_h_a*dist_h_b*cos(glm::radians(angle)) );

  result_length_constraint->distance = need_distance;
  
}
