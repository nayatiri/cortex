#include "pipeline.hh"
#include "mesh.hh"

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

/// Pipeline boilerplate empty impl

void Pipeline::render_frame() {}
void Pipeline::update_scene() {}
void Pipeline::update_time() {}
void Pipeline::init_pipeline() {}

/// Pipeline util functions

void Pipeline::check_gl_error(const char *context = "") {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    const char *errorStr = "Unknown error";
    switch (err) {
    case GL_INVALID_ENUM:
      errorStr = "GL_INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      errorStr = "GL_INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      errorStr = "GL_INVALID_OPERATION";
      break;
    case GL_OUT_OF_MEMORY:
      errorStr = "GL_OUT_OF_MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
      break;
    }

    if (context && *context)
      printf("OpenGL Error [%s]: %s (0x%X)\n", context, errorStr, err);
    else
      printf("OpenGL Error: %s (0x%X)\n", errorStr, err);
  }
}

template <typename T>
void Pipeline::upload_to_uniform(Shader bound_shader, std::string uniform_name,
                                 T upload_data) {

  GLuint loc = bound_shader.get_cached_uniform_id(uniform_name);

  if constexpr (std::is_same<T, glm::mat4>::value) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(upload_data));
  } else

      if constexpr (std::is_same<T, glm::vec3>::value) {
    glUniform3fv(loc, 1, glm::value_ptr(upload_data));
  } else

      if constexpr (std::is_same<T, glm::mat3>::value) {
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(upload_data));
  } else {
    log_error("unknown datatype passed to uniform!");
  }
};

/// shadowmapping impl

void Shadow_Map_Pipeline::render_depth_pass() {

  depth_shader->use();

  // configure spotlight shadow mapping
  glm::vec3 light_pos_new =
      m_active_scene->m_loaded_lights[0].get_light_position();
  //  glm::mat3 light_rotation =
  //  m_active_scene->m_loaded_lights[0].get_light_rotation_matrix();

  glm::mat4 light_look_at = glm::lookAt(
      light_pos_new,
      //light_pos_new + glm::normalize(light_rotation * glm::vec3(0, 0, -1)),
      glm::vec3(0.0f,0.0f,0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f));

  // use for sanity
  float width = m_active_scene->m_loaded_lights[0].light_width;
  glm::mat4 light_projection_mat =
      glm::ortho(-width, width, -width, width, 0.01f, 20.0f);

  shared_light_space_matrix = light_projection_mat * light_look_at;

  depth_shader->use();

  glViewport(0, 0, shadow_map_width, shadow_map_height);
  glBindFramebuffer(GL_FRAMEBUFFER, window_depth_map_fbo);
  glClear(GL_DEPTH_BUFFER_BIT);

  check_gl_error("after setting viewport stuff up (depth)");

  // render scene from light pov
  for (auto &entity : m_active_scene->m_loaded_entities) {
    for (auto &mesh : entity.m_mesh) {

      if(mesh.m_render_mode == E_WIREFRAME)
	return;

      // bind meshes vao context
      glBindVertexArray(mesh.m_mesh_vao);
      if (glIsVertexArray(mesh.m_mesh_vao) == GL_FALSE) {
        log_error("no valid VAO id! cant render mesh.");
      }

      check_gl_error("before setting uniforms (depth)");

      upload_to_uniform(*depth_shader, "model",
                        entity.get_model_matrix() * mesh.get_model_matrix());
      upload_to_uniform(*depth_shader, "light_space_matrix",
                        shared_light_space_matrix);

      check_gl_error("after setting uniforms (depth)");

      // we renderin
      glDrawArrays(GL_TRIANGLES, 0, mesh.m_vertices_array.size() / 3);

      check_gl_error("after glDrawArrays (depth)");
    }
  }
};

void Shadow_Map_Pipeline::render_color_point_cloud() {

  
  
}

