#include "force_generator.hh"
#include <cmath>
#include <cstdlib>

//glm::vec3 Force_generator::get_force() {};
//glm::vec3 Force_generator::get_force_in_time_interval( float time ) {};

//constructors
Conditional_force_generator::Conditional_force_generator(glm::vec3 force) {
  force_to_generate = force;
}

Constant_force_generator::Constant_force_generator(glm::vec3 force) {
  force_to_generate = force;
}


//get force
glm::vec3 Constant_force_generator::get_force(glm::vec3 position) {
  return force_to_generate;
};

glm::vec3 Conditional_force_generator::get_force(glm::vec3 position) {

  //TMP not corrent impl

  if(position.y < 0) {

    float distance = std::abs(position.y);
    
    return force_to_generate * distance;

    } else { 
    return {0,0,0};
  }
};


// get in time
glm::vec3 Constant_force_generator::get_force_in_time_interval(float time_interval, glm::vec3 position) {
  //TMP not correct impl
  return force_to_generate;
};

glm::vec3 Conditional_force_generator::get_force_in_time_interval(float time,glm::vec3 position) {
  //TMP not correct impl
  return force_to_generate;
};
