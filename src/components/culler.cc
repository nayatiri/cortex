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
      //cull_distance(m);
      //cull_behind(m);
    }    
  }  
  
};

void Culler::cull_distance(std::shared_ptr<Mesh> m) {
  //get distance from cam vs threshold
  glm::vec3 midpoint_pos = glm::vec3(m->AABB_visualizer->min_corner + m->AABB_visualizer->min_corner) / 2.0f; 
  if(glm::length(midpoint_pos - m_active_scene->m_local_player->m_player_camera->m_cameraPos) > 20.0f)
    m->m_mesh_culled = true;
  else
    m->m_mesh_culled = false;
};

void Culler::cull_behind(std::shared_ptr<Mesh> m) {
  glm::vec3 midpoint_pos = glm::vec3(m->AABB_visualizer->min_corner + m->AABB_visualizer->min_corner) / 2.0f; 
  glm::vec3 cam_to_midpoint =  glm::normalize(midpoint_pos - m_active_scene->m_local_player->m_player_camera->m_cameraPos);
  glm::vec3 cam_to_lookat = glm::normalize(m_active_scene->m_local_player->m_player_camera->m_cameraLookAt);

  float angle = glm::dot(cam_to_lookat, cam_to_midpoint);
  
  if(angle < 0.0f)
    m->m_mesh_culled = true;
  else
    m->m_mesh_culled = false;
  
};
