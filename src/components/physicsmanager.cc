#include "physicsmanager.hh"
#include "AABB.hh"
#include "entity.hh"
#include "force_generator.hh"
#include "logging.hh"
#include <memory>

Physics_Manager::Physics_Manager(std::shared_ptr<Scene> set_scene) {
  m_active_scene = set_scene;
};

AABB_Box Physics_Manager::create_collision_box_mesh(const AABB &box) {

  Shader touse = Shader("src/shaders/shader_src/point_vertex.glsl","src/shaders/shader_src/point_fragment.glsl");
  
  AABB_Box newbox = AABB_Box(touse);
  newbox.min_corner = box.min;
  newbox.max_corner = box.max;
  
  return newbox;
  
}

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

  return bbox;
}

void Physics_Manager::calculate_phys_boxes() {

  std::cout << "Scene contains entities: "
            << m_active_scene->m_loaded_entities.size() << std::endl;

  for (Entity &entity : m_active_scene->m_loaded_entities) {
    
    glm::mat4 entity_transform = entity.get_model_matrix();
    std::cout << "Entity contains meshes: " << entity.m_mesh.size()
              << std::endl;

    for (Mesh &mesh : entity.m_mesh) {
      
      if (mesh.m_type != E_MESH) {
        log_error("Mesh does'nt seem to be a Mesh lol. skipping.");
        continue;
      };

      if (mesh.m_vertices_array.size() < 3) {
        log_error("Attempted to create hitbox for mesh with insufficient "
                  "vertices (less than 3). Skipping.");
        continue;
      }

      AABB bbox = compute_world_space_aabb(mesh, entity_transform);
      
      mesh.AABB_visualizer = std::make_shared<AABB_Box>( create_collision_box_mesh(bbox) );

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
  if(!m_force_generators_initialized) {
    bool success = initialize_force_generators();
    if (success)
      m_force_generators_initialized = true;
    else
      m_force_generators_initialized = false;
  }

  //process physics if everything is setup
  run_integrator();

}

void Physics_Manager::handle_scene_physics_book() {}

bool Physics_Manager::initialize_force_generators() {
  
  if(m_active_scene->m_loaded_points.size() > 0){

    //gravity
    m_active_scene->m_loaded_force_generators.push_back(std::make_shared<Constant_force_generator>(glm::vec3(0.0f,-9.81f,0.0f)));

    //bouyancy for water at 0 height
    m_active_scene->m_loaded_force_generators.push_back(std::make_shared<Bouyancy_force_generator>(glm::vec3(0.0f,15.0f,0.0f), 0.0f, 1000.0f));

    //drag (linear and expo components)
    m_active_scene->m_loaded_force_generators.push_back(std::make_shared<Drag_force_generator>(0.1f,0.05f));
    
    //bouncy thing
    //m_active_scene->m_loaded_force_generators.push_back(std::make_shared<Conditional_force_generator>(glm::vec3(0.0f,15.0f,0.0f)));

    return true;
  }
  
  log_error("couldnt init gens!!!");
  
  return false;
  
}

void debug_particle_movement(std::shared_ptr<Point> p) {

    float mass = 1.0f / p->phys_props.inverse_mass;
    glm::vec3 accel_computed = p->phys_props.force * p->phys_props.inverse_mass;
    
    std::cout << "Point - mass: " << mass
	      << " | force: (" << p->phys_props.force.x << ", " << p->phys_props.force.y << ", " << p->phys_props.force.z << ")"
	      << " | inv_m: " << p->phys_props.inverse_mass
	      << " | accel: (" << accel_computed.x << ", " << accel_computed.y << ", " << accel_computed.z << ")"
	      << " | velocity: (" << p->phys_props.velocity.x << ", " << p->phys_props.velocity.y << ", " << p->phys_props.velocity.z << ")"
	      << std::endl;
}

// aka integrator
void Physics_Manager::run_integrator() {
  
  for(std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {

    //if point is fixed, lock it in place
    if(p->phys_props.fixed)
      continue;
    
    // sumn up all forces on point
    update_alembert_force(*p, m_active_scene->m_scene_deltatime); 

    //debug_particle_movement(p);
    
    // now take the summed forces and apply em to the points in the scene
    glm::vec3 acceleration = p->phys_props.force * p->phys_props.inverse_mass;
    p->phys_props.velocity += acceleration * m_active_scene->m_scene_deltatime;

    // firstly write forces to buffer
    p->buffer_integration_delta(p->phys_props.velocity * m_active_scene->m_scene_deltatime);
    
    // check for collission here?
    
    //reset force after applying it successfully
    p->phys_props.force = {0,0,0};
    
  }
  
  // after resolving all positions, write to active point position.
  for(std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {
    p->swap_integration_buffer();
  }
  
}

bool Physics_Manager::check_inside_AABB(Mesh &check_mesh, glm::vec3 check_position) {

  //glm::vec3 box_min = check_mesh.AABB_visualizer->min_corner;
  //glm::vec3 box_max = check_mesh.AABB_visualizer->max_corner;

  return false;
}

void Physics_Manager::update_alembert_force(Point &p, float delta_time) {
  
  for(std::shared_ptr<Force_generator> fg : m_active_scene->m_loaded_force_generators) {
    p.phys_props.force += fg->get_force(p,delta_time); 
  }
  
};
