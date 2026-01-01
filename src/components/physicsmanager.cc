#include "physicsmanager.hh"
#include "AABB.hh"
#include "collission.hh"
#include "constraint.hh"
#include "entity.hh"
#include "force_generator.hh"
#include "logging.hh"
#include "point.hh"
#include <algorithm>
#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <random>

#define STABILITY_THRESHOLD 0.02f
#define WIGGLE_ROOM 0.05f
#define MAX_SOLVER_ITERATIONS 100
#define CONVERGENCE_RELAXATION 0.4f
#define PI_CONST 3.41

Physics_Manager::Physics_Manager(std::shared_ptr<Scene> set_scene) {
  m_active_scene = set_scene;
};

AABB Physics_Manager::compute_world_space_aabb(Mesh &mesh,
                                               const glm::mat4 &transform) {

  AABB bbox{{10000.0f, 10000.0f, 10000.0f}, {-10000.0f, -10000.0f, -10000.0f}};

  const glm::mat4 mesh_transform = transform * mesh.get_model_matrix();

  for (size_t i = 0; i + 2 < mesh.m_vertices_array.size(); i += 3) {
    glm::vec4 vertex{mesh.m_vertices_array[i], mesh.m_vertices_array[i + 1],
                     mesh.m_vertices_array[i + 2], 1.0f};

    vertex = mesh_transform * vertex;

    bbox.min.x = std::min(bbox.min.x, vertex.x);
    bbox.min.y = std::min(bbox.min.y, vertex.y);
    bbox.min.z = std::min(bbox.min.z, vertex.z);

    bbox.max.x = std::max(bbox.max.x, vertex.x);
    bbox.max.y = std::max(bbox.max.y, vertex.y);
    bbox.max.z = std::max(bbox.max.z, vertex.z);
  }

  // adjust bbox with entity transform
  /*
  bbox.max = transform * glm::vec4(bbox.max,1.0f);
  bbox.min = transform * glm::vec4(bbox.min,1.0f);
  */

  return bbox;
}

bool Physics_Manager::check_violated_static_constraints() {

  bool violated = false;

  for (const std::shared_ptr<Constraint> &c :
       m_active_scene->m_loaded_constraints) {

    // check which kind of constraint we have.
    if (const std::shared_ptr<Fix_length_constraint> &lc =
            std::dynamic_pointer_cast<Fix_length_constraint>(c)) {

      // TODO make this shit a function or find better way lol actually just run
      // this in a for loop for all points in scene at start of run_integrator
      // ngl...
      if (lc->point_a->get_position() == lc->point_b->get_position()) {
        static std::mt19937 rng{std::random_device{}()};
        static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        glm::vec3 random_force = glm::vec3(dist(rng), dist(rng), dist(rng));
        lc->point_a->change_position(random_force * 0.1f);
        lc->point_b->change_position(random_force * -0.1f);
      }

      float error = glm::distance(lc->point_a->get_position(),
                                  lc->point_b->get_position()) -
                    lc->distance;
      if (error > WIGGLE_ROOM || error < -WIGGLE_ROOM) {
        std::cout << "length constraint violated with error:" << error << "\n";
        lc->violated = true;
        violated = true;
      }
    }

    if (const std::shared_ptr<Fix_angle_constraint> &ac =
            std::dynamic_pointer_cast<Fix_angle_constraint>(c)) {

      // TODO mb need on hinge too?
      if (ac->end_a->get_position() == ac->end_b->get_position()) {
        static std::mt19937 rng{std::random_device{}()};
        static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        glm::vec3 random_force = glm::vec3(dist(rng), dist(rng), dist(rng));
        ac->end_a->change_position(random_force * 0.05f);
        ac->end_b->change_position(random_force * -0.05f);
      }

      glm::vec3 h_to_a = ac->end_a->get_position() - ac->hinge->get_position();
      glm::vec3 h_to_b = ac->end_b->get_position() - ac->hinge->get_position();
      float current_angle = std::acos(
          glm::clamp(glm::dot(glm::normalize(h_to_a), glm::normalize(h_to_b)),
                     -1.0f, 1.0f));
      float error = current_angle - (ac->angle / (180.0f / PI_CONST));
      if (std::abs(error) > WIGGLE_ROOM) {
        ac->violated = true;
        violated = true;
      }
    }
  }

  return violated;
};

