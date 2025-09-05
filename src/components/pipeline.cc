#include "pipeline.hh"
#include "entity.hh"
#include "logging.hh"
#include "material.hh"
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
  
  int loc = bound_shader.get_cached_uniform_id(uniform_name);
  if(loc < 0){
    log_error("invalid uniform ID returned. Cant upload data!");
    return;
  }
  
  if constexpr (std::is_same<T, glm::mat4>::value) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(upload_data));
  } else
    
    if constexpr (std::is_same<T, glm::vec3>::value) {
      glUniform3fv(loc, 1, glm::value_ptr(upload_data));
    } else
      
      if constexpr (std::is_same<T, float>::value) {
	glUniform1f(loc, upload_data);
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

  glm::mat4 light_look_at = glm::lookAt(
					light_pos_new, // location
					glm::vec3(0.0f,0.0f,0.0f), // look at
					glm::vec3(0.0f, 1.0f, 0.0f)); // up vector 

  float width = m_active_scene->m_loaded_lights[0].light_width;
  glm::mat4 light_projection_mat =
    glm::ortho(-width, width, -width, width, 0.01f, 20.0f); // TODO make clip planes work properly

  shared_light_space_matrix = light_projection_mat * light_look_at;

  depth_shader->use();

  // set viewport to light settings (not fb output for user)
  glViewport(0, 0, shadow_map_width, shadow_map_height);
  glBindFramebuffer(GL_FRAMEBUFFER, window_depth_map_fbo);
  glClear(GL_DEPTH_BUFFER_BIT);

  check_gl_error("after setting viewport stuff up (depth)");

  // render scene from light pov
  for (auto &entity : m_active_scene->m_loaded_entities) {
    for (auto &mesh : entity.m_mesh) {

      //prevent rendering hitboxes / other non solid geometry
      if(mesh.m_render_mode == E_WIREFRAME)
	return;

      // bind meshes vao context
      glBindVertexArray(mesh.m_mesh_vao);
      if (glIsVertexArray(mesh.m_mesh_vao) == GL_FALSE) {
        log_error("no valid VAO id! cant render mesh.");
      }
      check_gl_error("after settin VAO (depth)");

      upload_to_uniform(*depth_shader, "model",
                        entity.get_model_matrix() * mesh.get_model_matrix());
      upload_to_uniform(*depth_shader, "light_space_matrix",
                        shared_light_space_matrix);
      check_gl_error("after setting uniforms (depth)");

      // render scene from light pov
      glDrawArrays(GL_TRIANGLES, 0, mesh.m_vertices_array.size() / 3);
      check_gl_error("after glDrawArrays (depth)");
    }
  }
};

void Shadow_Map_Pipeline::render_color_point_cloud() {}

