#include "renderer.hh"

// components
#include <chrono>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>

#include "./glad/glad.h"
#include "./libs/tiny_gltf.h"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stdlib
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

// components (custom)
#include "components/animation.hh"
#include "components/entity.hh"
#include "components/light.hh"
#include "components/logging.hh"
#include "components/material.hh"
#include "components/mesh.hh"
#include "components/scene.hh"
#include "components/utility.hh"
#include "shaders/shaderclass.hh"

#define DEF_NEAR_CLIP_PLANE 0.01f
#define DEF_FAR_CLIP_PLANE 10000.0f

/////////////////////
// CALLBACK FUNCTIONS
/////////////////////

void Renderer::scroll_callback(GLFWwindow *window, double xoffset,
                               double yoffset) {

  std::cout << "changed camera speed to: "
            << m_active_scene->m_camera->m_camera_base_speed << std::endl;

  m_active_scene->m_camera->m_camera_base_speed +=
      static_cast<float>(yoffset) * 0.1f;

  if (m_active_scene->m_camera->m_camera_base_speed < 0.1f) {
    m_active_scene->m_camera->m_camera_base_speed = 0.1f;
  }
}

void Renderer::mouse_callback(GLFWwindow *window, double xpos, double ypos) {

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

void Renderer::processInput(GLFWwindow *window) {

  if (m_active_scene->m_camera == nullptr)
    return;

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
    save_frame_to_png("output.png", m_viewport_width, m_viewport_height);
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

void Renderer::calculate_phys_boxes() {
  //buffer
  std::vector<Mesh> mesh_buffer;
  
  std::cout << "scene contains entities: " << m_active_scene->m_loaded_entities.size() << std::endl;
  for (Entity &entity : m_active_scene->m_loaded_entities) {

    glm::mat4 entity_mat = entity.m_model_matrix;
    std::cout << "entity contains meshes: " << entity.m_mesh.size() << std::endl;

    for (Mesh &mesh : entity.m_mesh) {

      if (mesh.m_type == E_MESH) {
	
	//prevent oob
	if (mesh.m_vertices_array.size() < 3) {
	  log_error("attempted to hitbox a plane??? idiot");
	  continue;
	}
	
	// calc hitbox
        float max_x = -10000.0f, max_y = -10000.0f, max_z = -10000.0f;
        float min_x = 10000.0f, min_y = 10000.0f, min_z = 10000.0f;
	glm::mat4 mesh_mat = mesh.m_model_matrix;
	
	//prep shit for box oop
        Shader shader_to_use("src/shaders/shader_src/wireframe.vert",
                             "src/shaders/shader_src/wireframe.frag");
        Material new_material(E_PHONG, shader_to_use);
        Mesh new_mesh(new_material);
        new_mesh.m_render_mode = E_WIREFRAME;
        new_mesh.m_type = E_COL_BOX;

	std::cout << "size item :" << mesh.m_vertices_array.size() << std::endl;

	//transform hitbox to world coordinates
	glm::mat4 trans_mat = entity_mat * mesh_mat;
	
	for (int i = 0; i + 2 < (int)mesh.m_vertices_array.size(); i += 3) {

	  glm::vec4 to_test_vec = glm::vec4(mesh.m_vertices_array[i],mesh.m_vertices_array[i+1],mesh.m_vertices_array[i+2],1.0f);

	  to_test_vec = trans_mat * to_test_vec;
	  
          if (to_test_vec.x < min_x)
            min_x = to_test_vec.x;
          if (to_test_vec.x > max_x)
            max_x = to_test_vec.x;
          if (to_test_vec.y < min_y)
            min_y = to_test_vec.y;
          if (to_test_vec.y > max_y)
            max_y = to_test_vec.y;
          if (to_test_vec.z < min_z)
            min_z = to_test_vec.z;
          if (to_test_vec.z> max_z)
            max_z = to_test_vec.z;
	}

        new_mesh.m_vertices_array = {
            // Front face (z = max_z)
            min_x,
            min_y,
            max_z,
            max_x,
            min_y,
            max_z,
            max_x,
            max_y,
            max_z,

            max_x,
            max_y,
            max_z,
            min_x,
            max_y,
            max_z,
            min_x,
            min_y,
            max_z,

            // Back face (z = min_z)
            max_x,
            min_y,
            min_z,
            min_x,
            min_y,
            min_z,
            min_x,
            max_y,
            min_z,

            min_x,
            max_y,
            min_z,
            max_x,
            max_y,
            min_z,
            max_x,
            min_y,
            min_z,

            // Left face (x = min_x)
            min_x,
            min_y,
            min_z,
            min_x,
            min_y,
            max_z,
            min_x,
            max_y,
            max_z,

            min_x,
            max_y,
            max_z,
            min_x,
            max_y,
            min_z,
            min_x,
            min_y,
            min_z,

            // Right face (x = max_x)
            max_x,
            min_y,
            max_z,
            max_x,
            min_y,
            min_z,
            max_x,
            max_y,
            min_z,

            max_x,
            max_y,
            min_z,
            max_x,
            max_y,
            max_z,
            max_x,
            min_y,
            max_z,

            // Top face (y = max_y)
            min_x,
            max_y,
            max_z,
            max_x,
            max_y,
            max_z,
            max_x,
            max_y,
            min_z,

            max_x,
            max_y,
            min_z,
            min_x,
            max_y,
            min_z,
            min_x,
            max_y,
            max_z,

            // Bottom face (y = min_y)
            min_x,
            min_y,
            min_z,
            max_x,
            min_y,
            min_z,
            max_x,
            min_y,
            max_z,

            max_x,
            min_y,
            max_z,
            min_x,
            min_y,
            max_z,
            min_x,
            min_y,
            min_z,
        };

        mesh_buffer.push_back(new_mesh);

	log_success("calculated hitbox for mesh!");
	
      }
    }
  }

  for(Mesh m : mesh_buffer) {
    m_active_scene->m_loaded_entities[0].m_mesh.push_back(m);
  }
  
}

void Renderer::handle_scene_physics() { return; }

void Renderer::setup_render_properties() {

  // render mode
  if (m_render_mode_wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  return;
}

bool Renderer::save_frame_to_png(const char *filename, int width, int height) {
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
}

void Renderer::framebuffer_size_callback(GLFWwindow *window, int width,
                                         int height) {
  log_success("framebuffer resized.");
  glViewport(0, 0, width, height);
}

void Renderer::handle_scene_animations() {

  // is an animation already running? animate it
  if (m_active_scene->m_camera->m_animation_table &&
      m_active_scene->m_camera->m_animation_table->at(0)->m_trigger_animation) {

    if (m_active_scene->m_camera->m_animation_table->at(0)->m_start_time <
            m_application_current_time &&
        m_active_scene->m_camera->m_animation_table->at(0)->m_start_time != 0) {

      float start_time =
          m_active_scene->m_camera->m_animation_table->at(0)->m_start_time;
      float num_checkpoints = m_active_scene->m_camera->m_animation_table->at(0)
                                  ->m_checkpoints->size();
      float anim_speed =
          m_active_scene->m_camera->m_animation_table->at(0)->m_animation_speed;
      float delta =
          (m_active_scene->m_camera->m_animation_table->at(0)->m_start_time *
               anim_speed +
           num_checkpoints) -
          m_application_current_time * anim_speed;

      std::cout << start_time << " start_time " << std::endl;
      std::cout << anim_speed << " anim speed " << std::endl;
      std::cout << delta << " delta " << std::endl;
      std::cout << num_checkpoints << " num_checkpoint " << std::endl;

      // animate
      unsigned int tomove_check = std::ceil(num_checkpoints - delta);
      float remainder = tomove_check - (num_checkpoints - delta);
      std::cout << remainder << "remainder" << std::endl;
      if (tomove_check > num_checkpoints - 1) {
        // done animating? reset.
        tomove_check = num_checkpoints - 1;
        log_error("end of anim reached?");
        m_active_scene->m_camera->m_animation_table->at(0)
            ->m_trigger_animation = false;
        m_active_scene->m_camera->m_animation_table->at(0)->m_start_time = 0;
        log_success("animation done!");
      }

      unsigned int tomove_next = tomove_check + 1;
      if (tomove_next > num_checkpoints - 1)
        tomove_next = num_checkpoints - 1;

      glm::vec3 old_campos_anim =
          m_active_scene->m_camera->m_animation_table->at(0)->m_checkpoints->at(
              tomove_check);
      glm::vec3 next_campos_anim =
          m_active_scene->m_camera->m_animation_table->at(0)->m_checkpoints->at(
              tomove_next);
      glm::vec3 old_campos_anim_rot =
          m_active_scene->m_camera->m_animation_table->at(0)
              ->m_checkpoints_rot->at(tomove_check);
      glm::vec3 next_campos_anim_rot =
          m_active_scene->m_camera->m_animation_table->at(0)
              ->m_checkpoints_rot->at(tomove_next);

      glm::vec3 interpolated_camera_pos =
          (remainder * old_campos_anim) + ((1 - remainder) * next_campos_anim);
      glm::vec3 interpolated_camera_rot =
          (remainder * old_campos_anim_rot) +
          ((1 - remainder) * next_campos_anim_rot);

      m_active_scene->m_camera->m_cameraPos = interpolated_camera_pos;
      m_active_scene->m_camera->m_cameraLookAt = interpolated_camera_rot;

      std::cout << tomove_check << " tomove_check " << std::endl;

      log_success("anim step done!");

    } else {
      log_error("no animation running");
    }
  }
}

void Renderer::render_frame() {

  if (!m_active_scene->m_camera) {
    log_error("no camera in scene! stopping render!");
    return;
  }
  if (m_active_scene->m_loaded_lights.size() < 1) {
    log_error(
        "not enough lights loaded for shader to function. stopping render.");
    return;
  }
  if (m_active_scene->m_loaded_entities.size() < 1) {
    log_error(
        "not enough lights loaded for shader to function. stopping render.");
    return;
  }

  setup_render_properties();
  handle_scene_animations();
  handle_scene_physics();
  processInput(associated_window);

  // setup constants for render pass
  glfwGetWindowSize(associated_window, &m_viewport_width, &m_viewport_height);

  // TMP make light spin and move
  // m_active_scene->m_loaded_lights[0].m_light_matrix =
  // glm::rotate(glm::translate(glm::mat4(1.0f),glm::vec3(sin(m_application_current_time)
  // * 4.5,5,5)),sin(m_application_current_time) * 0.5f,glm::vec3(0, 1, 0));
  // m_active_scene->m_loaded_lights[0].m_light_matrix = glm::rotate(
  //  glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-2, 5, 3)), -2.5f,
  //              glm::vec3(0, 1, 0)),
  //  -1.0f, glm::vec3(1, 0, 0));
  // ENDTMP

  depth_shader->use();

  // configure spotlight shadow mapping
  glm::vec3 light_pos_new =
      m_active_scene->m_loaded_lights[0].get_light_position();
  glm::mat3 light_rotation =
      m_active_scene->m_loaded_lights[0].get_light_rotation_matrix();

  glm::mat4 light_look_at = glm::lookAt(
      light_pos_new,
      light_pos_new + glm::normalize(light_rotation * glm::vec3(0, 0, -1)),
      glm::vec3(0.0f, 1.0f, 0.0f));

  // use for sanity
  float width = m_active_scene->m_loaded_lights[0].light_width;
  glm::mat4 light_projection_mat =
      glm::ortho(-width, width, -width, width, 0.01f, 20.0f);

  // glm::mat4 light_projection_mat = glm::perspective(glm::radians(90.0f),
  // (float)shadow_width / (float)shadow_height, 0.1f,50.0f);
  glm::mat4 light_space_matrix = light_projection_mat * light_look_at;

  depth_shader->use();

  glViewport(0, 0, shadow_map_width, shadow_map_height);
  glBindFramebuffer(GL_FRAMEBUFFER, window_depth_map_fbo);
  glClear(GL_DEPTH_BUFFER_BIT);

  check_gl_error("after setting viewport stuff up (depth)");

  // render scene from light pov
  for (auto &entity : m_active_scene->m_loaded_entities) {
    for (auto &mesh : entity.m_mesh) {

      // bind meshes vao context
      glBindVertexArray(mesh.m_mesh_vao);
      if (glIsVertexArray(mesh.m_mesh_vao) == GL_FALSE) {
        log_error("no valid VAO id! cant render mesh.");
      }

      check_gl_error("before setting uniforms (depth)");

      upload_to_uniform("model", depth_shader->ID,
                        entity.m_model_matrix * mesh.m_model_matrix);
      upload_to_uniform("light_space_matrix", depth_shader->ID,
                        light_space_matrix);

      check_gl_error("after setting uniforms (depth)");

      // we renderin
      glDrawArrays(GL_TRIANGLES, 0, mesh.m_vertices_array.size() / 3);

      check_gl_error("after glDrawArrays (depth)");
    }
  }

  // rebind old fb
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // render scene with old settings
  glViewport(0, 0, m_viewport_width, m_viewport_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  check_gl_error("after clearing frame");

  // bungie employees hate this simple trick
  float currentFrame = glfwGetTime();
  m_deltaTime = currentFrame - m_application_current_time;
  m_application_current_time = currentFrame;

  glm::mat4 view_mat = glm::lookAt(m_active_scene->m_camera->m_cameraPos,
                                   m_active_scene->m_camera->m_cameraLookAt +
                                       m_active_scene->m_camera->m_cameraPos,
                                   m_active_scene->m_camera->m_cameraUp);

  // projection matrix
  glm::mat4 projection_mat = glm::perspective(
      glm::radians(90.0f), (float)m_viewport_width / (float)m_viewport_height,
      DEF_NEAR_CLIP_PLANE, DEF_FAR_CLIP_PLANE);

  // render meshes
  for (auto &entity : m_active_scene->m_loaded_entities) {
    for (auto &mesh : entity.m_mesh) {

      //change hitbox or flat style
      if(mesh.m_render_mode == E_WIREFRAME)
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      
      // bind meshes vao context
      glBindVertexArray(mesh.m_mesh_vao);
      if (glIsVertexArray(mesh.m_mesh_vao) == GL_FALSE) {
        log_error("no valid VAO id! cant render mesh.");
      }

      check_gl_error("after binding vao");

      mesh.m_material.m_shader.use();

      check_gl_error("after setting shader active");

      if (mesh.m_material.m_material_type == E_PBR_TEX) {

        // bind texture to uniform
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.m_material.bound_texture_id);
        GLint loc_tex =
            glGetUniformLocation(mesh.m_material.m_shader.ID, "uTexture");
        glUniform1i(loc_tex, 0);
        // bind depth map to uniform
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, window_depth_map);
        GLint loc_depth =
            glGetUniformLocation(mesh.m_material.m_shader.ID, "uDepthMap");
        glUniform1i(loc_depth, 1);

        check_gl_error("after uploading textures");
      }

      // TMP ghetto light + color
      glm::vec3 light_position =
          m_active_scene->m_loaded_lights[0].get_light_position();

      upload_to_uniform("objectColor", mesh.m_material.m_shader.ID,
                        glm::vec3(0.5, 0.8, 0.2));
      upload_to_uniform("lightColor", mesh.m_material.m_shader.ID,
                        glm::vec3(0.8, 0.8, 0.8));

      upload_to_uniform("model", mesh.m_material.m_shader.ID,
                        entity.m_model_matrix * mesh.m_model_matrix);

      upload_to_uniform("view", mesh.m_material.m_shader.ID, view_mat);
      upload_to_uniform("viewPosition", mesh.m_material.m_shader.ID,
                        m_active_scene->m_camera->m_cameraPos);
      upload_to_uniform("projection", mesh.m_material.m_shader.ID,
                        projection_mat);
      upload_to_uniform("lightPosition", mesh.m_material.m_shader.ID,
                        light_position);
      upload_to_uniform("viewPos", mesh.m_material.m_shader.ID,
                        m_active_scene->m_camera->m_cameraPos);

      upload_to_uniform("light_space_matrix", mesh.m_material.m_shader.ID,
                        light_space_matrix);

      check_gl_error("after setting uniforms");

      // we renderin
      glDrawArrays(GL_TRIANGLES, 0, mesh.m_vertices_array.size() / 3);

      check_gl_error("after glDrawArrays");
    }
  }

  ////////////////////////
  // finally draw visualizers for all lights in the scene
  ///////////////////////
  for (auto &light_source : m_active_scene->m_loaded_lights) {

    // bind meshes vao context
    glBindVertexArray(light_source.m_light_visualizer_mesh.m_mesh_vao);
    if (glIsVertexArray(light_source.m_light_visualizer_mesh.m_mesh_vao) ==
        GL_FALSE) {
      log_error("no valid VAO id! cant render mesh.");
    }

    check_gl_error("after binding vao (lights)");

    light_source.m_light_visualizer_mesh.m_material.m_shader.use();

    check_gl_error("after setting shader active (lights)");

    if (light_source.m_light_visualizer_mesh.m_material.m_material_type ==
        E_PBR_TEX) {

      // bind texture to uniform
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(
          GL_TEXTURE_2D,
          light_source.m_light_visualizer_mesh.m_material.bound_texture_id);
      GLint loc_tex = glGetUniformLocation(
          light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
          "uTexture");
      glUniform1i(loc_tex, 0);
      // bind depth map to uniform
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, window_depth_map);
      GLint loc_depth = glGetUniformLocation(
          light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
          "uDepthMap");
      glUniform1i(loc_depth, 1);

      check_gl_error("after uploading textures");
    }

    upload_to_uniform(
        "objectColor",
        light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
        glm::vec3(0.5, 0.8, 0.2));
    upload_to_uniform(
        "lightColor",
        light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
        glm::vec3(0.8, 0.8, 0.8));
    upload_to_uniform(
        "model", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
        light_source.m_light_matrix);

    upload_to_uniform(
        "view", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
        view_mat);
    upload_to_uniform(
        "viewPosition",
        light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
        m_active_scene->m_camera->m_cameraPos);
    upload_to_uniform(
        "projection",
        light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
        projection_mat);
    upload_to_uniform(
        "lightPosition",
        light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
        glm::vec3(0.0f));
    upload_to_uniform(
        "viewPos", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
        m_active_scene->m_camera->m_cameraPos);
    upload_to_uniform(
        "light_space_matrix",
        light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
        light_space_matrix);

    check_gl_error("after setting uniforms");

    // we renderin
    glDrawArrays(GL_TRIANGLES, 0,
                 light_source.m_light_visualizer_mesh.m_vertices_array.size() /
                     3);

    check_gl_error("after glDrawArrays (lights)");
  }

  // draw to screen
  glfwSwapBuffers(associated_window);
  glfwPollEvents();
}

void Renderer::init_scene(const char *scene_fp) {

  Entity load_entity;
  load_entity.m_mesh = std::move(
      load_all_meshes_from_gltf(scene_fp, num_loaded_textures, m_texture_map));

  //  glDisable(GL_CULL_FACE);

  m_active_scene = std::move(std::make_unique<Scene>());

  m_active_scene->m_camera = std::move(std::make_unique<Camera>());

  Light main_light(std::move(load_all_meshes_from_gltf(
      "models/light/scene.gltf", num_loaded_textures, m_texture_map))[0]);
  main_light.m_light_type = E_POINT_LIGHT;
  main_light.m_color = 0xFFFFFF;
  main_light.m_strength = 10;

  main_light.m_light_matrix =
      glm::translate(main_light.m_light_matrix, glm::vec3(10.0f, 2.0f, 1.0f));

  m_active_scene->add_entity_to_scene(load_entity);
  m_active_scene->add_light_to_scene(main_light);

  //calc physboxes for meshes
  calculate_phys_boxes();
  
  // initialize scene vbos
  init_scene_vbos();

  // Initialize shader programs
  log_debug("Initializing Shader Programs for scene...");
  for (auto &entity_to_render : m_active_scene->m_loaded_entities) {
    for (auto &mesh_of_entity : entity_to_render.m_mesh) {
      mesh_of_entity.m_material.m_shader.use();
    }
  }
  log_success("Finished initialization for Shader Programs");

  // SHADOW MAPPING
  glGenFramebuffers(1, &window_depth_map_fbo);

  glGenTextures(1, &window_depth_map);
  glBindTexture(GL_TEXTURE_2D, window_depth_map);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadow_map_width,
               shadow_map_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindFramebuffer(GL_FRAMEBUFFER, window_depth_map_fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         window_depth_map, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  depth_shader = new Shader("src/shaders/shader_src/depth.vert",
                            "src/shaders/shader_src/depth.frag");
  
  log_success("done initializing renderer.");
}

void Renderer::init_scene_vbos() {

  if (m_active_scene->m_loaded_entities.size() > 0 &&
      m_active_scene->m_loaded_lights.size() > 0) {

    log_debug("Initializing VBOs for Lights...");

    ///////////////////////
    // create vbos for the  mesh (legit only need one lol)
    ///////////////////////
    Light &light_source = m_active_scene->m_loaded_lights[0];
    // vao
    glGenVertexArrays(1, &light_source.m_light_visualizer_mesh.m_mesh_vao);

    glBindVertexArray(light_source.m_light_visualizer_mesh.m_mesh_vao);

    // vbo mesh (vertices)
    glGenBuffers(1, &light_source.m_light_visualizer_mesh.m_vertices_glid);
    glBindBuffer(GL_ARRAY_BUFFER,
                 light_source.m_light_visualizer_mesh.m_vertices_glid);
    glBufferData(GL_ARRAY_BUFFER,
                 light_source.m_light_visualizer_mesh.m_vertices_array.size() *
                     sizeof(float),
                 light_source.m_light_visualizer_mesh.m_vertices_array.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &light_source.m_light_visualizer_mesh.m_tex_coords_glid);
    glBindBuffer(GL_ARRAY_BUFFER,
                 light_source.m_light_visualizer_mesh.m_tex_coords_glid);
    glBufferData(
        GL_ARRAY_BUFFER,
        light_source.m_light_visualizer_mesh.m_tex_coords_array.size() *
            sizeof(float),
        light_source.m_light_visualizer_mesh.m_tex_coords_array.data(),
        GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ///////////////////////
    // now load all the data for the rest of the scene
    ///////////////////////
    log_debug("Initializing VBOs for scene...");
    for (auto &entity_to_render : m_active_scene->m_loaded_entities) {
      log_debug_sub("Found active scene.");
      for (auto &mesh_of_entity : entity_to_render.m_mesh) {

        if (!mesh_of_entity.m_mesh_vbo_needs_refresh)
          return;

        log_debug_sub("Found mesh in active scene.");

        // generate missing geometry if its missing (500iq)
        if (mesh_of_entity.m_normals_array.size() < 1) {
          log_debug("mesh doesnt have normals, calculating them now!");
          mesh_of_entity.m_normals_array =
              calculate_vert_normals(mesh_of_entity.m_vertices_array);
        }

        if (mesh_of_entity.m_tex_coords_array.size() > 0) {

          // calculate vertex tangents / binormals if they are missing and
          // texture coords are present
          if (mesh_of_entity.m_binormals_array.size() < 2) {

            log_debug(
                "texture coords available, but not tangents. calculating.");

            tan_bin_glob retglob = calculate_vert_tan_bin(
                mesh_of_entity.m_vertices_array, mesh_of_entity.m_normals_array,
                mesh_of_entity.m_tex_coords_array);

            mesh_of_entity.m_binormals_array = retglob.vert_binormals;
            mesh_of_entity.m_tangents_array = retglob.vert_tangents;
          }

        } else {

          log_error("imported mesh has missing UV coordinates, using fallback "
                    "coords.");

          mesh_of_entity.m_binormals_array.resize(
              mesh_of_entity.m_vertices_array.size());
          mesh_of_entity.m_tangents_array.resize(
              mesh_of_entity.m_vertices_array.size());
        }

        // vao
        glGenVertexArrays(1, &mesh_of_entity.m_mesh_vao);
        glBindVertexArray(mesh_of_entity.m_mesh_vao);

        // vbo mesh (vertices)
        glGenBuffers(1, &mesh_of_entity.m_vertices_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_vertices_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh_of_entity.m_vertices_array.size() * sizeof(float),
                     mesh_of_entity.m_vertices_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // vbo mesh (tex coords)
        glGenBuffers(1, &mesh_of_entity.m_tex_coords_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_tex_coords_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh_of_entity.m_tex_coords_array.size() * sizeof(float),
                     mesh_of_entity.m_tex_coords_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // vbo mesh (normals)
        glGenBuffers(1, &mesh_of_entity.m_normals_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_normals_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh_of_entity.m_normals_array.size() * sizeof(float),
                     mesh_of_entity.m_normals_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // vbo mesh (tangents)
        glGenBuffers(1, &mesh_of_entity.m_tangents_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_tangents_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh_of_entity.m_tangents_array.size() * sizeof(float),
                     mesh_of_entity.m_tangents_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // vbo mesh (bitangents)
        glGenBuffers(1, &mesh_of_entity.m_binormals_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_binormals_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh_of_entity.m_binormals_array.size() * sizeof(float),
                     mesh_of_entity.m_binormals_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(4);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        mesh_of_entity.m_mesh_vbo_needs_refresh = false;
      }
    }

    log_success("done initializing VBOs for all entities!");

  } else {

    log_error("scene doesnt contain at least one light + entity, not "
              "initializing vbos");
  }
}

template <typename T>
void Renderer::upload_to_uniform(std::string location, GLuint shader_id,
                                 T input) {

  GLuint loc = glGetUniformLocation(shader_id, location.c_str());

  if constexpr (std::is_same<T, glm::mat4>::value) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(input));
  } else

      if constexpr (std::is_same<T, glm::vec3>::value) {
    glUniform3fv(loc, 1, glm::value_ptr(input));
  } else

      if constexpr (std::is_same<T, glm::mat3>::value) {
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(input));
  } else {
    log_error("unknown datatype passed to uniform!");
  }
};

Renderer::Renderer(uint window_width, uint window_height) {

  std::cout << R"(                     __                 
  ____  ____________/  |_  ____ ___  ___
_/ ___\/  _ \_  __ \   __\/ __ \\  \/  /
\  \__(  <_> )  | \/|  | \  ___/ >    < 
 \___  >____/|__|   |__|  \___  >__/\_ \
     \/                       \/      \/
)";

  log_debug("initializing window");

  // Create the window for this renderer
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  associated_window = glfwCreateWindow(window_width, window_height,
                                       "cortex - dev build", NULL, NULL);
  if (associated_window == NULL) {
    log_error("failed to create glfw window!");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(associated_window);

  // initiate glad
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    log_error("failed to load glad!");
    return;
  }

  // setup
  glViewport(0, 0, m_viewport_width, m_viewport_height);
  glEnable(GL_DEPTH_TEST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // format for class usage
  glfwSetFramebufferSizeCallback(associated_window,
                                 Renderer::framebuffer_size_callback);
  glfwSetWindowUserPointer(associated_window, this);

  // set callbacks using lambda functions
  glfwSetScrollCallback(associated_window, [](GLFWwindow *w, double xoffset,
                                              double yoffset) {
    Renderer *renderer = static_cast<Renderer *>(glfwGetWindowUserPointer(w));
    if (renderer) {
      renderer->scroll_callback(w, xoffset, yoffset);
    }
  });
  glfwSetCursorPosCallback(associated_window, [](GLFWwindow *w, double xpos,
                                                 double ypos) {
    Renderer *renderer = static_cast<Renderer *>(glfwGetWindowUserPointer(w));
    if (renderer) {
      renderer->mouse_callback(w, xpos, ypos);
    }
  });

  glfwSetInputMode(associated_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  return;
}
