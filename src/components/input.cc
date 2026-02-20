#include "input.hh"
#include "logging.hh"
#include <GLFW/glfw3.h>
#include <chrono>
#include <thread>

/*TODO

  move to smaller functions / recode input system to have a bitmask for all keys, then be able to check(key) to get its keystate

 */

void Input_Manager::process_input(GLFWwindow *window,
                                  float m_application_current_time,
                                  float m_delta_time) {

  if (m_active_scene == nullptr || m_active_scene->m_local_player->m_player_camera == nullptr) {
    Logger::log_error("active scene is fucked. cant process inputs");
    return;
  }

  if (window == nullptr)
    Logger::log_error("window is null, cannot process input.");

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
    m_should_shutdown = true;
    Logger::log_success("shutting down window.");
  }

  float cameraSpeed =
      m_active_scene->m_local_player->m_player_camera->m_camera_base_speed * 10.0f * m_delta_time;

  // toggle WIREFRAME Q+W
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    if (!m_last_wireframe_state) {
      m_active_scene->render_properties.render_wireframe = !m_active_scene->render_properties.render_wireframe;
      m_is_wireframe_on_cooldown = true;
      m_last_wireframe_state = true;
    }
  } else {
    m_last_wireframe_state = false;
  }

  //toggle HITBOX Q+H
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
    if (!m_last_hitbox_state) {
      m_active_scene->render_properties.render_hitbox = !m_active_scene->render_properties.render_hitbox;
      m_is_hitbox_on_cooldown = true;
      m_last_hitbox_state = true;
    }
  } else {
    m_last_hitbox_state = false;
  }

  //toggle culling Q+C
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
    if (!m_last_culling_state) {
      m_active_scene->render_properties.cull_scene = !m_active_scene->render_properties.cull_scene;
      m_is_culling_on_cooldown = true;
      m_last_culling_state = true;
      Logger::log_debug("cull on");
      std::cout << m_active_scene->render_properties.cull_scene << std::endl;
    }
  } else {
    m_last_culling_state = false;
  }

  
  //toggle Normal visualizer Q+N
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
    if (!m_last_normal_visualizer_state) {
      m_active_scene->render_properties.render_normal_visualizer = !m_active_scene->render_properties.render_normal_visualizer;
      m_is_normal_visualizer_on_cooldown = true;
      m_last_normal_visualizer_state = true;
    }
  } else {
    m_last_normal_visualizer_state = false;
  }

  
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    m_active_scene->m_local_player->m_player_camera->m_cameraPos +=
        cameraSpeed * glm::normalize(glm::vec3(
                          m_active_scene->m_local_player->m_player_camera->m_cameraLookAt.x, 0.0f,
                          m_active_scene->m_local_player->m_player_camera->m_cameraLookAt.z));
  }

  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
    if (m_active_scene->m_local_player->m_player_camera->m_animation_table == nullptr) {
      m_active_scene->m_local_player->m_player_camera->m_animation_table =
          new std::vector<animation *>();
      m_active_scene->m_local_player->m_player_camera->m_animation_table->clear();
      m_active_scene->m_local_player->m_player_camera->m_animation_table->reserve(1);
      m_active_scene->m_local_player->m_player_camera->m_animation_table->push_back(new animation);
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_checkpoints =
          new std::vector<glm::vec3>();
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
          ->m_checkpoints->clear();
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_checkpoints_rot =
          new std::vector<glm::vec3>();
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
          ->m_checkpoints_rot->clear();
    } else {
      printf("%d ", (int)m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
                        ->m_checkpoints->size());
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
	->m_checkpoints->push_back(m_active_scene->m_local_player->get_position());
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
          ->m_checkpoints_rot->push_back(
              m_active_scene->m_local_player->m_player_camera->m_cameraLookAt);
      Logger::log_debug("saved animation point");
      //      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
    if (m_active_scene->m_local_player->m_player_camera->m_animation_table &&
        m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_checkpoints) {
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
          ->m_checkpoints->clear();
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
          ->m_checkpoints_rot->clear();
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_start_time = 0;
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_has_been_smoothed =
          false;
    }
  }

  //FOV
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
    m_active_scene->m_local_player->m_player_camera->fov += 1.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
    m_active_scene->m_local_player->m_player_camera->fov -= 1.0f;
  }


  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {

    // does an animation exist? start animation
    if (m_active_scene->m_local_player->m_player_camera->m_animation_table) {
      if (m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
                  ->m_checkpoints->size() > 1 &&
          m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_start_time ==
              0) {
        m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
            ->m_trigger_animation = true;
        Logger::log_success("queuing animation");
      }
    }

    if (m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
            ->m_has_been_smoothed == false) {

      for (int i = 0;
           i < ((int)m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
                    ->m_checkpoints->size()) -
                   20;
           i++) {
        glm::vec3 step_nosmooth =
            m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
                ->m_checkpoints->at(i);
        glm::vec3 step_next_nosmooth =
            m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
                ->m_checkpoints->at(i + 1);
        glm::vec3 step_smoothed = (step_nosmooth + step_next_nosmooth);
        step_smoothed /= 2;
        m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_checkpoints->at(
            i) = step_smoothed;
        Logger::log_error("smooting in progress");
      }
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_has_been_smoothed =
          true;
    }

    // has an animation been set to start? initialize it + set vars
    if (m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)
            ->m_trigger_animation == true) {
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_start_time =
          m_application_current_time;
      m_active_scene->m_local_player->m_player_camera->m_animation_table->at(0)->m_last_checkpoint = 0;
      Logger::log_success("initizlizing animation");
    }
  }

  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    //    save_frame_to_png("output.png", m_viewport_width, m_viewport_height);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
    glm::vec3 point_pos = m_active_scene->m_local_player->m_player_camera->m_cameraPos;
    std::shared_ptr<Point>to_add = std::make_shared<Point>(point_pos.x,point_pos.y-1.0f,point_pos.z);
    to_add->phys_props.velocity = glm::normalize(m_active_scene->m_local_player->m_player_camera->m_cameraLookAt) * 20.0f;
    m_active_scene->add_point_to_scene(to_add);

    std::cout << "number of points in scene:" << m_active_scene->m_loaded_points.size() << std::endl;  
    
  }
  
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    m_active_scene->m_local_player->m_player_camera->m_cameraPos +=
        cameraSpeed * glm::normalize(glm::vec3(
                          -m_active_scene->m_local_player->m_player_camera->m_cameraLookAt.x, 0.0f,
                          -m_active_scene->m_local_player->m_player_camera->m_cameraLookAt.z));
  }

  // move all particles to 0,5,0 (ik its dumb)
  if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
    for (std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {
      p->set_position(0, 5, 0);
      p->phys_props.velocity = {0, 0, 0};
      p->phys_props.force = {0, 0, 0};
      p->phys_props.acceleration = {0, 0, 0};
    }
  }

  // move left
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    m_active_scene->m_local_player->m_player_camera->m_cameraPos -=
        glm::normalize(glm::cross(m_active_scene->m_local_player->m_player_camera->m_cameraLookAt,
                                  m_active_scene->m_local_player->m_player_camera->m_cameraUp)) *
        cameraSpeed;

  //move right
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    m_active_scene->m_local_player->m_player_camera->m_cameraPos +=
        glm::normalize(glm::cross(m_active_scene->m_local_player->m_player_camera->m_cameraLookAt,
                                  m_active_scene->m_local_player->m_player_camera->m_cameraUp)) *
        cameraSpeed;

  //move down
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    m_active_scene->m_local_player->m_player_camera->m_cameraPos +=
        glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)) * cameraSpeed;

  //move up
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    m_active_scene->m_local_player->m_player_camera->m_cameraPos +=
        glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)) * cameraSpeed;

  // toggle cursor 
  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
    if (m_last_mouse_state == false) {
      m_is_mouse_grabbed = !m_is_mouse_grabbed;
      if (m_is_mouse_grabbed) {
        m_is_mouse_on_cooldown = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      } else {
        m_is_mouse_on_cooldown = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }
      m_last_mouse_state = true;
    }
  }

  if (glfwGetKey(window, GLFW_KEY_G) != GLFW_PRESS) {
    m_last_mouse_state = false;
  }

  // toggle fixed state  
  bool is_x_pressed = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
  if (is_x_pressed && !m_was_x_pressed && m_active_scene->m_selectionstate->selected_point != nullptr) {
    m_active_scene->m_selectionstate->selected_point->phys_props.fixed =
      !m_active_scene->m_selectionstate->selected_point->phys_props.fixed;
  }
  m_was_x_pressed = is_x_pressed;

  // throw selected particle upwards
  if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && m_active_scene->m_selectionstate->selected_point != nullptr) {
    std::shared_ptr<Point> p = m_active_scene->m_selectionstate->selected_point;
    p->phys_props.add_force(0, 50, 0);
  }

  // throw selected particle towards X
  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && m_active_scene->m_selectionstate->selected_point != nullptr) {
    std::shared_ptr<Point> p = m_active_scene->m_selectionstate->selected_point;
    p->phys_props.add_force(50, 0, 0);
  }
  
  return;
}

