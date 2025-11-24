#include "force_generator.hh"
#include "logging.hh"
#include <cmath>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <random>

// glm::vec3 Force_generator::get_force() {};
// glm::vec3 Force_generator::get_force_in_time_interval( float time ) {};

// constructors
Conditional_force_generator::Conditional_force_generator(glm::vec3 force) {
  force_to_generate = force;
}

Constant_force_generator::Constant_force_generator(glm::vec3 force) {
  force_to_generate = force;
}

Drag_force_generator::Drag_force_generator(float coeff_1, float coeff_2) {
  drag_coefficient_1 = coeff_1;
  drag_coefficient_2 = coeff_2;
}

Spring_force_generator::Spring_force_generator(std::shared_ptr<Point> from,
                                               std::shared_ptr<Point> to,
                                               float strength,
                                               float rest_length)
    : from(from), to(to), strength(strength), rest_length(rest_length) {}

Bouyancy_force_generator::Bouyancy_force_generator(glm::vec3 force,
                                                   float height,
                                                   float density) {
  force_to_generate = force;
  medium_height = height;
  medium_density = density;
}

Temporary_force_generator::Temporary_force_generator(std::shared_ptr<Point> to,
						     float strength_n,
                                                     glm::vec3 direction_n)
  : point(to), strength(strength_n), direction(direction_n) {}

Stiff_Spring_force_generator::Stiff_Spring_force_generator(
    std::shared_ptr<Point> from, std::shared_ptr<Point> to, float strength,
    float rest_length)
    : from(from), to(to), strength(strength), rest_length(rest_length) {}

// get force
glm::vec3 Constant_force_generator::get_force(Point &p, float delta_time) {
  if (p.phys_props.inverse_mass == 0.0f)
    return {0.0f, 0.0f, 0.0f};
  float mass = 1.0f / p.phys_props.inverse_mass;
  return force_to_generate * mass;
};

glm::vec3 Conditional_force_generator::get_force(Point &p, float delta_time) {

  // TMP not corrent impl

  if (p.get_position().y < 6) {

    float distance = std::abs(p.get_position().y);

    return force_to_generate * distance;

  } else {
    return {0, 0, 0};
  }
};

glm::vec3 Drag_force_generator::get_force(Point &p, float delta_time) {

  glm::vec3 velocity = p.phys_props.velocity;
  float speed = glm::length(velocity);

  if (speed < 1e-6f)
    return glm::vec3(0.0f);
  if (velocity == glm::vec3(0.0f, 0.0f, 0.0f))
    return glm::vec3(0.0f);

  float drag_coeff =
      drag_coefficient_1 * speed + drag_coefficient_2 * speed * speed;

  glm::vec3 force = -drag_coeff * glm::normalize(velocity);

  return force;
};

glm::vec3 Spring_force_generator::get_force(Point &p, float delta_time) {

  if (&p != from.get() && &p != to.get()) {
    return glm::vec3(0.0f);
  }

  glm::vec3 from_pos = from->get_position();
  glm::vec3 to_pos = to->get_position();
  glm::vec3 displacement = from_pos - to_pos;

  float distance = glm::length(displacement);

  if (distance == 0.0f) {
    static std::mt19937 rng{std::random_device{}()};
    static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    glm::vec3 random_force = glm::vec3(dist(rng), dist(rng), dist(rng));
    return (&p == from.get()) ? random_force : -random_force;
  }

  glm::vec3 direction_to = glm::normalize(displacement);

  float delta = distance - rest_length;

  float force_magnitude = -strength * delta;

  glm::vec3 force_on_from = direction_to * force_magnitude;
  glm::vec3 force_on_to = -force_on_from;

  return (&p == from.get()) ? force_on_from : force_on_to;
}

glm::vec3 Bouyancy_force_generator::get_force(Point &p, float delta_time) {

  glm::vec3 force = {0, 0, 0};

  float depth = p.get_position().y;

  // float max_submersible_depth = p.phys_props.radius * 2;

  // are we outside the medium?
  if (depth - p.phys_props.radius >= medium_height)
    return force;

  // are we fully submerged?
  if (depth + p.phys_props.radius <= medium_height) {
    force.y =
        medium_density * p.phys_props.get_volume() * -p.phys_props.gravity;
    return force;
  }

  // Partially submerged case:
  float radius = p.phys_props.radius;
  float bottom_y = depth - radius;
  float h = medium_height - bottom_y;

  // Clamp h between 0 and 2*radius
  h = glm::clamp(h, 0.0f, 2.0f * radius);

  // Spherical cap volume
  float submerged_vol = M_PI * h * h * (3.0f * radius - h) / 3.0f;

  force.y = medium_density * submerged_vol * (-p.phys_props.gravity);

  return force;
};

glm::vec3 Stiff_Spring_force_generator::get_force(Point &p, float delta_time) {

  // TODO shit calculation, cant find resource for stable calculation for 2
  // loose points all lit just does fucking 1 anchor 1 loose type shi

  // check for inf mass
  if (p.phys_props.inverse_mass == 0)
    return {0, 0, 0};

  //  glm::vec3 position = p.get_position();

  return {0, 0, 0};
};


glm::vec3 Temporary_force_generator::get_force(Point &p, float delta_time) {

  //TODO make proper impl
  if (p.phys_props.inverse_mass == 0.0f)
    return {0.0f, 0.0f, 0.0f};
  
  float mass = 1.0f / p.phys_props.inverse_mass;

  //std::cout << "applied force: " << (direction * mass).x << ","  << (direction * mass).y << "," << (direction * mass).z << std::endl;
  
  force_applied = true;
  
  return direction * mass;
  
}
