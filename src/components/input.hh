#pragma once

#include "scene.hh"
#include <memory>

class Input_Manager {
public:

  // Window control
  bool m_is_mouse_grabbed = true;
  bool m_is_mouse_on_cooldown = false;
  bool m_first_mouse = true;
  bool m_last_mouse_state = false;
  int m_viewport_width = 1920;
  int m_viewport_height = 1080;

  std::shared_ptr<Scene> m_active_scene = nullptr;
  
  float m_deltaTime = 0.0f;
  float m_application_current_time = 0.0f;

  bool m_should_shutdown = false;
  
  bool m_render_mode_wireframe = false;
  bool m_last_wireframe_state = false;
  bool m_is_wireframe_on_cooldown = false;

  // Player Position buffers 
  double m_lastX = 0;
  double m_lastY = 0;
  double m_yaw = 0;
  double m_pitch = 0;

  
  Input_Manager(std::shared_ptr<Scene> m_scene_ptr);

  void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

  void mouse_callback(GLFWwindow *window, double xpos, double ypos);

  void process_input(GLFWwindow *window);

  bool save_frame_to_png(const char *filename, int width, int height);
};
