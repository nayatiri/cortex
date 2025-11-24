#include "physicsmanager.hh"
#include "AABB.hh"
#include "collission.hh"
#include "entity.hh"
#include "force_generator.hh"
#include "logging.hh"
#include "point.hh"
#include <algorithm>
#include <glm/geometric.hpp>
#include <memory>
#include <random>

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

bool Physics_Manager::check_and_build_collissions() {

  bool found_collissions = false;

  for (std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {
    for (std::shared_ptr<Point> q : m_active_scene->m_loaded_points) {

      if (p == q)
        continue;
      
      //collission detected?
      if (p->phys_props.radius + q->phys_props.radius >
          p->get_distance_to_other_point(q) + 0.01f) {

        /*
	  std::cout << "collission detected with pq rad: " <<
          p->phys_props.radius + q->phys_props.radius <<
          " and dist: " << p->get_distance_to_other_point(q) <<
          " n p q pos " << p->get_position().y <<
          " " << q->get_position().y << "\n";
	*/

        found_collissions = true;

	if(!p->phys_props.involved_in_collission && !q->phys_props.involved_in_collission) {
	  m_active_scene->m_current_collissions.push_back(
							  std::make_shared<Point_Point_Collission>(
												   p,
												   q,
												   glm::normalize(p->get_position() - q->get_position())));
	  p->phys_props.involved_in_collission = true;
	  q->phys_props.involved_in_collission = true;
	} else {
	  //log_error("tried to create collission for points, that already have a collission... not making a new one.");
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
  if(m_active_scene->m_current_collissions.empty())
    return;

  for(std::shared_ptr<Collission> c : m_active_scene->m_current_collissions) {

    // handle point point cols
    std::shared_ptr<Point_Point_Collission> ppc = std::dynamic_pointer_cast<Point_Point_Collission>(c);
    if(ppc) {

      float distance_should_be = ppc->point_a->phys_props.radius + ppc->point_b->phys_props.radius;
      float distance_is = ppc->point_a->get_distance_to_other_point(ppc->point_b);

      float distance_adjust = ((distance_should_be - distance_is) / 2) + 0.01;

      //std::cout << "adjusting points with offset: " << distance_adjust << "\n";

      // points in same location? random direction move
      if(ppc->point_a->get_position() == ppc->point_b->get_position()) {
	static std::mt19937 rng{std::random_device{}()};
	static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
	glm::vec3 random_force = glm::vec3(dist(rng), dist(rng), dist(rng));
	ppc->point_a->change_position( random_force * distance_adjust );
	ppc->point_b->change_position( random_force * -distance_adjust);
	continue;
      }
      
      //solve collission
      ppc->point_a->change_position( ppc->contact_normal * distance_adjust );
      ppc->point_b->change_position(ppc->contact_normal * -distance_adjust);

      //handle collission force
      m_active_scene->m_loaded_force_generators.push_back(std::make_shared<Temporary_force_generator>(ppc->point_a, 10.0f, ppc->contact_normal));
      m_active_scene->m_loaded_force_generators.push_back(std::make_shared<Temporary_force_generator>(ppc->point_b, 10.0f, -ppc->contact_normal));
      
    }

    c->resolved = true;
    
  }

  //clear all collissions
  for(auto p : m_active_scene->m_loaded_points) {p->phys_props.involved_in_collission = false;}
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

  // process physics if everything is setup
  run_integrator();
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
void Physics_Manager::run_integrator() {

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

    bool should_override_mass = false;
    float old_mass = 0.0f;

    // if point is fixed, lock it in place with inverted mass
    if (p->phys_props.fixed) {
      old_mass = p->phys_props.inverse_mass;
      p->phys_props.inverse_mass = 0.0f;
      should_override_mass = true;
    }

    // sumn up all forces on point
    update_alembert_force(*p, m_active_scene->m_scene_deltatime);

    // debug_particle_movement(p);

    // now take the summed forces and apply em to the points in the scene
    glm::vec3 acceleration = p->phys_props.force * p->phys_props.inverse_mass;
    p->phys_props.velocity += acceleration * m_active_scene->m_scene_deltatime;

    // prevent velocity explosion by capping it
    if (glm::length(p->phys_props.velocity) > 500.0f)
      p->phys_props.velocity =
          (glm::normalize(p->phys_props.velocity) * 500.0f);

    // firstly write forces to buffer
    p->buffer_integration_delta(p->phys_props.velocity *
                                m_active_scene->m_scene_deltatime);

    // reset force after applying it successfully
    p->phys_props.force = {0, 0, 0};
    // reset mass if point was fixed
    if (should_override_mass)
      p->phys_props.inverse_mass = old_mass;
  }

  // calculate collissions, adjust buffer integration delta on col
  while (check_and_build_collissions()) {
    resolve_collissions_for_scene_preview();
  }

  // after resolving all positions, write to active point position. (if point
  // isnt fixed)
  for (std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {
    if (!p->phys_props.fixed)
      p->swap_integration_buffer();
  }

  //clear temporary force gens, after their force is applied.
  m_active_scene->m_loaded_force_generators.erase(
    std::remove_if(
        m_active_scene->m_loaded_force_generators.begin(),
        m_active_scene->m_loaded_force_generators.end(),
        [](const std::shared_ptr<Force_generator>& fg) {
            // Check if it is a Temporary_force_generator
            auto* temp = dynamic_cast<Temporary_force_generator*>(fg.get());
            if (!temp) return false;

            // Then check whether force_applied is true
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
