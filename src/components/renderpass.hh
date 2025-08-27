#pragma once

#include "../shaders/shaderclass.hh"
#include "scene.hh"

class Renderpass_Object {
public:
  
  virtual ~Renderpass_Object() = default;
  virtual void setup_vbos();
  virtual void render_frame();

  void update_active_scene(std::shared_ptr<Scene> active_scene);
  template <typename T> void upload_to_uniform(Shader bound_shader,
					       std::string uniform_name,
					       T upload_data);

  //shared variables between render passes
  std::shared_ptr<Scene> m_active_scene = nullptr;
  unsigned int window_depth_map;
  int m_viewport_width, m_viewport_height;
  glm::mat4 shared_light_space_matrix;
  glm::mat4 shared_camera_view_matrix;
  glm::mat4 shared_camera_projection_matrix;
  
  };

// depth pass renderpass  
class Renderpass_Depth: public Renderpass_Object {
public:
  // shadow mapping utils
  unsigned int window_depth_map_fbo;
  const unsigned int shadow_map_width = 4000;
  const unsigned int shadow_map_height = 4000;
  Shader* depth_shader = nullptr;
  
  void setup_vbos();
  void render_frame();
  
};

// color pass renderpass
class Renderpass_Color:public Renderpass_Object {
public:  
  void setup_vbos();
  void render_frame();
};

// overlay pass renderpass
class Renderpass_Overlay : public Renderpass_Object {
public:

  void setup_vbos();
  void render_frame();
  
};
