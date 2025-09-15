#include "force_generator.hh"
#include "logging.hh"
#include <cmath>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <random>

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

  if(p.get_position().y < 6) {

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

  //TODO make this mess not have 0(p*n2) xd
  
  if (&p == from.get()) {
    
    glm::vec3 from_to = from->get_position() - to->get_position();
    
    float distance = glm::length(from_to);

    if (distance == 0.0f) {
      static std::mt19937 rng{std::random_device{}()};
      static std::uniform_real_distribution<float> dist(-1.0f, 1.0f); 
      glm::vec3 random_force = glm::vec3(dist(rng), dist(rng), dist(rng));
      return random_force;
    }
    
    glm::vec3 spring_direction_from_to = glm::normalize(from_to);

    // if distance smaller than rest_length we need to extend the spring
    if(distance < rest_length) {
      return spring_direction_from_to * (rest_length - distance) * strength;
    }

    // if distance bigger, contract the spring
    if(distance > rest_length) {
      return -spring_direction_from_to * (distance - rest_length) * strength;
    }

  }

  if (&p == to.get()) {
    
    glm::vec3 from_to = to->get_position() - from->get_position();
    
    float distance = glm::length(from_to);

    if (distance == 0.0f) {
      static std::mt19937 rng{std::random_device{}()};
      static std::uniform_real_distribution<float> dist(-1.0f, 1.0f); 
      glm::vec3 random_force = glm::vec3(dist(rng), dist(rng), dist(rng));
      return random_force;
    }

    glm::vec3 spring_direction_from_to = glm::normalize(from_to);

    // if distance smaller than rest_length we need to extend the spring
    if(distance < rest_length) {
      return spring_direction_from_to * (rest_length - distance) * strength;
    }

    // if distance bigger, contract the spring
    if(distance > rest_length) {
      return -spring_direction_from_to * (distance - rest_length) * strength;
    }

  }

  return glm::vec3(0.0f,0.0f,0.0f);
  
};
