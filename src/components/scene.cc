#include "scene.hh"
#include "point.hh"
#include "selectionstate.hh"

void Scene::add_point_to_scene(std::shared_ptr<Point> to_add) {
  m_loaded_points.push_back(to_add);  
}

void Scene::add_entity_to_scene(Entity to_add) {
  m_loaded_entities.push_back(to_add);  
}


void Scene::add_light_to_scene(Light to_add) {
  m_loaded_lights.push_back(to_add);
}

// the concept is baffling
Scene::Scene()
    : universal_hitbox_shader("src/shaders/shader_src/hitbox_vertex.glsl",
                              "src/shaders/shader_src/hitbox_fragment.glsl"),
      universal_line_shader("src/shaders/shader_src/line_vertex.glsl",
                            "src/shaders/shader_src/line_fragment.glsl"),
      universal_point_shader("src/shaders/shader_src/point_vertex.glsl",
                             "src/shaders/shader_src/point_fragment.glsl"),
      universal_phong_shader("src/shaders/shader_src/phong.vert",
                             "src/shaders/shader_src/phong.frag"),
      universal_flat_shader("src/shaders/shader_src/flat.vert",
                            "src/shaders/shader_src/flat.frag"),
      universal_depth_shader("src/shaders/shader_src/depth.vert",
                             "src/shaders/shader_src/depth.frag") {

  m_selectionstate = std::make_shared<Selectionstate>();
}
