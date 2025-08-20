#pragma once

#include "./glad/glad.h"
#include "./libs/tiny_gltf.h"
#include "./components/scene.hh"
#include "./components/input.hh"
#include "./components/animationmanager.hh"

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


  // new input handling
  std::unique_ptr<Input_Manager> m_input_manager = nullptr;

  // animation handline
  std::unique_ptr<Animation_Manager> m_animation_manager = nullptr;

  GLFWwindow* associated_window;
  
  // bungie employees hate this one simple trick
  float m_deltaTime = 0.0f;
  float m_application_current_time = 0.0f;

  // Render properties
  unsigned int window_depth_map;
  unsigned int window_depth_map_fbo;
  Shader* depth_shader;
  const unsigned int shadow_map_width = 4000;
  const unsigned int shadow_map_height = 4000;

  int m_viewport_width, m_viewport_height;
  
  bool m_render_mode_wireframe = false;
  
  // Scene management
  std::shared_ptr<Scene> m_active_scene;
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
  void setup_render_properties();
  void handle_scene_physics();
  void calculate_phys_boxes();

  /////////////////////
  // RENDER FUNCTIONS
  /////////////////////

  Renderer(uint window_width, uint window_height);
 
};