Input_Manager::Input_Manager(std::shared_ptr<Scene> m_scene_ptr) {

  Logger::log_success("input manger online");

  m_active_scene = m_scene_ptr;
}

/*bool Input_Manager::save_frame_to_png(const char *filename, int width, int
  height) { std::vector<unsigned char> pixels(width * height * 3);

  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadBuffer(GL_FRONT);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

  for (int y = 0; y < height / 2; ++y) {
    int index1 = y * width * 3;
    int index2 = (height - 1 - y) * width * 3;
    for (int x = 0; x < width * 3; ++x)
      std::swap(pixels[index1 + x], pixels[index2 + x]);
  }

  int success =
      stbi_write_png(filename, width, height, 3, pixels.data(), width * 3);
  if (!success) {
    std::cerr << "Failed to write PNG file\n";
    return false;
  }
  std::cout << "Saved framebuffer to " << filename << "\n";
  return true;
  }*/

void Input_Manager::scroll_callback(GLFWwindow *window, double xoffset,
                                    double yoffset) {
  Input_Manager *im =
      static_cast<Input_Manager *>(glfwGetWindowUserPointer(window));

  if (!im) {
    std::cerr << "Input_Manager is null in scroll callback!" << std::endl;
    return;
  }

  if (!im->m_active_scene || !im->m_active_scene->m_local_player->m_player_camera) {
    return;
  }

  im->m_active_scene->m_local_player->m_player_camera->m_camera_base_speed +=
      static_cast<float>(yoffset) * 0.1f;

  if (im->m_active_scene->m_local_player->m_player_camera->m_camera_base_speed < 0.1f) {
    im->m_active_scene->m_local_player->m_player_camera->m_camera_base_speed = 0.1f;
  }

  std::cout << "Camera speed: "
            << im->m_active_scene->m_local_player->m_player_camera->m_camera_base_speed << std::endl;
}

