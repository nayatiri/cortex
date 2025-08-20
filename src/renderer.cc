#include "renderer.hh"

// components
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
#include <vector>

// components (custom)
#include "components/animation.hh"
#include "components/entity.hh"
#include "components/input.hh"
#include "components/light.hh"
#include "components/logging.hh"
#include "components/mesh.hh"
#include "components/scene.hh"
#include "components/utility.hh"
#include "components/animationmanager.hh"
#include "shaders/shaderclass.hh"


#define DEF_NEAR_CLIP_PLANE 0.01f
#define DEF_FAR_CLIP_PLANE 10000.0f

void Renderer::setup_render_properties() {

  // render mode
  if (m_render_mode_wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  return;
}

void Renderer::framebuffer_size_callback(GLFWwindow *window, int width,
                                         int height) {
  log_success("framebuffer resized.");
  glViewport(0, 0, width, height);
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

  // bungie employees hate this simple trick
  float currentFrame = glfwGetTime();
  m_deltaTime = currentFrame - m_application_current_time;
  m_application_current_time = currentFrame;

  // handle all abstracted stuff that changes the scene somehow
  setup_render_properties();
  m_animation_manager->handle_scene_animations(m_application_current_time);
  m_physics_manager->handle_scene_physics();
  m_input_manager->process_input(associated_window, m_application_current_time, m_deltaTime);

  // make sure data changes get reflected in VRAM
  if(m_active_scene->m_scene_vbos_need_refresh)
    init_scene_vbos();
  
  // setup constants for render pass
  glfwGetWindowSize(associated_window, &m_viewport_width, &m_viewport_height);

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

  m_active_scene = std::make_shared<Scene>();
  m_input_manager->m_active_scene = m_active_scene;
  m_animation_manager->m_active_scene = m_active_scene;
  m_physics_manager->m_active_scene = m_active_scene;
  log_success("set Scene ptr to IM, AM and PM");
  
  m_active_scene->m_camera = std::make_unique<Camera>();

  Light main_light(std::move(load_all_meshes_from_gltf(
      "models/light/scene.gltf", num_loaded_textures, m_texture_map))[0]);
  main_light.m_light_type = E_POINT_LIGHT;
  main_light.m_color = 0xFFFFFF;
  main_light.m_strength = 10;

  main_light.m_light_matrix =
      glm::translate(main_light.m_light_matrix, glm::vec3(10.0f, 2.0f, 1.0f));

  m_active_scene->add_entity_to_scene(load_entity);
  m_active_scene->add_light_to_scene(main_light);

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

void Renderer::cleanup_mesh_vbos(Mesh& mesh) {
  if (mesh.m_mesh_vao != 0) {
    glDeleteVertexArrays(1, &mesh.m_mesh_vao);
    mesh.m_mesh_vao = 0;
  }

  auto delete_buffer = [](GLuint& buffer_id) {
    if (buffer_id != 0) {
      glDeleteBuffers(1, &buffer_id);
      buffer_id = 0;
    }
  };

  delete_buffer(mesh.m_vertices_glid);
  delete_buffer(mesh.m_tex_coords_glid);
  delete_buffer(mesh.m_normals_glid);
  delete_buffer(mesh.m_tangents_glid);
  delete_buffer(mesh.m_binormals_glid);
}

void Renderer::init_scene_vbos() {
  if (m_active_scene->m_loaded_entities.empty() ||
      m_active_scene->m_loaded_lights.empty()) {
    log_error("Scene doesn't contain at least one light + entity, not initializing VBOs");
    return;
  }

  log_debug("Initializing/Updating VBOs for scene...");

  ////////////////////////////////////
  // Update Light VBOs (for visualizers)
  ////////////////////////////////////
  for (auto &light : m_active_scene->m_loaded_lights) {
    if (!light.m_light_visualizer_mesh.m_mesh_vbo_needs_refresh)
      continue;

    auto &mesh = light.m_light_visualizer_mesh;

    // Clean up existing GL resources if they exist
    cleanup_mesh_vbos(mesh);

    // Generate new VAO + VBOs
    glGenVertexArrays(1, &mesh.m_mesh_vao);
    glBindVertexArray(mesh.m_mesh_vao);

    // Vertices
    if (!mesh.m_vertices_array.empty()) {
      glGenBuffers(1, &mesh.m_vertices_glid);
      glBindBuffer(GL_ARRAY_BUFFER, mesh.m_vertices_glid);
      glBufferData(GL_ARRAY_BUFFER,
                   mesh.m_vertices_array.size() * sizeof(float),
                   mesh.m_vertices_array.data(),
                   GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);
    }

    // TexCoords (optional)
    if (!mesh.m_tex_coords_array.empty()) {
      glGenBuffers(1, &mesh.m_tex_coords_glid);
      glBindBuffer(GL_ARRAY_BUFFER, mesh.m_tex_coords_glid);
      glBufferData(GL_ARRAY_BUFFER,
                   mesh.m_tex_coords_array.size() * sizeof(float),
                   mesh.m_tex_coords_array.data(),
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(1);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh.m_mesh_vbo_needs_refresh = false;
    log_debug_sub("Updated VBOs for light visualizer");
  }

  ////////////////////////////////////
  // Update Entity Mesh VBOs
  ////////////////////////////////////
  for (auto &entity : m_active_scene->m_loaded_entities) {
    for (auto &mesh : entity.m_mesh) {
      if (!mesh.m_mesh_vbo_needs_refresh)
        continue;  

      log_debug_sub("Reinitializing VBOs for mesh (needs refresh)");

      // Clean up old buffers to prevent leaks
      cleanup_mesh_vbos(mesh);

      // Recalculate normals if missing
      if (mesh.m_normals_array.empty()) {
        log_debug("Mesh missing normals, recalculating...");
        mesh.m_normals_array = calculate_vert_normals(mesh.m_vertices_array);
      }

      // Recalculate tangents/binormals if needed and texcoords exist
      if (!mesh.m_tex_coords_array.empty()) {
        if (mesh.m_tangents_array.empty() || mesh.m_binormals_array.empty()) {
          log_debug("Missing tangents/binormals, calculating...");
          tan_bin_glob tb = calculate_vert_tan_bin(
              mesh.m_vertices_array, mesh.m_normals_array, mesh.m_tex_coords_array);
          mesh.m_tangents_array = tb.vert_tangents;
          mesh.m_binormals_array = tb.vert_binormals;
        }
      } else {
        log_error("Mesh has no UVs; using zeroed tangents/binormals");
        mesh.m_tangents_array.resize(mesh.m_vertices_array.size(), 0.0f);
        mesh.m_binormals_array.resize(mesh.m_vertices_array.size(), 0.0f);
      }

      // Create VAO
      glGenVertexArrays(1, &mesh.m_mesh_vao);
      glBindVertexArray(mesh.m_mesh_vao);

      // verts
      if (!mesh.m_vertices_array.empty()) {
        glGenBuffers(1, &mesh.m_vertices_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.m_vertices_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh.m_vertices_array.size() * sizeof(float),
                     mesh.m_vertices_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
      }

      // tex coords
      if (!mesh.m_tex_coords_array.empty()) {
        glGenBuffers(1, &mesh.m_tex_coords_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.m_tex_coords_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh.m_tex_coords_array.size() * sizeof(float),
                     mesh.m_tex_coords_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
      }

      // normals
      if (!mesh.m_normals_array.empty()) {
        glGenBuffers(1, &mesh.m_normals_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.m_normals_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh.m_normals_array.size() * sizeof(float),
                     mesh.m_normals_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
      }

      // tangents
      if (!mesh.m_tangents_array.empty()) {
        glGenBuffers(1, &mesh.m_tangents_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.m_tangents_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh.m_tangents_array.size() * sizeof(float),
                     mesh.m_tangents_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(3);
      }

      // binormals
      if (!mesh.m_binormals_array.empty()) {
        glGenBuffers(1, &mesh.m_binormals_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.m_binormals_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh.m_binormals_array.size() * sizeof(float),
                     mesh.m_binormals_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(4);
      }

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      mesh.m_mesh_vbo_needs_refresh = false;
      log_debug_sub("Successfully updated VBOs for mesh");
    }
  }

  m_active_scene->m_scene_vbos_need_refresh = false;
  log_success("Successfully initialized/updated VBOs for all dirty meshes!");
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

  // create input manager
  log_error("init IM with broken pointer");
  std::cout << "w" << m_active_scene << std::endl;
  m_input_manager = std::move(std::make_unique<Input_Manager>(nullptr));
  m_animation_manager = std::move(std::make_unique<Animation_Manager>(nullptr));
  m_physics_manager = std::move(std::make_unique<Physics_Manager>(nullptr));
  
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
  glfwSetWindowUserPointer(associated_window, m_input_manager.get());

  // set callbacks using lambda functions
  glfwSetScrollCallback(associated_window, [](GLFWwindow *w, double xoffset,
                                              double yoffset) {
    Input_Manager *imanager = static_cast<Input_Manager *>(glfwGetWindowUserPointer(w));
    
    if (imanager) {
      imanager->scroll_callback(w, xoffset, yoffset);
    }
  });
  
  glfwSetCursorPosCallback(associated_window, [](GLFWwindow *w, double xpos,
                                                 double ypos) {
    Input_Manager *imanager = static_cast<Input_Manager *>(glfwGetWindowUserPointer(w));
    if (imanager) {
      imanager->mouse_callback(w, xpos, ypos);
    }
  });

  glfwSetInputMode(associated_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  return;
  
}
