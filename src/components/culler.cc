#include "culler.hh"
#include "entity.hh"
#include "logging.hh"
#include "scene.hh"
#include <glm/geometric.hpp>
#include <memory>

Culler::Culler(std::shared_ptr<Scene> new_scene_ptr) {

  m_active_scene = new_scene_ptr;
}

void Culler::cull_scene() {

  for(Entity e : m_active_scene->m_loaded_entities) {

    for(std::shared_ptr<Mesh> m : e.m_mesh) {

      //get distance from cam vs threshold

      if(glm::length(m->AABB_visualizer->min_corner - m_active_scene->m_local_player->m_player_camera->m_cameraPos) > 20.0f
	 || glm::length(m->AABB_visualizer->max_corner - m_active_scene->m_local_player->m_player_camera->m_cameraPos) > 20.0f)
	m->m_mesh_culled = true;
      else
	m->m_mesh_culled = false;

    }
    
  }
  
};