void Input_Manager::mouse_button_callback(GLFWwindow *window, int button,
                                          int action, int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && m_is_mouse_grabbed == false) {
    
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    //std::cout << "Cursor Position at ( " << xpos << " : " << ypos << " )" << std::endl;
    
    m_active_scene->m_selectionstate->mouse_pos_x = xpos;
    m_active_scene->m_selectionstate->mouse_pos_y = ypos;
    m_active_scene->m_selectionstate->launch_picker = true;
    
  }
}

void Input_Manager::mouse_callback(GLFWwindow *window, double xpos,
                                   double ypos) {

  if (!m_is_mouse_grabbed) {
    return;
  }

  float xoffset = xpos - m_lastX;
  float yoffset = m_lastY - ypos;

  if (m_first_mouse) {
    xoffset = xpos - m_lastX;
    yoffset = m_lastY - ypos;
    m_lastX = xpos;
    m_lastY = ypos;
    m_first_mouse = false;
    m_is_mouse_on_cooldown = false;
  }

  if (m_is_mouse_on_cooldown) {
    xoffset = xpos - m_lastX;
    yoffset = m_lastY - ypos;
    m_lastX = xpos;
    m_lastY = ypos;
    m_is_mouse_on_cooldown = false;
    return;
  }

  m_lastX = xpos;
  m_lastY = ypos;

  float sensitivity = 0.08f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  m_yaw += xoffset;
  m_pitch += yoffset;

  if (m_pitch > 89.0f)
    m_pitch = 89.0f;
  if (m_pitch < -89.0f)
    m_pitch = -89.0f;

  glm::vec3 m_direction;
  m_direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  m_direction.y = sin(glm::radians(m_pitch));
  m_direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
  m_active_scene->m_local_player->m_player_camera->m_cameraLookAt = glm::normalize(m_direction);
}

void Input_Manager::fb_resize_callback(GLFWwindow *window, int width, int height) {

  m_active_scene->reinit_text_vbos = true;

  m_active_scene->m_scene_framebuffer_width = width;

  m_active_scene->m_scene_framebuffer_height = height;
  
}