void Shadow_Map_Pipeline::render_color_pass() {

  // rebind main "visible" fb
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // clear fb + setup viewport back to the output size
  glViewport(0, 0, m_viewport_width, m_viewport_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  check_gl_error("after clearing frame");

  // adjust camera view / projection matrices
  shared_camera_view_matrix =
      glm::lookAt(m_active_scene->m_camera->m_cameraPos,
                  m_active_scene->m_camera->m_cameraLookAt +
                      m_active_scene->m_camera->m_cameraPos,
                  m_active_scene->m_camera->m_cameraUp);

  shared_camera_projection_matrix = glm::perspective(
      glm::radians(90.0f), (float)m_viewport_width / (float)m_viewport_height,
      0.001f, 1000.0f);

  // render meshes
  for (auto &entity : m_active_scene->m_loaded_entities) {
    if(entity.entity_type == Entity_Point)
      continue;
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

      // use shader of mesh
      mesh.m_material.m_shader.use();
      check_gl_error("after setting shader active");

      if (mesh.m_material.m_material_type == E_PBR_TEX) {
	
        // bind texture to sampler slot + set uniform to texture sampler ID
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.m_material.bound_texture_id);

        GLint loc_tex =
            mesh.m_material.m_shader.get_cached_uniform_id("uTexture");

        glUniform1i(loc_tex, 0);
	
	// bind depth map to sampler slot + set depth map to texture sampler ID
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, window_depth_map);
        GLint loc_depth =
            glGetUniformLocation(mesh.m_material.m_shader.ID, "uDepthMap");
        glUniform1i(loc_depth, 1);

        check_gl_error("after uploading textures");

	// set rest of uniforms
	upload_to_uniform(mesh.m_material.m_shader, "light_space_matrix",
			  shared_light_space_matrix);	
      }
      
      if(mesh.m_material.m_material_type == E_PHONG) {

	// set rest of uniforms
	upload_to_uniform(mesh.m_material.m_shader, "lightPosition",
			  m_active_scene->m_loaded_lights[0].get_light_position());
	upload_to_uniform(mesh.m_material.m_shader, "viewPos",
			  m_active_scene->m_camera->m_cameraPos);	
      }

      upload_to_uniform(mesh.m_material.m_shader, "model",//
                        entity.get_model_matrix() * mesh.get_model_matrix());
      upload_to_uniform(mesh.m_material.m_shader, "view",//
                        shared_camera_view_matrix);
      upload_to_uniform(mesh.m_material.m_shader, "projection",//
                        shared_camera_projection_matrix);

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
    /*
      Lights always get Materials of type E_PHONG and shaders phong.vert / phong.frag
    */

    // bind meshes vao context
    glBindVertexArray(light_source.m_light_visualizer_mesh.m_mesh_vao);
    if (glIsVertexArray(light_source.m_light_visualizer_mesh.m_mesh_vao) ==
        GL_FALSE) {
      log_error("no valid VAO id! cant render mesh.");
    }
    check_gl_error("after binding vao (lights)");
    
    light_source.m_light_visualizer_mesh.m_material.m_shader.use();
    check_gl_error("after setting shader active (lights)");

    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "model", glm::translate(glm::mat4(1.0f),light_source.get_light_position()) * light_source.get_light_rotation_matrix());
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "view", shared_camera_view_matrix);
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "projection", shared_camera_projection_matrix);    
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "lightPosition", glm::vec3(0.0f));
    upload_to_uniform(light_source.m_light_visualizer_mesh.m_material.m_shader,
                      "viewPos", m_active_scene->m_camera->m_cameraPos);


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

  //TODO: - make hitbox shader a dedicated line shader
  //      - on the same note make AABB only the corners instead of lots of tris
  
  // render hitbox meshes
  for (auto &entity : m_active_scene->m_loaded_entities) {
    if(entity.entity_type == Entity_Point)
      continue;
    for (auto &mesh : entity.m_mesh) {
      
      /*
	Wireframe hitboxes always use shader wireframe.vert / wireframe.frag
	They have their own VAO / VBO for AABB coords as a member of the mesh.
	Yes this can result in every mesh having infinite mesh members, since
	every hitbox can have a hitbox too. but since we calculate AABB this
	shouldnt be an issue.
       */
      
      // change to hitbox style (wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      
      // bind meshes vao context
      glBindVertexArray(mesh.AABB_visualizer->m_mesh_vao);
      if (glIsVertexArray(mesh.AABB_visualizer->m_mesh_vao) == GL_FALSE) {
        log_error("no valid VAO id! cant render mesh.");
      }
      check_gl_error("after binding vao (overlay pass hitboxes)");

      mesh.AABB_visualizer->m_material.m_shader.use();
      check_gl_error("after setting shader active (overlay pass hitboxes)");

      // set uniforms
      upload_to_uniform(mesh.m_material.m_shader, "model",
                        entity.get_model_matrix() * mesh.get_model_matrix());
      upload_to_uniform(mesh.m_material.m_shader, "view",
                        shared_camera_view_matrix);
      upload_to_uniform(mesh.m_material.m_shader, "projection",
                        shared_camera_projection_matrix);
      check_gl_error("after setting uniforms (overlay pass hitboxes)");

      // render call
      glDrawArrays(GL_TRIANGLES, 0, mesh.AABB_visualizer->m_vertices_array.size() / 3);
      check_gl_error("after glDrawArrays (overlay pass hitboxes)");
    }
  }

  //////////////////////////////////////
  // rendering point cloud visualizer //
  //////////////////////////////////////
  
  for (auto &entity : m_active_scene->m_loaded_entities) {

    if(entity.entity_type==Entity_Point) {

      // back to solid + blending yippie
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_BLEND);
      
      entity.m_mesh[0].m_material.m_shader.use();    
      check_gl_error("after shader use (point cloud)");
      
      // bind meshes vao context
      glBindVertexArray(entity.m_mesh[0].m_mesh_vao);
      if (glIsVertexArray(entity.m_mesh[0].m_mesh_vao) == GL_FALSE) {
        log_error("no valid VAO id! cant render mesh.");
      }      
      check_gl_error("after vao bind (point cloud)");


      upload_to_uniform(entity.m_mesh[0].m_material.m_shader,"camera_position", m_active_scene->m_camera->m_cameraPos);
      upload_to_uniform(entity.m_mesh[0].m_material.m_shader,"point_position", entity.get_position());
      upload_to_uniform(entity.m_mesh[0].m_material.m_shader,"radius", 0.3f);
      
      upload_to_uniform(entity.m_mesh[0].m_material.m_shader, "model",
                        entity.get_model_matrix() * entity.m_mesh[0].get_model_matrix());
      upload_to_uniform(entity.m_mesh[0].m_material.m_shader, "view",
                        shared_camera_view_matrix);
      upload_to_uniform(entity.m_mesh[0].m_material.m_shader, "projection",
                        shared_camera_projection_matrix);

      upload_to_uniform(entity.m_mesh[0].m_material.m_shader,"screen_width", (float)m_active_scene->m_scene_framebuffer_width);
      upload_to_uniform(entity.m_mesh[0].m_material.m_shader,"screen_height", (float)m_active_scene->m_scene_framebuffer_height);
      
      
      glDrawArrays(GL_TRIANGLES, 0,3);
      check_gl_error("after glDrawArrays (point cloud)");

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
