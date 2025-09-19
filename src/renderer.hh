#pragma once

#include "./glad/glad.h"
#include "./libs/tiny_gltf.h"
#include "./components/scene.hh"
#include "./components/input.hh"
#include "./components/animationmanager.hh"
#include "components/physicsmanager.hh"
#include "components/pipeline.hh"

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
  // new input handling
  std::unique_ptr<Input_Manager> m_input_manager = nullptr;

  // animation handline
  std::unique_ptr<Animation_Manager> m_animation_manager = nullptr;

  // animation handline
  std::unique_ptr<Physics_Manager> m_physics_manager = nullptr;
  
  // bungie employees hate this one simple trick
  float m_deltaTime = 0.0f;
  float m_application_current_time = 0.0f;

  //FLAGS / ATTRIBUTE
  bool m_should_shutdown = false;
  bool m_render_mode_wireframe = false;
  int m_viewport_width = 0, m_viewport_height = 0;
  GLFWwindow* associated_window = nullptr;

  //abstract render function
  std::unique_ptr<Pipeline> m_pipeline = nullptr;
  
  // Scene management
  std::shared_ptr<Scene> m_active_scene = nullptr;

  // texture cache index
  std::atomic<uint32_t> num_loaded_textures = 0;
  std::vector<std::tuple<std::string, unsigned int, GLuint>> m_texture_map = {};
  
  /////////////////////
  // CALLBACK FUNCTIONS
  /////////////////////
  static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
  void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
  void mouse_callback(GLFWwindow *window, double xpos, double ypos);
  void processInput(GLFWwindow *window);

  /////////////////////
  // VRAM MANAGEMENT FUNCTIONS
  /////////////////////
  template <typename T> void upload_to_uniform(Shader bound_shader,std::string uniform_name, T input);
  void init_scene_vbos();
  void cleanup_mesh_vbos(Mesh& mesh);

  /////////////////////
  // SCENE MANAGEMENT
  /////////////////////
  void init_scene(const char* scene_fp);
  void add_model_to_scene( const char* filepath);
  void add_model_to_player_hand( const char* filepath);
  void add_point_to_scene(float x,float y,float z);
  void add_player_to_scene(bool make_local_player);
  void create_spring_constraint(std::shared_ptr<Point> from, std::shared_ptr<Point> to, float strength, float rest_length);
  void create_fixed_constraint(std::shared_ptr<Point> p , bool fixed);
  void setup_render_properties();

  /////////////////////
  // UTILITY FUNCTIONS
  /////////////////////
  void update_scene_time();
  bool save_frame_to_png(const char* filename, int width, int height);
  
  /////////////////////
  // RENDER FUNCTIONS
  /////////////////////
  Renderer(uint window_width, uint window_height);
  void render_frame();
  void abstract_render();
};

