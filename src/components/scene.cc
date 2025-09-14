#include "scene.hh"
#include "point.hh"

void Scene::add_point_to_scene(Point to_add) {
  m_loaded_points.push_back(to_add);  
}

void Scene::add_entity_to_scene(Entity to_add) {
  m_loaded_entities.push_back(to_add);  
}


void Scene::add_light_to_scene(Light to_add) {
  m_loaded_lights.push_back(to_add);
}

// the concept is baffling
Scene::Scene() : universal_hitbox_shader("src/shaders/shader_src/line_vertex.glsl","src/shaders/shader_src/line_fragment.glsl"), universal_point_shader("src/shaders/shader_src/point_vertex.glsl","src/shaders/shader_src/point_fragment.glsl") {}
