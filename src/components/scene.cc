#include "scene.hh"
#include "overlay_element.hh"
#include "point.hh"
#include "selectionstate.hh"
#include <GLFW/glfw3.h>

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
      universal_dotted_line_shader("src/shaders/shader_src/dotted_vertex.glsl",
                            "src/shaders/shader_src/dotted_fragment.glsl"),
      universal_point_shader("src/shaders/shader_src/point_vertex.glsl",
                             "src/shaders/shader_src/point_fragment.glsl"),
      universal_text_shader("src/shaders/shader_src/text_vertex.glsl",
                            "src/shaders/shader_src/text_fragment.glsl"),
      universal_phong_shader("src/shaders/shader_src/phong.vert",
                             "src/shaders/shader_src/phong.frag"),
      universal_pbr_shader("src/shaders/shader_src/pbr.vert",
                           "src/shaders/shader_src/pbr.frag"),
      universal_flat_shader("src/shaders/shader_src/flat.vert",
                            "src/shaders/shader_src/flat.frag"),
      universal_depth_shader("src/shaders/shader_src/depth.vert",
                             "src/shaders/shader_src/depth.frag") {
  
  m_selectionstate = std::make_shared<Selectionstate>();
}

void Scene::add_text_to_overlay(std::string to_add, unsigned int anchor_x,
				unsigned int anchor_y, std::atomic<unsigned int> &num_loaded_textures,
    std::vector<std::tuple<std::string, unsigned int, unsigned int>> &texture_map) {

  if(!tex_atlas_initialized)
    texture_atlas.load_glyph_table("fonts/dejavu.fnt","fonts/texture_atlas.png", num_loaded_textures, texture_map);

  float ndc_pos_x = ((float)anchor_x / (float)m_scene_framebuffer_width) * 2.0f - 1.0f;
  float ndc_pos_y = 1.0f - ((float)anchor_y / (float)m_scene_framebuffer_height) * 2.0f;
  std::shared_ptr<Overlay_Element> oe = std::make_shared<Overlay_Element>(to_add, ndc_pos_x, ndc_pos_y);
  oe->texture_glid = texture_atlas.texture_gluint; 
  m_loaded_overlay_elements.push_back(oe);

};
