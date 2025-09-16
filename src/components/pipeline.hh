#pragma once

#include "scene.hh"

class Pipeline {
public:

  ~Pipeline() = default;

  virtual void init_pipeline() = 0;
  virtual void render_frame() = 0;
  
  void check_gl_error(const char *context);
  template <typename T> void upload_to_uniform(Shader bound_shader,
					       std::string uniform_name,
					       T upload_data);

  // flags
  bool vbos_need_refresh = false;
  bool pipeline_is_setup = false;

  //shared variables between render passes
  std::shared_ptr<Scene> m_active_scene = nullptr;
  unsigned int window_depth_map;
  int m_viewport_width, m_viewport_height;
  glm::mat4 shared_light_space_matrix;
  glm::mat4 shared_camera_view_matrix;
  glm::mat4 shared_camera_projection_matrix;

  
};

class Shadow_Map_Pipeline : public Pipeline {
private:
  void render_depth_pass();
  void render_color_pass();
  void render_overlay_pass();

  void render_color_point_cloud();

  
  
  void init_depth_pass();
  void init_color_pass();

  // shadow mapping utils
  unsigned int window_depth_map_fbo;
  const unsigned int shadow_map_width = 4000;
  const unsigned int shadow_map_height = 4000;
  Shader* depth_shader = nullptr;
  
public:
  void render_frame();
  void init_pipeline();
  void update_scene(std::shared_ptr<Scene> new_scene);
  void update_time(float new_time);

  // TODO move this from here into input.
  void handle_pick();
};


class Ray_Traced_Pipeline : public Pipeline {
public:
  void render_frame();
  void update_scene();
  void update_time();  
};
