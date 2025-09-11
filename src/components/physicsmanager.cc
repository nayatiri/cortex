#include "physicsmanager.hh"
#include "AABB.hh"
#include "entity.hh"
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

  if (!m_phys_boxes_initialized) {
    calculate_phys_boxes();
    m_phys_boxes_initialized = true;
  }

  //gravity?
  handle_scene_physics_diy();

}

void Physics_Manager::handle_scene_physics_book() {
  
}

// aka integrator
void Physics_Manager::handle_scene_physics_diy() {
  for ( Entity &e : m_active_scene->m_loaded_entities) {
    for ( Mesh &m : e.m_mesh) {

      if(e.entity_type == Entity_Point && m.phys_props.is_physics_object) {

	std::cout << "evaluating point with velocity: " << m.phys_props.velocity.x << " x "<< m.phys_props.velocity.y << " y " << m.phys_props.velocity.z << " z " << std::endl;
	std::cout << "evaluating point with force: " << m.phys_props.force.x << " x "<< m.phys_props.force.y << " y " << m.phys_props.force.z << " z " << std::endl;
	std::cout << "evaluating point with acceleration: " << m.phys_props.acceleration.x << " x "<< m.phys_props.acceleration.y << " y " << m.phys_props.acceleration.z << " z " << std::endl;

	//add gravity
	m.phys_props.force += m.phys_props.gravity;
	
	// p' = p + pt + 0.5 pt ^2
	glm::vec3 acceleration = m.phys_props.acceleration;
	acceleration += m.phys_props.force * m.phys_props.inverse_mass;
	
        m.phys_props.velocity += acceleration * m_active_scene->m_scene_deltatime;
	m.phys_props.velocity *= pow(m.phys_props.damping, m_active_scene->m_scene_deltatime);

	e.change_position(m.phys_props.velocity * m_active_scene->m_scene_deltatime);
	
	m.phys_props.force = {0,0,0};
	
      }
    }
  }
}

bool Physics_Manager::check_inside_AABB(Mesh &check_mesh, glm::vec3 check_position) {


  //glm::vec3 box_min = check_mesh.AABB_visualizer->min_corner;
  //  glm::vec3 box_max = check_mesh.AABB_visualizer->max_corner;

  
  
  return false;
}
