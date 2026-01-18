#pragma once

#include "collission.hh"
#include "constraint.hh"
#include "entity.hh"
#include "force_generator.hh"
#include "light.hh"
#include "player.hh"
#include "point.hh"
#include "render_properties.hh"
#include "selectionstate.hh"
#include "spring.hh"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "overlay_element.hh"
#include "texture_atlas.hh"

// stdlib
#include <memory>
#include <vector>

class Scene {
public:

  void add_entity_to_scene(Entity to_add);
  void add_light_to_scene(Light to_add);
  void add_point_to_scene(std::shared_ptr<Point> to_add);
  void add_text_to_overlay(std::string to_add, unsigned int anchor_x, unsigned int anchor_y, std::atomic<unsigned int> &num_loaded_textures,
    std::vector<std::tuple<std::string, unsigned int, unsigned int>> &texture_map);

  Scene();

  //TODO maybe doesnt belong here? should move to renderer
  // players in the scene
  std::vector<std::shared_ptr<Player>> m_player_list = {};
  std::shared_ptr<Player> m_local_player = nullptr;

  // containers of things in the scene
  std::vector<Entity> m_loaded_entities = {};
  std::vector<Light> m_loaded_lights = {};
  std::vector<std::shared_ptr<Overlay_Element>> m_loaded_overlay_elements = {};

  // physics containers
  std::vector<std::shared_ptr<Spring>> m_loaded_springs = {};
  std::vector<std::shared_ptr<Point>> m_loaded_points = {};
  std::vector<std::shared_ptr<Collission>> m_current_collissions = {};
  std::vector<std::shared_ptr<Force_generator>> m_loaded_force_generators = {};
  std::vector<std::shared_ptr<Constraint>> m_loaded_constraints = {};

  //interaction
  std::shared_ptr<Selectionstate> m_selectionstate = nullptr;

  bool m_scene_vbos_need_refresh = false;

  float m_scene_deltatime = 0;
  float m_scene_abstime = 0;

  // overlay
  Texture_Atlas texture_atlas;
  bool tex_atlas_initialized = false;
  bool reinit_text_vbos = true;

  //sdf
  GLuint shared_sdf_vao = 0;
  unsigned int shared_sdf_vbo = 0;
  bool shared_sdf_vao_initialized = false;


  //TODO: make this not be here xd
  int m_scene_framebuffer_width = 0;
  int m_scene_framebuffer_height = 0;

  //universal shaders (sdf magic) :)
  Shader universal_hitbox_shader;
  Shader universal_line_shader;
  Shader universal_dotted_line_shader;
  Shader universal_point_shader;
  Shader universal_text_shader;

  //universal shaders normal shit
  Shader universal_phong_shader;
  Shader universal_pbr_shader;
  Shader universal_flat_shader;
  Shader universal_depth_shader;

  //render properties
  Render_Properties render_properties;
  
};
