#pragma once

#include "scene.hh"
#include <atomic>
#include <memory>

class Input_Manager {
public:

  ///////////
  static constexpr int COMMON_KEYS[] = {
    // letters
    GLFW_KEY_A, GLFW_KEY_B, GLFW_KEY_C, GLFW_KEY_D, GLFW_KEY_E,
    GLFW_KEY_F, GLFW_KEY_G, GLFW_KEY_H, GLFW_KEY_I, GLFW_KEY_J,
    GLFW_KEY_K, GLFW_KEY_L, GLFW_KEY_M, GLFW_KEY_N, GLFW_KEY_O,
    GLFW_KEY_P, GLFW_KEY_Q, GLFW_KEY_R, GLFW_KEY_S, GLFW_KEY_T,
    GLFW_KEY_U, GLFW_KEY_V, GLFW_KEY_W, GLFW_KEY_X, GLFW_KEY_Y,
    GLFW_KEY_Z,
    
    // numbers
    GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4,
    GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9,

    // mod keys
    GLFW_KEY_SPACE,
    GLFW_KEY_LEFT_SHIFT,
    GLFW_KEY_LEFT_CONTROL,
    GLFW_KEY_LEFT_ALT,
    GLFW_KEY_ENTER,
    GLFW_KEY_ESCAPE,
    GLFW_KEY_TAB,
    GLFW_KEY_BACKSPACE,
    GLFW_KEY_INSERT,
    GLFW_KEY_DELETE,
    GLFW_KEY_HOME,
    GLFW_KEY_END,
    GLFW_KEY_PAGE_UP,
    GLFW_KEY_PAGE_DOWN,

    // arrows
    GLFW_KEY_UP,
    GLFW_KEY_DOWN,
    GLFW_KEY_LEFT,
    GLFW_KEY_RIGHT,

    // fkeys
    GLFW_KEY_F1,  GLFW_KEY_F2,  GLFW_KEY_F3,  GLFW_KEY_F4,
    GLFW_KEY_F5,  GLFW_KEY_F6,  GLFW_KEY_F7,  GLFW_KEY_F8,
    GLFW_KEY_F9,  GLFW_KEY_F10, GLFW_KEY_F11, GLFW_KEY_F12,

    // puncuations
    GLFW_KEY_MINUS,
    GLFW_KEY_EQUAL,
    GLFW_KEY_LEFT_BRACKET,
    GLFW_KEY_RIGHT_BRACKET,
    GLFW_KEY_BACKSLASH,
    GLFW_KEY_SEMICOLON,
    GLFW_KEY_APOSTROPHE,
    GLFW_KEY_COMMA,
    GLFW_KEY_PERIOD,
    GLFW_KEY_SLASH,

    // yikespad
    GLFW_KEY_KP_0, GLFW_KEY_KP_1, GLFW_KEY_KP_2,
    GLFW_KEY_KP_3, GLFW_KEY_KP_4, GLFW_KEY_KP_5,
    GLFW_KEY_KP_6, GLFW_KEY_KP_7, GLFW_KEY_KP_8,
    GLFW_KEY_KP_9,
    GLFW_KEY_KP_DECIMAL,
    GLFW_KEY_KP_DIVIDE,
    GLFW_KEY_KP_MULTIPLY,
    GLFW_KEY_KP_SUBTRACT,
    GLFW_KEY_KP_ADD,
    GLFW_KEY_KP_ENTER,
  };
  
  struct KeyState {
    std::atomic<int> keysym = 0;
    std::atomic<bool> keystate = false;
    std::atomic<bool> last_keystate = false;
  };
  
  std::vector<std::unique_ptr<KeyState>> key_map;
  GLFWwindow *window_ptr;
  
  void init_key_states();
  void update_key_syms();
  void start_input_manager(GLFWwindow*);
  
  ///////////
  
  // Window control
  bool m_is_mouse_grabbed = true;
  bool m_is_mouse_on_cooldown = false;
  bool m_first_mouse = true;
  bool m_last_mouse_state = false;
  int m_viewport_width = 1920;
  int m_viewport_height = 1080;

  std::shared_ptr<Scene> m_active_scene = nullptr;

  bool m_should_shutdown = false;
  
  bool m_render_mode_wireframe = false;
  bool m_last_wireframe_state = false;
  bool m_is_wireframe_on_cooldown = false;

  bool m_render_mode_hitbox = false;
  bool m_last_hitbox_state = false;
  bool m_is_hitbox_on_cooldown = false;

  bool m_culling_mode = false;
  bool m_last_culling_state = false;
  bool m_is_culling_on_cooldown = false;
  
  bool m_render_mode_normal_visualizer = false;
  bool m_last_normal_visualizer_state = false;
  bool m_is_normal_visualizer_on_cooldown = false;

  
  // Player Position buffers 
  double m_lastX = 0;
  double m_lastY = 0;
  double m_yaw = 0;
  double m_pitch = 0;

  //shitty lock buffer TODO make this not be here and write proper input class
  bool m_was_x_pressed = false;

  
  Input_Manager(std::shared_ptr<Scene> m_scene_ptr);

  void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

  void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

  void mouse_callback(GLFWwindow *window, double xpos, double ypos);

  void fb_resize_callback(GLFWwindow *window, int width, int height);

  void process_input(GLFWwindow *window, float m_application_current_time, float m_delta_time);

  bool save_frame_to_png(const char *filename, int width, int height);

  void handle_mouse_pick();
};