glm::vec3 Physics_Manager::rotate_around_axis(glm::vec3 point, glm::vec3 pivot,
                                              glm::vec3 axis, float angle_rad) {
  glm::vec3 v = point - pivot;

  float cos_t = cos(angle_rad);
  float sin_t = sin(angle_rad);

  // eodrigues rotation formula
  glm::vec3 axis_normalized = glm::normalize(axis);
  glm::vec3 v_rot =
      v * cos_t + glm::cross(axis_normalized, v) * sin_t +
      axis_normalized * glm::dot(axis_normalized, v) * (1.0f - cos_t);

  return pivot + v_rot;
}

void Physics_Manager::solve_violated_static_constraints() {

  for (const std::shared_ptr<Constraint> &c :
       m_active_scene->m_loaded_constraints) {

    // skip fine constraints
    if (!c->violated)
      continue;

    //////////////////
    // fix length constraints.
    //////////////////
    if (const std::shared_ptr<Fix_length_constraint> &lc =
            std::dynamic_pointer_cast<Fix_length_constraint>(c)) {

      float error = glm::distance(lc->point_a->get_position(),
                                  lc->point_b->get_position()) -
                    lc->distance;

      glm::vec3 a_to_b = glm::normalize(lc->point_b->get_position() -
                                        lc->point_a->get_position());
      glm::vec3 b_to_a = glm::normalize(lc->point_a->get_position() -
                                        lc->point_b->get_position());

      // both free or both fixed
      if ((!lc->point_a->phys_props.fixed && !lc->point_b->phys_props.fixed) ||
          (lc->point_a->phys_props.fixed && lc->point_b->phys_props.fixed)) {
        a_to_b *= (error / 2);
        b_to_a *= (error / 2);
      } else if (lc->point_a->phys_props.fixed &&
                 !lc->point_b->phys_props.fixed) {
        a_to_b *= (0);
        b_to_a *= (error);
      } else if (!lc->point_a->phys_props.fixed &&
                 lc->point_b->phys_props.fixed) {
        a_to_b *= (error);
        b_to_a *= (0);
      }

      lc->point_a->change_position(a_to_b * CONVERGENCE_RELAXATION);
      lc->point_b->change_position(b_to_a * CONVERGENCE_RELAXATION);
      lc->violated = false;
    }

    //////////////////
    // fix angle constraints
    //////////////////
    if (const std::shared_ptr<Fix_angle_constraint> &ac =
            std::dynamic_pointer_cast<Fix_angle_constraint>(c)) {

      glm::vec3 h_to_a = ac->end_a->get_position() - ac->hinge->get_position();
      glm::vec3 h_to_b = ac->end_b->get_position() - ac->hinge->get_position();
      float ac_angle_rad = ac->angle / (180.0f / PI_CONST);
      float current_angle = std::acos(
          glm::clamp(glm::dot(glm::normalize(h_to_a), glm::normalize(h_to_b)),
                     -1.0f, 1.0f));
      glm::vec3 hinge_normal = glm::normalize(glm::cross(h_to_b, h_to_a));

      float correct_rad = current_angle - ac_angle_rad;
      if (std::abs(correct_rad) > WIGGLE_ROOM) {
        float correct_a = -correct_rad;
        float correct_b = correct_rad;

        // check which end is fixe
        if (!ac->end_a->phys_props.fixed && !ac->end_b->phys_props.fixed) {
          correct_a *= 0.5f;
          correct_b *= 0.5f;
        } else if (ac->end_a->phys_props.fixed &&
                   !ac->end_b->phys_props.fixed) {
          correct_a = 0.0f;
        } else if (!ac->end_a->phys_props.fixed &&
                   ac->end_b->phys_props.fixed) {
          correct_b = 0.0f;
        }

        // write positions to points
        glm::vec3 new_pos_a = rotate_around_axis(
            ac->end_a->get_position(), ac->hinge->get_position(), hinge_normal,
            correct_a * CONVERGENCE_RELAXATION);
        ac->end_a->set_position(new_pos_a.x, new_pos_a.y, new_pos_a.z);
        glm::vec3 new_pos_b = rotate_around_axis(
            ac->end_b->get_position(), ac->hinge->get_position(), hinge_normal,
            correct_b * CONVERGENCE_RELAXATION);
        ac->end_b->set_position(new_pos_b.x, new_pos_b.y, new_pos_b.z);
      }
      ac->violated = false;
    }
  }
};

