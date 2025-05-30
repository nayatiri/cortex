#include "renderer.hh"

// components
#include <glm/matrix.hpp>
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
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// components (custom)
#include "./components/entity.hh"
#include "./components/importer.hh"
#include "./components/material.hh"
#include "./components/utility.hh"
#include "components/light.hh"
#include "components/logging.hh"
#include "components/scene.hh"
#include "shaders/shaderclass.hh"

#define DEF_NEAR_CLIP_PLANE 0.01f
#define DEF_FAR_CLIP_PLANE 10000.0f

/////////////////////
// CALLBACK FUNCTIONS
/////////////////////

void Renderer::scroll_callback(GLFWwindow *window, double xoffset,
                               double yoffset) {

  Renderer *renderer =
      static_cast<Renderer *>(glfwGetWindowUserPointer(window));

  std::cout << "changed camera speed to: " << m_camera_base_speed << std::endl;

  m_camera_base_speed += static_cast<float>(yoffset) * 0.1f;

  if (m_camera_base_speed < 0.1f) {
    m_camera_base_speed = 0.1f;
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
  m_cameraLookAt = glm::normalize(m_direction);
}

void Renderer::framebuffer_size_callback(GLFWwindow *window, int width,
                                         int height) {
  log_success("framebuffer resized.");
  glViewport(0, 0, width, height);
}

void Renderer::processInput(GLFWwindow *window) {

  if (window == nullptr)
    log_error("window is null, cannot process input.");

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
    m_should_shutdown = true;
    log_success("shutting down window.");
  }

  float cameraSpeed = m_camera_base_speed * 10.0f * m_deltaTime;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    m_cameraPos +=
        cameraSpeed *
        glm::normalize(glm::vec3(m_cameraLookAt.x, 0.0f, m_cameraLookAt.z));
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    m_cameraPos +=
        cameraSpeed *
        glm::normalize(glm::vec3(-m_cameraLookAt.x, 0.0f, -m_cameraLookAt.z));
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    m_cameraPos -=
        glm::normalize(glm::cross(m_cameraLookAt, m_cameraUp)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    m_cameraPos +=
        glm::normalize(glm::cross(m_cameraLookAt, m_cameraUp)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    m_cameraPos += glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    m_cameraPos += glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)) * cameraSpeed;

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

void Renderer::init_scene_vbos() {

  if (m_active_scene.m_loaded_entities.size() > 0 &&
      m_active_scene.m_loaded_lights.size() > 0) {

    log_debug("Initializing VBOs for Lights...");

    ///////////////////////
    // create vbos for the light mesh (legit only need one lol)
    ///////////////////////
    Light &light_source = m_active_scene.m_loaded_lights[0];
    // vao
    glGenVertexArrays(1, &light_source.m_light_visualizer_mesh.m_mesh_vao);

    std::cout << light_source.m_light_visualizer_mesh.m_mesh_vao << " tj light" << std::endl;

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
    for (auto &entity_to_render : m_active_scene.m_loaded_entities) {
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
  
  m_cameraPos = glm::vec3(1.0f);

  log_debug("initializing window");

  // Create the window for this renderer
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(window_width, window_height,
                                        "cortex - dev build", NULL, NULL);
  if (window == NULL) {
    log_error("failed to create glfw window!");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(window);

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
  glfwSetFramebufferSizeCallback(window, Renderer::framebuffer_size_callback);
  glfwSetWindowUserPointer(window, this);

  // set callbacks using lambda functions
  glfwSetScrollCallback(window, [](GLFWwindow *w, double xoffset,
                                   double yoffset) {
    Renderer *renderer = static_cast<Renderer *>(glfwGetWindowUserPointer(w));
    if (renderer) {
      renderer->scroll_callback(w, xoffset, yoffset);
    }
  });
  glfwSetCursorPosCallback(window, [](GLFWwindow *w, double xpos, double ypos) {
    Renderer *renderer = static_cast<Renderer *>(glfwGetWindowUserPointer(w));
    if (renderer) {
      renderer->mouse_callback(w, xpos, ypos);
    }
  });

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // debug render mode
  if (m_render_mode_wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  /// SCENE SETUP

  Entity second_entity;
  //  second_entity.m_mesh =
  //  std::move(load_all_meshes_from_gltf("models/levi_entrance/levi_entrance.gltf",
  //  num_loaded_textures, m_texture_map));
  second_entity.m_mesh = std::move(load_all_meshes_from_gltf(
      "models/potter/scene.gltf", num_loaded_textures, m_texture_map));

  // TMP
  glDisable(GL_CULL_FACE);

  Scene main_scene;
  m_active_scene = main_scene;

  Light main_light(std::move(load_all_meshes_from_gltf(
      "models/light/scene.gltf", num_loaded_textures, m_texture_map))[0]);
  main_light.m_light_type = E_POINT_LIGHT;
  main_light.m_color = 0xFFFFFF;
  main_light.m_strength = 10;
  main_light.m_light_matrix =
      glm::translate(main_light.m_light_matrix, glm::vec3(10.0f, 10.0f, 0.0f));

  m_active_scene.add_entity_to_scene(second_entity);
  m_active_scene.add_light_to_scene(main_light);

  /// END SCENE SETUP

  // initialize scene vbos
  init_scene_vbos();

  //////////////////////////////
  // Initialize shader programs
  //////////////////////////////

  log_debug("Initializing Shader Programs for scene...");
  for (auto &entity_to_render : m_active_scene.m_loaded_entities) {
    log_debug_sub("Found active scene.");
    for (auto &mesh_of_entity : entity_to_render.m_mesh) {
      log_debug_sub("Found meshmaterial in active scene.");

      mesh_of_entity.m_material.m_shader.use();
    }
  }

  // SHADOW MAPPING

  unsigned int depthMapFBO;
  glGenFramebuffers(1, &depthMapFBO);

  const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

  unsigned int depthMap;
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
               SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {0.0, 0.0, 0.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  Shader depth_shader("src/shaders/shader_src/depth.vert",
                      "src/shaders/shader_src/depth.frag");
  depth_shader.use();

  // main render loop
  while (!glfwWindowShouldClose(window)) {

    processInput(window);

    // configure spotlight shadow mapping
    glm::vec3 light_pos_new = glm::vec3(0.0f, 1.0f, 5.0f);
    glm::mat4 light_look_at = glm::lookAt(
        light_pos_new,
        light_pos_new + glm::vec3(cos(m_lastFrame), 0.0f, sin(m_lastFrame)),
        glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 light_projection_mat = glm::perspective(
        glm::radians(120.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, 0.1f,
        100.0f);

    glm::mat4 light_space_matrix = light_projection_mat * light_look_at;

    depth_shader.use();

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    check_gl_error("after setting viewport stuff up (depth)");

    // render scene from light pov
    for (auto &entity : m_active_scene.m_loaded_entities) {
      for (auto &mesh : entity.m_mesh) {

        // bind meshes vao context
        glBindVertexArray(mesh.m_mesh_vao);
        if (glIsVertexArray(mesh.m_mesh_vao) == GL_FALSE) {
          log_error("no valid VAO id! cant render mesh.");
        }

        check_gl_error("before setting uniforms (depth)");

        upload_to_uniform("model", depth_shader.ID,
                          entity.m_model_matrix * mesh.m_model_matrix);
        upload_to_uniform("light_space_matrix", depth_shader.ID,
                          light_space_matrix);

        check_gl_error("after setting uniforms (depth)");

        // we renderin
        glDrawArrays(GL_TRIANGLES, 0, mesh.m_vertices_array.size() / 3);

        check_gl_error("after glDrawArrays (depth)");
      }
    }

    // rebind old fb
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render scene

    glViewport(0, 0, m_viewport_width, m_viewport_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    check_gl_error("after clearing frame");

    // bungie employees hate this simple trick
    float currentFrame = glfwGetTime();
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;

    glm::mat4 view_mat =
        glm::lookAt(m_cameraPos, m_cameraLookAt + m_cameraPos, m_cameraUp);

    // projection matrix
    glfwGetWindowSize(window, &m_viewport_width, &m_viewport_height);
    glm::mat4 projection_mat = glm::perspective(
        glm::radians(90.0f), (float)m_viewport_width / (float)m_viewport_height,
        DEF_NEAR_CLIP_PLANE, DEF_FAR_CLIP_PLANE);

    // render meshes
    for (auto &entity : m_active_scene.m_loaded_entities) {
      for (auto &mesh : entity.m_mesh) {

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
          glBindTexture(GL_TEXTURE_2D, depthMap);
          GLint loc_depth =
              glGetUniformLocation(mesh.m_material.m_shader.ID, "uDepthMap");
          glUniform1i(loc_depth, 1);

          check_gl_error("after uploading textures");
        }

        // TMP ghetto light + color
        m_active_scene.m_loaded_lights[0].m_light_matrix = glm::translate(
            glm::mat4(1.0f), glm::vec3(cos(m_lastFrame / 3) * 10, 2.0f,
                                       sin(m_lastFrame / 3) * 10));
        glm::vec3 light_position(
            m_active_scene.m_loaded_lights[0].m_light_matrix[3][0],
            m_active_scene.m_loaded_lights[0].m_light_matrix[3][1],
            m_active_scene.m_loaded_lights[0].m_light_matrix[3][2]);
        upload_to_uniform("objectColor", mesh.m_material.m_shader.ID,
                          glm::vec3(0.5, 0.8, 0.2));
        upload_to_uniform("lightColor", mesh.m_material.m_shader.ID,
                          glm::vec3(0.8, 0.8, 0.8));

        upload_to_uniform("model", mesh.m_material.m_shader.ID,
                          entity.m_model_matrix * mesh.m_model_matrix);

        upload_to_uniform("view", mesh.m_material.m_shader.ID, view_mat);
        upload_to_uniform("viewPosition", mesh.m_material.m_shader.ID,
                          m_cameraPos);
        upload_to_uniform("projection", mesh.m_material.m_shader.ID,
                          projection_mat);
        upload_to_uniform("lightPosition", mesh.m_material.m_shader.ID,
                          light_position);
        upload_to_uniform("viewPos", mesh.m_material.m_shader.ID, m_cameraPos);

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
    for (auto &light_source : m_active_scene.m_loaded_lights) {

      std::cout << light_source.m_light_visualizer_mesh.m_mesh_vao << " erm " <<std::endl;
      
      // bind meshes vao context
      glBindVertexArray(light_source.m_light_visualizer_mesh.m_mesh_vao);
      if (glIsVertexArray(light_source.m_light_visualizer_mesh.m_mesh_vao) == GL_FALSE) {
        log_error("no valid VAO id! cant render mesh.");
      }

      check_gl_error("after binding vao (lights)");

      light_source.m_light_visualizer_mesh.m_material.m_shader.use();

      check_gl_error("after setting shader active (lights)");

      if (light_source.m_light_visualizer_mesh.m_material.m_material_type == E_PBR_TEX) {

        // bind texture to uniform
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, light_source.m_light_visualizer_mesh.m_material.bound_texture_id);
        GLint loc_tex =
            glGetUniformLocation(light_source.m_light_visualizer_mesh.m_material.m_shader.ID, "uTexture");
        glUniform1i(loc_tex, 0);
        // bind depth map to uniform
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        GLint loc_depth =
            glGetUniformLocation(light_source.m_light_visualizer_mesh.m_material.m_shader.ID, "uDepthMap");
        glUniform1i(loc_depth, 1);

        check_gl_error("after uploading textures");
      }

      glm::vec3 light_position(
          light_source.m_light_matrix[3][0],
          light_source.m_light_matrix[3][1],
          light_source.m_light_matrix[3][2]);
      
      upload_to_uniform("objectColor", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
                        glm::vec3(0.5, 0.8, 0.2));
      upload_to_uniform("lightColor", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
                        glm::vec3(0.8, 0.8, 0.8));

      upload_to_uniform("model", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
                        light_source.m_light_matrix);

      upload_to_uniform("view", light_source.m_light_visualizer_mesh.m_material.m_shader.ID, view_mat);
      upload_to_uniform("viewPosition", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
                        m_cameraPos);
      upload_to_uniform("projection", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
                        projection_mat);
      upload_to_uniform("lightPosition", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
                        light_position);
      upload_to_uniform("viewPos", light_source.m_light_visualizer_mesh.m_material.m_shader.ID, m_cameraPos);

      upload_to_uniform("light_space_matrix", light_source.m_light_visualizer_mesh.m_material.m_shader.ID,
                        light_space_matrix);

      check_gl_error("after setting uniforms");

      // we renderin
      glDrawArrays(GL_TRIANGLES, 0, light_source.m_light_visualizer_mesh.m_vertices_array.size() / 3);

      check_gl_error("after glDrawArrays (lights)");
    }

    // draw to screen
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  log_success("shutdown signal recieved, ended gracefully.");
  glfwTerminate();

  return;
}
