#pragma once

#include "./glad/glad.h"
#include "./libs/tiny_gltf.h"
#include "./components/scene.hh"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stdlib
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <atomic>

class Renderer {
public:
  bool m_should_shutdown = false;

  // Window control
  bool m_is_mouse_grabbed = true;
  bool m_is_mouse_on_cooldown = false;
  bool m_first_mouse = true;
  bool m_last_mouse_state = false;
  int m_viewport_width = 1920;
  int m_viewport_height = 1080;

  GLFWwindow* associated_window;

  // Player Position buffers 
  double m_lastX = 0;
  double m_lastY = 0;
  double m_yaw = 0;
  double m_pitch = 0;
  
  // bungie employees hate this one simple trick
  float m_deltaTime = 0.0f;
  float m_application_current_time = 0.0f;

  // Render properties
  bool m_render_mode_wireframe = false;
  bool m_last_wireframe_state = false;
  bool m_is_wireframe_on_cooldown = false;
  unsigned int window_depth_map;
  unsigned int window_depth_map_fbo;
  Shader* depth_shader;
  const unsigned int shadow_map_width = 4000;
  const unsigned int shadow_map_height = 4000;
  
  // Scene management
  std::unique_ptr<Scene> m_active_scene;
  std::atomic<uint32_t> num_loaded_textures = 0;

  std::vector<std::tuple<std::string, unsigned int, GLuint>> m_texture_map;

  /////////////////////
  // CALLBACK FUNCTIONS
  /////////////////////
  
  void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  void mouse_callback(GLFWwindow *window, double xpos, double ypos);
  static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
  void processInput(GLFWwindow *window);

  template <typename T> void upload_to_uniform(std::string location, GLuint shader_id, T input);

  void init_scene_vbos();
  void init_scene(const char* scene_fp);
  void render_frame();
  bool save_frame_to_png(const char* filename, int width, int height);
  void handle_scene_animations();
  void setup_render_properties();
  void handle_scene_physics();
  void calculate_phys_boxes();

  /////////////////////
  // RENDER FUNCTIONS
  /////////////////////

  Renderer(uint window_width, uint window_height);
 
};