bool Physics_Manager::check_and_build_collissions() {

  bool found_collissions = false;

  for (std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {
    for (std::shared_ptr<Point> q : m_active_scene->m_loaded_points) {

      if (p == q)
        continue;

      // collission detected?
      if (p->phys_props.radius + q->phys_props.radius >
          p->get_distance_to_other_point(q) + WIGGLE_ROOM) {

        /*
          std::cout << "collission detected with pq rad: " <<
          p->phys_props.radius + q->phys_props.radius <<
          " and dist: " << p->get_distance_to_other_point(q) <<
          " n p q pos " << p->get_position().y <<
          " " << q->get_position().y << "\n";
        */

        found_collissions = true;

        if (!p->phys_props.involved_in_collission &&
            !q->phys_props.involved_in_collission) {
          m_active_scene->m_current_collissions.push_back(
              std::make_shared<Point_Point_Collission>(
                  p, q, glm::normalize(p->get_position() - q->get_position())));
          p->phys_props.involved_in_collission = true;
          q->phys_props.involved_in_collission = true;
        } else {
          // log_error("tried to create collission for points, that already have
          // a collission... not making a new one.");
        }
      }
    }
  }

  return found_collissions;
}

void Physics_Manager::update_phys_box(Mesh &mesh, Entity &parent) {

  // TODO abyssmally inefficient, i know horrendous overhead crazy shit impl,
  // ill fix this when it becomes a problem xd
  AABB bbox = compute_world_space_aabb(mesh, parent.get_model_matrix());
  mesh.AABB_visualizer = std::make_shared<AABB_Box>(bbox.min, bbox.max);

  mesh.phys_box_needs_recalculation = false;
};

void Physics_Manager::resolve_collissions_for_scene_preview() {
  if (m_active_scene->m_current_collissions.empty())
    return;

  for (std::shared_ptr<Collission> c : m_active_scene->m_current_collissions) {

    // handle point point cols
    std::shared_ptr<Point_Point_Collission> ppc =
        std::dynamic_pointer_cast<Point_Point_Collission>(c);

    if (ppc) {
      float distance_should_be =
          ppc->point_a->phys_props.radius + ppc->point_b->phys_props.radius;
      float distance_is =
          ppc->point_a->get_distance_to_other_point(ppc->point_b);

      float distance_adjust = ((distance_should_be - distance_is) / 2);

      // std::cout << "adjusting points with offset: " << distance_adjust <<
      // "\n";

      // points in same location? random direction move
      if (ppc->point_a->get_position() == ppc->point_b->get_position()) {
        static std::mt19937 rng{std::random_device{}()};
        static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        glm::vec3 random_force = glm::vec3(dist(rng), dist(rng), dist(rng));
        ppc->point_a->change_position(random_force * distance_adjust);
        ppc->point_b->change_position(random_force * -distance_adjust);
        continue;
      }

      // solve collission
      ppc->point_a->change_position(ppc->contact_normal * distance_adjust);
      ppc->point_b->change_position(ppc->contact_normal * -distance_adjust);

      // compute relative velocity:
      glm::vec3 rv =
          ppc->point_a->phys_props.velocity - ppc->point_b->phys_props.velocity;
      float velAlongNormal = dot(rv, ppc->contact_normal);

      // hardcode restitution + impulse scalar
      float e = 0.5f;
      float j = -(1 + e) * velAlongNormal /
                (ppc->point_a->phys_props.inverse_mass +
                 ppc->point_b->phys_props.inverse_mass);
      glm::vec3 J = j * ppc->contact_normal;

      // Add impulse through force generators:
      m_active_scene->m_loaded_force_generators.push_back(
          std::make_shared<Impulse_force_generator>(ppc->point_a, J));
      m_active_scene->m_loaded_force_generators.push_back(
          std::make_shared<Impulse_force_generator>(ppc->point_b, -J));
    }

    c->resolved = true;
  }

  // clear all collissions
  for (auto p : m_active_scene->m_loaded_points) {
    p->phys_props.involved_in_collission = false;
  }
  m_active_scene->m_current_collissions.clear();
};

void Physics_Manager::calculate_phys_boxes() {

  std::cout << "Scene contains entities: "
            << m_active_scene->m_loaded_entities.size() << std::endl;

  for (Entity &entity : m_active_scene->m_loaded_entities) {

    glm::mat4 entity_transform = entity.get_model_matrix();
    std::cout << "Entity contains meshes: " << entity.m_mesh.size()
              << std::endl;

    for (std::shared_ptr<Mesh> &mesh : entity.m_mesh) {

      if (mesh->m_mesh_type != E_MESH) {
        log_error("Mesh does'nt seem to be a Mesh lol. skipping.");
        continue;
      };

      if (mesh->m_vertices_array.size() < 3) {
        log_error("Attempted to create hitbox for mesh with insufficient "
                  "vertices (less than 3). Skipping.");
        continue;
      }

      AABB bbox = compute_world_space_aabb(*mesh, entity_transform);

      mesh->AABB_visualizer = std::make_shared<AABB_Box>(bbox.min, bbox.max);
      mesh->phys_box_needs_recalculation = false;

      m_active_scene->m_scene_vbos_need_refresh = true;

      log_success("Calculated hitbox for mesh, and added it to Mesh!");
    }
  }

  m_active_scene->m_scene_vbos_need_refresh = true;
}

void Physics_Manager::handle_scene_physics() {

  // create AABB boxes for our scene, if they havent been already created
  if (!m_phys_boxes_initialized) {
    calculate_phys_boxes();
    m_phys_boxes_initialized = true;
  }

  // if the scene doesnt have any force generators, setup force generators
  if (!m_force_generators_initialized &&
      m_active_scene->m_loaded_points.size() > 0) {
    bool success = initialize_force_generators();
    if (success)
      m_force_generators_initialized = true;
    else
      m_force_generators_initialized = false;
  }

  std::cout << m_active_scene->m_scene_deltatime << "\n";

  // if we have low fps, run smaller integration steps, to prevent numerical
  // instability
  if (m_active_scene->m_scene_deltatime < STABILITY_THRESHOLD)
    run_integrator(m_active_scene->m_scene_deltatime);
  else {
    unsigned int num_steps =
        m_active_scene->m_scene_deltatime / STABILITY_THRESHOLD;
    for (unsigned int i = 0; i < num_steps; i++) {
      run_integrator(m_active_scene->m_scene_deltatime / num_steps);
    }
  }
}

void Physics_Manager::handle_scene_physics_book() {}

bool Physics_Manager::initialize_force_generators() {

  if (m_active_scene->m_loaded_points.size() > 0) {

    // gravity
    m_active_scene->m_loaded_force_generators.push_back(
        std::make_shared<Constant_force_generator>(
            glm::vec3(0.0f, -9.81f, 0.0f)));

    // bouyancy for water at 0 height
    m_active_scene->m_loaded_force_generators.push_back(
        std::make_shared<Bouyancy_force_generator>(glm::vec3(0.0f, 15.0f, 0.0f),
                                                   0.0f, 1000.0f));

    // drag (linear and expo components)
    m_active_scene->m_loaded_force_generators.push_back(
        std::make_shared<Drag_force_generator>(0.1f, 0.05f));

    // bouncy thing
    // m_active_scene->m_loaded_force_generators.push_back(std::make_shared<Conditional_force_generator>(glm::vec3(0.0f,15.0f,0.0f)));

    return true;
  }

  log_error("initializing force generators requested but no points in scene, "
            "aborting!!!");

  return false;
}

void debug_particle_movement(std::shared_ptr<Point> p) {

  float mass = 1.0f / p->phys_props.inverse_mass;
  glm::vec3 accel_computed = p->phys_props.force * p->phys_props.inverse_mass;

  std::cout << "Point - mass: " << mass << " | force: ("
            << p->phys_props.force.x << ", " << p->phys_props.force.y << ", "
            << p->phys_props.force.z << ")"
            << " | inv_m: " << p->phys_props.inverse_mass << " | accel: ("
            << accel_computed.x << ", " << accel_computed.y << ", "
            << accel_computed.z << ")"
            << " | velocity: (" << p->phys_props.velocity.x << ", "
            << p->phys_props.velocity.y << ", " << p->phys_props.velocity.z
            << ")" << std::endl;
}

// aka integrator
void Physics_Manager::run_integrator(float simulation_step_dt) {

  // update mesh physics
  for (Entity e : m_active_scene->m_loaded_entities) {
    for (std::shared_ptr<Mesh> m : e.m_mesh) {

      // update AABBs
      if (m->phys_box_needs_recalculation) {
        update_phys_box(*m, e);
      }
    }
  }

  // run point physics
  for (std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {

    // if point is fixed, lock it in place with inverted mass
    bool should_override_mass = false;
    float old_mass = 0.0f;
    if (p->phys_props.fixed) {
      old_mass = p->phys_props.inverse_mass;
      p->phys_props.inverse_mass = 0.0f;
      should_override_mass = true;
    }

    // sumn up all forces on point
    update_alembert_force(*p, simulation_step_dt);

    // debug_particle_movement(p);

    // now take the summed forces and apply em to the points in the scene
    glm::vec3 acceleration = p->phys_props.force * p->phys_props.inverse_mass;
    p->phys_props.velocity += acceleration * simulation_step_dt;

    // prevent velocity explosion by capping it
    if (glm::length(p->phys_props.velocity) > 500.0f)
      p->phys_props.velocity =
          (glm::normalize(p->phys_props.velocity) * 500.0f);

    // firstly write forces to buffer
    p->buffer_integration_delta(p->phys_props.velocity * simulation_step_dt);

    // reset force after applying it successfully
    p->phys_props.force = {0, 0, 0};
    // reset mass if point was fixed
    if (should_override_mass)
      p->phys_props.inverse_mass = old_mass;
  }

  // calc satic constraints n shi
  int iteration_count = 0;
  while (check_violated_static_constraints()) {
    if (iteration_count > MAX_SOLVER_ITERATIONS) {
      log_error("reached solver threshold. exiting solver with potentially "
                "inconsistent physics state!");
      break;
    }
    solve_violated_static_constraints();
    iteration_count++;
  }

  // calculate collissions, adjust buffer integration delta on col
  iteration_count = 0;
  while (check_and_build_collissions()) {
    resolve_collissions_for_scene_preview();
    iteration_count++;
  }

  // after resolving all positions, write to active point position. (if point
  // isnt fixed)
  for (std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {
    if (!p->phys_props.fixed)
      p->swap_integration_buffer();
  }

  // clear temporary force gens, after their force is applied.
  m_active_scene->m_loaded_force_generators.erase(
      std::remove_if(m_active_scene->m_loaded_force_generators.begin(),
                     m_active_scene->m_loaded_force_generators.end(),
                     [](const std::shared_ptr<Force_generator> &fg) {
                       auto *temp =
                           dynamic_cast<Impulse_force_generator *>(fg.get());
                       if (!temp)
                         return false;
                       return temp->force_applied;
                     }),
      m_active_scene->m_loaded_force_generators.end());
}

bool Physics_Manager::check_inside_AABB(Mesh &check_mesh,
                                        glm::vec3 check_position) {

  // glm::vec3 box_min = check_mesh.AABB_visualizer->min_corner;
  // glm::vec3 box_max = check_mesh.AABB_visualizer->max_corner;

  return false;
}

void Physics_Manager::update_alembert_force(Point &p, float delta_time) {

  for (std::shared_ptr<Force_generator> fg :
       m_active_scene->m_loaded_force_generators) {
    p.phys_props.force += fg->get_force(p, delta_time);
  }
};
