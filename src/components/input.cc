#include "input.hh"
#include "logging.hh"
#include "../glad/glad.h"
#include "../libs/tiny_gltf.h"


void Input_Manager::process_input(GLFWwindow *window) {

  if (m_active_scene == nullptr || m_active_scene->m_camera == nullptr) {
    log_error("active scene is fucked. cant process inputs");
    return;
  }

  if (window == nullptr)
    log_error("window is null, cannot process input.");

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
    m_should_shutdown = true;
    log_success("shutting down window.");
  }

  float cameraSpeed =
      m_active_scene->m_camera->m_camera_base_speed * 10.0f * m_deltaTime;

  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    if (!m_last_wireframe_state) {
      m_render_mode_wireframe = !m_render_mode_wireframe;
      m_is_wireframe_on_cooldown = true;
      m_last_wireframe_state = true;
    }
  } else {
    m_last_wireframe_state = false;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    m_active_scene->m_camera->m_cameraPos +=
        cameraSpeed * glm::normalize(glm::vec3(
                          m_active_scene->m_camera->m_cameraLookAt.x, 0.0f,
                          m_active_scene->m_camera->m_cameraLookAt.z));
  }

  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
    if (m_active_scene->m_camera->m_animation_table == nullptr) {
      m_active_scene->m_camera->m_animation_table =
          new std::vector<animation *>();
      m_active_scene->m_camera->m_animation_table->clear();
      m_active_scene->m_camera->m_animation_table->reserve(1);
      m_active_scene->m_camera->m_animation_table->push_back(new animation);
      m_active_scene->m_camera->m_animation_table->at(0)->m_checkpoints =
          new std::vector<glm::vec3>();
      m_active_scene->m_camera->m_animation_table->at(0)
          ->m_checkpoints->clear();
      m_active_scene->m_camera->m_animation_table->at(0)->m_checkpoints_rot =
          new std::vector<glm::vec3>();
      m_active_scene->m_camera->m_animation_table->at(0)
          ->m_checkpoints_rot->clear();
    } else {
      printf("%d ", (int)m_active_scene->m_camera->m_animation_table->at(0)
                        ->m_checkpoints->size());
      m_active_scene->m_camera->m_animation_table->at(0)
          ->m_checkpoints->push_back(m_active_scene->m_camera->m_cameraPos);
      m_active_scene->m_camera->m_animation_table->at(0)
          ->m_checkpoints_rot->push_back(
              m_active_scene->m_camera->m_cameraLookAt);
      log_debug("saved animation point");
      //      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
    if (m_active_scene->m_camera->m_animation_table &&
        m_active_scene->m_camera->m_animation_table->at(0)->m_checkpoints) {
      m_active_scene->m_camera->m_animation_table->at(0)
          ->m_checkpoints->clear();
      m_active_scene->m_camera->m_animation_table->at(0)
          ->m_checkpoints_rot->clear();
      m_active_scene->m_camera->m_animation_table->at(0)->m_start_time = 0;
      m_active_scene->m_camera->m_animation_table->at(0)->m_has_been_smoothed =
          false;
    }
  }

  if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {

    // does an animation exist? start animation
    if (m_active_scene->m_camera->m_animation_table) {
      if (m_active_scene->m_camera->m_animation_table->at(0)
                  ->m_checkpoints->size() > 1 &&
          m_active_scene->m_camera->m_animation_table->at(0)->m_start_time ==
              0) {
        m_active_scene->m_camera->m_animation_table->at(0)
            ->m_trigger_animation = true;
        log_success("queuing animation");
      }
    }

    if (m_active_scene->m_camera->m_animation_table->at(0)
            ->m_has_been_smoothed == false) {

      for (int i = 0;
           i < ((int)m_active_scene->m_camera->m_animation_table->at(0)
                    ->m_checkpoints->size()) -
                   20;
           i++) {
        glm::vec3 step_nosmooth =
            m_active_scene->m_camera->m_animation_table->at(0)
                ->m_checkpoints->at(i);
        glm::vec3 step_next_nosmooth =
            m_active_scene->m_camera->m_animation_table->at(0)
                ->m_checkpoints->at(i + 1);
        glm::vec3 step_smoothed = (step_nosmooth + step_next_nosmooth);
        step_smoothed /= 2;
        m_active_scene->m_camera->m_animation_table->at(0)->m_checkpoints->at(
            i) = step_smoothed;
        log_error("smooting in progress");
      }
      m_active_scene->m_camera->m_animation_table->at(0)->m_has_been_smoothed =
          true;
    }

    // has an animation been set to start? initialize it + set vars
    if (m_active_scene->m_camera->m_animation_table->at(0)
            ->m_trigger_animation == true) {
      m_active_scene->m_camera->m_animation_table->at(0)->m_start_time =
          m_application_current_time;
      m_active_scene->m_camera->m_animation_table->at(0)->m_last_checkpoint = 0;
      log_success("initizlizing animation");
    }
  }

  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    //    save_frame_to_png("output.png", m_viewport_width, m_viewport_height);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    m_active_scene->m_camera->m_cameraPos +=
        cameraSpeed * glm::normalize(glm::vec3(
                          -m_active_scene->m_camera->m_cameraLookAt.x, 0.0f,
                          -m_active_scene->m_camera->m_cameraLookAt.z));
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    m_active_scene->m_camera->m_cameraPos -=
        glm::normalize(glm::cross(m_active_scene->m_camera->m_cameraLookAt,
                                  m_active_scene->m_camera->m_cameraUp)) *
        cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    m_active_scene->m_camera->m_cameraPos +=
        glm::normalize(glm::cross(m_active_scene->m_camera->m_cameraLookAt,
                                  m_active_scene->m_camera->m_cameraUp)) *
        cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    m_active_scene->m_camera->m_cameraPos +=
        glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    m_active_scene->m_camera->m_cameraPos +=
        glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)) * cameraSpeed;

  // tj camera

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

  return;
}

Input_Manager::Input_Manager(std::shared_ptr<Scene> m_scene_ptr) {

  log_success("input manger online");

  m_active_scene =  m_scene_ptr;
  
}

/*bool Input_Manager::save_frame_to_png(const char *filename, int width, int height) {
  std::vector<unsigned char> pixels(width * height * 3);

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

  std::cout << "changed camera speed to: "
            << m_active_scene->m_camera->m_camera_base_speed << std::endl;

  m_active_scene->m_camera->m_camera_base_speed +=
      static_cast<float>(yoffset) * 0.1f;

  if (m_active_scene->m_camera->m_camera_base_speed < 0.1f) {
    m_active_scene->m_camera->m_camera_base_speed = 0.1f;
  }
}

void Input_Manager::mouse_callback(GLFWwindow *window, double xpos, double ypos) {

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
  m_active_scene->m_camera->m_cameraLookAt = glm::normalize(m_direction);
}
