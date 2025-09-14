#include "force_generator.hh"
#include "logging.hh"
#include <cmath>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>

//glm::vec3 Force_generator::get_force() {};
//glm::vec3 Force_generator::get_force_in_time_interval( float time ) {};

//constructors
Conditional_force_generator::Conditional_force_generator(glm::vec3 force) {
  force_to_generate = force;
}

Constant_force_generator::Constant_force_generator(glm::vec3 force) {
  force_to_generate = force;
}

Drag_force_generator::Drag_force_generator(float coeff_1, float coeff_2 ) {
  drag_coefficient_1 = coeff_1;
  drag_coefficient_2 = coeff_2;
}

Spring_force_generator::Spring_force_generator(std::shared_ptr<Point> from, std::shared_ptr<Point> to, float strength, float rest_length) : from(from),
																	    to(to),
																	    strength(strength),
																	    rest_length(rest_length) {}

//get force
glm::vec3 Constant_force_generator::get_force(Point& p, float delta_time) {
  return force_to_generate;
};

glm::vec3 Conditional_force_generator::get_force(Point& p, float delta_time) {

  //TMP not corrent impl

  if(p.get_position().y < 0) {

    float distance = std::abs(p.get_position().y);
    
    return force_to_generate * distance;

    } else { 
    return {0,0,0};
  }
};

glm::vec3 Drag_force_generator::get_force(Point& p, float delta_time) {

    glm::vec3 velocity = p.phys_props.velocity;
    float speed = glm::length(velocity);
    
    if (speed < 1e-6f)
      return glm::vec3(0.0f);
    if (velocity == glm::vec3(0.0f,0.0f,0.0f))
      return glm::vec3(0.0f);
    
    float drag_coeff = drag_coefficient_1 * speed + drag_coefficient_2 * speed * speed;

    glm::vec3 force = -drag_coeff * glm::normalize(velocity);

    return force;

};

glm::vec3 Spring_force_generator::get_force(Point& p, float delta_time) {
  return glm::vec3(0.0f,0.0f,0.0f);
};
