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

void Physics_Manager::handle_scene_physics_diy() {
  for ( Entity &e : m_active_scene->m_loaded_entities) {
    for ( Mesh &m : e.m_mesh) {
      /*

       check if entity is stationary
       if not, check impulse. if its not 0, turn  velocity += impulse/mass
       impulse = 0
       velocity += gravity * deltatime
       preview position vector =  position + velocity * deltatime
       create relative movement vector from  preview - position
       check if movement vector collides with any face of the scene
       if collission, mark entity as colliding, dont do movement, but adjust energies to calculate bounce or similar
       if no collission, adjust position by movement vector, set impulse to 0
       
      */

      // if its a point, and its a physics object
      if(e.entity_type == Entity_Point && m.phys_props.is_physics_object) {

	// check our bozo acceleration structure (its not an acceleration structure xd)
	for ( Entity &check_e : m_active_scene->m_loaded_entities) {
	  for ( Mesh &check_m : check_e.m_mesh) {
	    if(!check_inside_AABB(check_m, e.get_position()))
	      continue;
	    
	    if(m.phys_props.impulse.length() == 0) {
	      m.phys_props.stationary = true;
	    } else {
	      m.phys_props.stationary = false;
	    }
	    
	    if(m.phys_props.stationary)
	      continue;
	    
	    
	    
	    // m.change_position( 0 , - m.phys_props.gravity * m_active_scene->m_scene_deltatime , 0 );
	    
	  }
	}	
      } else {continue;}

    }
  }
}

bool Physics_Manager::check_inside_AABB(Mesh &check_mesh, glm::vec3 check_position) {


  //glm::vec3 box_min = check_mesh.AABB_visualizer->min_corner;
  //  glm::vec3 box_max = check_mesh.AABB_visualizer->max_corner;

  
  
  return false;
}