void Shadow_Map_Pipeline::render_color_pass() {

  // rebind old fb
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // render scene with old settings
  glViewport(0, 0, m_viewport_width, m_viewport_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  check_gl_error("after clearing frame");

  shared_camera_view_matrix =
      glm::lookAt(m_active_scene->m_camera->m_cameraPos,
                  m_active_scene->m_camera->m_cameraLookAt +
                      m_active_scene->m_camera->m_cameraPos,
                  m_active_scene->m_camera->m_cameraUp);

  // projection matrix
  shared_camera_projection_matrix = glm::perspective(
      glm::radians(90.0f), (float)m_viewport_width / (float)m_viewport_height,
      0.001f, 1000.0f);

  // render meshes
  for (auto &entity : m_active_scene->m_loaded_entities) {
    for (auto &mesh : entity.m_mesh) {

      // change hitbox or flat style
      if (mesh.m_render_mode == E_WIREFRAME)
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
            mesh.m_material.m_shader.get_cached_uniform_id("uTexture");

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

      upload_to_uniform(mesh.m_material.m_shader, "objectColor",
                        glm::vec3(0.5, 0.8, 0.2));
      upload_to_uniform(mesh.m_material.m_shader, "lightColor",
                        glm::vec3(0.8, 0.8, 0.8));

      upload_to_uniform(mesh.m_material.m_shader, "model",
                        entity.get_model_matrix() * mesh.get_model_matrix());

      upload_to_uniform(mesh.m_material.m_shader, "view",
                        shared_camera_view_matrix);
      upload_to_uniform(mesh.m_material.m_shader, "viewPosition",
                        m_active_scene->m_camera->m_cameraPos);
      upload_to_uniform(mesh.m_material.m_shader, "projection",
                        shared_camera_projection_matrix);
      upload_to_uniform(mesh.m_material.m_shader, "lightPosition",
                        light_position);
      upload_to_uniform(mesh.m_material.m_shader, "viewPos",
                        m_active_scene->m_camera->m_cameraPos);

      upload_to_uniform(mesh.m_material.m_shader, "light_space_matrix",
                        shared_light_space_matrix);

      check_gl_error("after setting uniforms (shadow map color pass)");

      // we renderin
      glDrawArrays(GL_TRIANGLES, 0, mesh.m_vertices_array.size() / 3);

      check_gl_error("after glDrawArrays");
    }
  }
}

void Shadow_Map_Pipeline::render_overlay_pass() {

  /////////////////////////////////
  // rendering light visualizers //
  /////////////////////////////////

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
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

      check_gl_error("after uploading textures (lights)");
    }

    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "objectColor", glm::vec3(0.5, 0.8, 0.2));
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "lightColor", glm::vec3(0.8, 0.8, 0.8));
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "model", glm::translate(glm::mat4(1.0f),light_source.get_light_position()) * light_source.get_light_rotation_matrix());

    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "view", shared_camera_view_matrix);
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "viewPosition", m_active_scene->m_camera->m_cameraPos);
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "projection", shared_camera_projection_matrix);
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "lightPosition", glm::vec3(0.0f));
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "viewPos", m_active_scene->m_camera->m_cameraPos);
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "light_space_matrix", shared_light_space_matrix);

    check_gl_error("after setting uniforms (overlay pass light)");

    // we renderin
    glDrawArrays(GL_TRIANGLES, 0,
                 light_source.m_light_visualizer_mesh.m_vertices_array.size() /
                     3);

    check_gl_error("after glDrawArrays (lights)");
  }


  ///////////////////////////////
  // rendering hitbox overlays //
  ///////////////////////////////

  // render hitbox meshes
  for (auto &entity : m_active_scene->m_loaded_entities) {
    for (auto &mesh : entity.m_mesh) {
      
      // change to hitbox style
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

      // bind meshes vao context
      glBindVertexArray(mesh.AABB_visualizer->m_mesh_vao);
      if (glIsVertexArray(mesh.AABB_visualizer->m_mesh_vao) == GL_FALSE) {
        log_error("no valid VAO id! cant render mesh.");
      }

      check_gl_error("after binding vao (overlay pass hitboxes)");

      mesh.AABB_visualizer->m_material.m_shader.use();

      check_gl_error("after setting shader active (overlay pass hitboxes)");

      upload_to_uniform(mesh.m_material.m_shader, "model",
                        entity.get_model_matrix() * mesh.get_model_matrix());
      upload_to_uniform(mesh.m_material.m_shader, "view",
                        shared_camera_view_matrix);
      upload_to_uniform(mesh.m_material.m_shader, "projection",
                        shared_camera_projection_matrix);

      check_gl_error("after setting uniforms (overlay pass hitboxes)");

      // we renderin
      glDrawArrays(GL_TRIANGLES, 0, mesh.AABB_visualizer->m_vertices_array.size() / 3);

      check_gl_error("after glDrawArrays (overlay pass hitboxes)");
    }
  }
}

void Shadow_Map_Pipeline::render_frame() {
  
  if (m_active_scene == nullptr)
    return;
  
  if(pipeline_is_setup) {
    render_depth_pass();
    render_color_pass();
    render_overlay_pass();
  } else {
    init_pipeline();
  }
}

void Shadow_Map_Pipeline::init_pipeline() {
  
  if (m_active_scene == nullptr)
    return;
  
  init_depth_pass();
  init_color_pass();
  
  pipeline_is_setup = true;
  
}

void Shadow_Map_Pipeline::init_depth_pass() {
    
  printf("setup vbos called from depth!\n");
  
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
}

void Shadow_Map_Pipeline::init_color_pass() {


  
}

void Shadow_Map_Pipeline::update_scene() {}

void Shadow_Map_Pipeline::update_time() {}

/// Raytrace impl

void Ray_Traced_Pipeline::render_frame() {}
void Ray_Traced_Pipeline::update_scene() {}
void Ray_Traced_Pipeline::update_time() {}
