#include "pipeline.hh"
#include "AABB.hh"
#include "constraint.hh"
#include "entity.hh"
#include "logging.hh"
#include "material.hh"
#include "mesh.hh"
#include "overlay_element.hh"
#include "physicsmanager.hh"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <memory>

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
    std::cout << "Invalid Uniform ID! Cant get Uniform: " << uniform_name << std::endl;
    return;
  }
  
  if constexpr (std::is_same<T, glm::mat4>::value) {
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(upload_data));
  } else
    
    if constexpr (std::is_same<T, int>::value) {
      glUniform1i(loc, upload_data);
    } else

      if constexpr (std::is_same<T, bool>::value) {
	glUniform1i(loc, upload_data);
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
	    Logger::log_error("unknown datatype passed to uniform!");
	  }
  
};

/// shadowmapping impl

void Shadow_Map_Pipeline::render_depth_pass() {

  m_active_scene->universal_depth_shader.use();

  // configure spotlight shadow mapping
  glm::mat4 light_look_at =
      m_active_scene->m_loaded_lights[0].get_light_look_at();
    
  float width = m_active_scene->m_loaded_lights[0].light_width;
  glm::mat4 light_projection_mat =
    glm::ortho(-width, width, -width, width, 0.01f, 50.0f);
  
  shared_light_space_matrix = light_projection_mat * light_look_at;

  // set viewport to light settings (not fb output for user)
  glViewport(0, 0, shadow_map_width, shadow_map_height);
  glBindFramebuffer(GL_FRAMEBUFFER, window_depth_map_fbo);
  glClear(GL_DEPTH_BUFFER_BIT);

  check_gl_error("after setting viewport stuff up (depth)");

  /////////////////////////////////
  // render scene from light pov //
  /////////////////////////////////
 
  for (Entity &entity : m_active_scene->m_loaded_entities) {
    for (std::shared_ptr<Mesh> mesh : entity.m_mesh) {

      //prevent rendering hitboxes / other non solid geometry
      if(mesh->m_render_mode == E_WIREFRAME || mesh->m_mesh_type != E_MESH)
	return;

      // dont render transparent meshes TODO make good
      if(mesh->m_material->transparent == true)
	continue;

      // bind meshes vao context
      glBindVertexArray(mesh->m_mesh_vao);
      if (glIsVertexArray(mesh->m_mesh_vao) == GL_FALSE) {
        Logger::log_error("no valid VAO id! cant render mesh.");
      }
      check_gl_error("after settin VAO (depth)");


      //TMP
      if(!entity.is_held_by_localplayer){
	upload_to_uniform( m_active_scene->universal_depth_shader , "model",
			  entity.get_model_matrix() * mesh->get_model_matrix());
      } else {
	glm::mat3 rotation = glm::mat3(m_active_scene->m_local_player->m_player_camera->get_view_matrix());
	glm::mat4 inv_rotation = glm::mat4(glm::transpose(glm::mat3(rotation)));
	inv_rotation = glm::rotate(inv_rotation, glm::radians(180.0f), glm::vec3(0,1,0));
	glm::vec3 pos_offset = m_active_scene->m_local_player->get_position();
        inv_rotation[3] = glm::vec4(pos_offset,1.0f);
       	
	upload_to_uniform(m_active_scene->universal_depth_shader, "model", inv_rotation);       

      }

      //ENDTMP
      upload_to_uniform( m_active_scene->universal_depth_shader , "light_space_matrix",
                        shared_light_space_matrix);
      check_gl_error("after setting uniforms (depth)");

      // render scene from light pov
      glDrawArrays(GL_TRIANGLES, 0, mesh->m_vertices_array.size() / 3);
      check_gl_error("after glDrawArrays (depth)");
    }
  }
};

void Shadow_Map_Pipeline::render_color_point_cloud() {}

void Shadow_Map_Pipeline::render_skybox(std::shared_ptr<Mesh> &mesh) {
  
  Shader* touse = &m_active_scene->universal_flat_shader;
  touse->use();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  
  // bind meshes vao context
  glBindVertexArray(mesh->m_mesh_vao);
  
  // bind texture to sampler slot + set uniform to texture sampler ID
  // uploding texture slot ID to shaders sampler. so that uTexture will sample what we wrote into BL_TEXTURE0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mesh->m_material->m_material_albedo_glid);
  upload_to_uniform(*touse, "skyTexture",0);
  
  check_gl_error("after uploading textures");
  
  upload_to_uniform(*touse, "model",
		    glm::translate(glm::mat4(1.0f),m_active_scene->m_local_player->get_position()) );
  upload_to_uniform(*touse, "view",
		    m_active_scene->m_local_player->m_player_camera->get_view_matrix());
  upload_to_uniform(*touse, "projection",
		    m_active_scene->m_local_player->m_player_camera->get_projection_matrix());

  // we renderin
  glDrawArrays(GL_TRIANGLES, 0, mesh->m_vertices_array.size() / 3);
  check_gl_error("after glDrawArrays");
  
}

void Shadow_Map_Pipeline::render_color_pass() {

  // rebind main "visible" fb
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // clear fb + setup viewport back to the output size
  glViewport(0, 0, m_active_scene->m_scene_framebuffer_width, m_active_scene->m_scene_framebuffer_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  check_gl_error("after clearing frame");

  // adjust camera view / projection matrices
  m_active_scene->m_local_player->m_player_camera->set_view_matrix(
      m_active_scene->m_local_player->get_position(),
      m_active_scene->m_local_player->m_player_camera->m_cameraLookAt +
          m_active_scene->m_local_player->get_position(),
      m_active_scene->m_local_player->m_player_camera->m_cameraUp);

  m_active_scene->m_local_player->m_player_camera->set_projection_matrix(
      glm::radians(m_active_scene->m_local_player->m_player_camera->fov),
      (float)m_active_scene->m_scene_framebuffer_width /
          (float)m_active_scene->m_scene_framebuffer_height,
      0.001f, 1000.0f);

  ///////////////////
  // render meshes //
  ///////////////////
  
  for (Entity &entity : m_active_scene->m_loaded_entities) {
    for (std::shared_ptr<Mesh> &mesh : entity.m_mesh) {

      // dont render transparent meshes TODO make this actual proper impl
      if(mesh->m_material->transparent == true)
	continue;

      // render fullbright skybox
      if( mesh->m_mesh_type == E_SKYBOX ) {
        render_skybox(mesh); continue; }

      // dont render collission boxes either TODO: turn into toggle for col box debug view
      if( mesh->m_mesh_type == E_COL_BOX )
	continue;

      // is mesh culled away?
      if(mesh->m_mesh_culled && m_active_scene->render_properties.cull_scene)
	continue;
      
      // use shader of mesh
      Shader* touse = nullptr;
      if(mesh->m_material->m_material_type == E_PBR)
	touse = &m_active_scene->universal_pbr_shader;
      else
	touse = &m_active_scene->universal_phong_shader;
      touse->use();
      
      check_gl_error("after setting shader active");
      
      // change hitbox or flat style
      if (m_active_scene->render_properties.render_wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

      // bind meshes vao context
      glBindVertexArray(mesh->m_mesh_vao);
      if (glIsVertexArray(mesh->m_mesh_vao) == GL_FALSE) {
        Logger::log_error("no valid VAO id! cant render mesh.");
      }
      check_gl_error("after binding vao");

      if (mesh->m_material->m_material_type == E_PBR) {
	
        // bind texture to sampler slot + set uniform to texture sampler ID
	// uploding texture slot ID to shaders sampler. so that uTexture will sample what we wrote into BL_TEXTURE0
	glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->m_material->m_material_albedo_glid);
	upload_to_uniform(*touse, "uTexture",0);
	
	// bind depth map to sampler slot + set depth map to texture sampler ID
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, window_depth_map);
	upload_to_uniform(*touse, "uDepthMap", 1);

	// do we have normal tex? use it :-)
	if(mesh->m_material->m_material_normal_glid) {
	  glActiveTexture(GL_TEXTURE2);
	  glBindTexture(GL_TEXTURE_2D, mesh->m_material->m_material_normal_glid);
	  upload_to_uniform(*touse, "uNormalMap", 2);	  
	  upload_to_uniform(*touse, "use_normal_map", 1);
	} else {
	  upload_to_uniform(*touse, "use_normal_map", 0);
	}
	
	// or metalrough?? haha omg cool comments
	if(mesh->m_material->m_material_metallic_roughness_glid) {
	  glActiveTexture(GL_TEXTURE3);
	  glBindTexture(GL_TEXTURE_2D, mesh->m_material->m_material_metallic_roughness_glid);
	  upload_to_uniform(*touse, "uMetallicRoughnessMap", 3);
	  upload_to_uniform(*touse, "useMetallicRoughness", 1);
	} else {
	  upload_to_uniform(*touse, "useMetallicRoughness", 0);
	}
        
        check_gl_error("after uploading textures");

	// set rest of uniforms
	upload_to_uniform(*touse, "light_space_matrix",
			  shared_light_space_matrix);
	upload_to_uniform(*touse, "lightPosition",
			  m_active_scene->m_loaded_lights[0].get_light_position());
	upload_to_uniform(*touse, "cameraPosition",
			  m_active_scene->m_local_player->m_player_camera->m_cameraPos);
	upload_to_uniform(*touse, "lightIntensity",
			  m_active_scene->m_loaded_lights[0].m_strength);

        // set debug normal uniform
	if(m_active_scene->render_properties.render_normal_visualizer)
	  upload_to_uniform(*touse, "render_debug_normals", 1);
	else 
	  upload_to_uniform(*touse, "render_debug_normals", 0);

        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(entity.get_model_matrix() * mesh->get_model_matrix())));
	upload_to_uniform(*touse, "normalMatrix", normalMat);
	
      }
      
      
      if(mesh->m_material->m_material_type == E_PHONG) {

	// set rest of uniforms
	upload_to_uniform(*touse, "lightPosition",
			  m_active_scene->m_loaded_lights[0].get_light_position());
	upload_to_uniform(*touse, "viewPos",
			  m_active_scene->m_local_player->m_player_camera->m_cameraPos);
	upload_to_uniform(*touse, "objectColor",
			  glm::vec3(mesh->m_material->m_material_phong_base_color));
      }

      if(!entity.is_held_by_localplayer){
	upload_to_uniform(*touse, "model",
			  entity.get_model_matrix() * mesh->get_model_matrix());
	upload_to_uniform(*touse, "view",
			  m_active_scene->m_local_player->m_player_camera->get_view_matrix());
	upload_to_uniform(*touse, "projection",
			  m_active_scene->m_local_player->m_player_camera->get_projection_matrix());
      } else {
	//make shit held

	glm::mat3 rotation = glm::mat3(m_active_scene->m_local_player->m_player_camera->get_view_matrix());
	glm::mat4 inv_rotation = glm::mat4(glm::transpose(glm::mat3(rotation)));
	inv_rotation = glm::rotate(inv_rotation, glm::radians(180.0f), glm::vec3(0,1,0));
	glm::vec3 pos_offset = m_active_scene->m_local_player->get_position();
        inv_rotation[3] = glm::vec4(pos_offset,1.0f);
       	
	upload_to_uniform(*touse, "model", inv_rotation);       
	upload_to_uniform(*touse, "view", m_active_scene->m_local_player->m_player_camera->get_view_matrix());
	upload_to_uniform(*touse, "projection", glm::translate(m_active_scene->m_local_player->m_player_camera->get_projection_matrix(),glm::vec3(0.25,-0.2,-0.2)));      
      }
      
      check_gl_error("after setting uniforms (shadow map color pass)");

      // we renderin
      glDrawArrays(GL_TRIANGLES, 0, mesh->m_vertices_array.size() / 3);
      check_gl_error("after glDrawArrays");
    }
  }
}

void Shadow_Map_Pipeline::render_overlay_pass() {

  /* TODO rendering:

     - create SDF depth shader for points + springs to shadow map them
     
   */

  /////////////////////////////////
  // rendering light visualizers //
  /////////////////////////////////

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  for (auto &light_source : m_active_scene->m_loaded_lights) {
    /*
      Lights always get Materials of type E_PHONG and shaders phong.vert / phong.frag
    */

    // bind meshes vao context
    glBindVertexArray(light_source.m_light_visualizer_mesh->m_mesh_vao);
    if (glIsVertexArray(light_source.m_light_visualizer_mesh->m_mesh_vao) ==
        GL_FALSE) {
      Logger::log_error("no valid VAO id! cant render mesh.");
    }
    check_gl_error("after binding vao (lights)");
    
    m_active_scene->universal_phong_shader.use();
    check_gl_error("after setting shader active (lights)");

    upload_to_uniform(m_active_scene->universal_phong_shader,
                      "model", glm::translate(glm::mat4(1.0f),light_source.get_light_position()) * light_source.get_light_rotation_matrix());
    upload_to_uniform(m_active_scene->universal_phong_shader,
                      "view", m_active_scene->m_local_player->m_player_camera->get_view_matrix());
    upload_to_uniform(m_active_scene->universal_phong_shader,
                      "projection", m_active_scene->m_local_player->m_player_camera->get_projection_matrix());    
    upload_to_uniform(m_active_scene->universal_phong_shader,
                      "lightPosition", glm::vec3(0.0f));
    upload_to_uniform(m_active_scene->universal_phong_shader,
                      "viewPos", m_active_scene->m_local_player->get_position());
    
    check_gl_error("after setting uniforms (overlay pass light)");

    // we renderin
    glDrawArrays(GL_TRIANGLES, 0,
                 light_source.m_light_visualizer_mesh->m_vertices_array.size() /
                     3);

    check_gl_error("after glDrawArrays (lights)");
  }


  ///////////////////////////////
  // rendering hitbox overlays //
  ///////////////////////////////

  if (m_active_scene->render_properties.render_hitbox) {
    for (Entity& entity : m_active_scene->m_loaded_entities) {
      for (std::shared_ptr<Mesh> &mesh : entity.m_mesh) {

        /*
          Hitboxes always use shader hitbox_vertex.glsl / hitbox_fragment.glsl
          which implement line drawing using signed distance fields. They have
          their own VAO / VBO for AABB coords as a member of the mesh. We upload
          MinBox and MaxBox of the AABB shader does the rest.
        */

        // change to hitbox style (wireframe)
        m_active_scene->universal_hitbox_shader.use();

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_BLEND);

        // bind meshes vao context
        glBindVertexArray(m_active_scene->shared_sdf_vao);
        if (glIsVertexArray(m_active_scene->shared_sdf_vao) == GL_FALSE) {
          Logger::log_error("no valid VAO id! cant render mesh.");
        }

        // set uniforms
        upload_to_uniform(m_active_scene->universal_hitbox_shader,
                          "camera_position",
                          m_active_scene->m_local_player->get_position());
        upload_to_uniform(m_active_scene->universal_hitbox_shader,
                          "box_position_min", mesh->AABB_visualizer->min_corner);
        upload_to_uniform(m_active_scene->universal_hitbox_shader,
                          "box_position_max", mesh->AABB_visualizer->max_corner);
        upload_to_uniform(m_active_scene->universal_hitbox_shader, "radius",
                          1.0f);

        upload_to_uniform(m_active_scene->universal_hitbox_shader, "view",
                          m_active_scene->m_local_player->m_player_camera->get_view_matrix());
        upload_to_uniform(m_active_scene->universal_hitbox_shader, "projection",
                          m_active_scene->m_local_player->m_player_camera->get_projection_matrix());

        upload_to_uniform(m_active_scene->universal_hitbox_shader,
                          "screen_width",
                          (float)m_active_scene->m_scene_framebuffer_width);
        upload_to_uniform(m_active_scene->universal_hitbox_shader,
                          "screen_height",
                          (float)m_active_scene->m_scene_framebuffer_height);

        // can eval here if collision / stationary then change box color or
        // smthn
        upload_to_uniform(m_active_scene->universal_hitbox_shader, "box_color",
                          glm::vec3(0.8, 0.5, 0.2));

        // render call
        glDrawArrays(GL_TRIANGLES, 0, 3);
        check_gl_error("after glDrawArrays (overlay pass hitboxes)");
      }
    }
  }

  //////////////////////////////////////
  // rendering AABB min/max corners   //
  //////////////////////////////////////
  if (m_active_scene->render_properties.render_hitbox) {
    for(Entity e : m_active_scene->m_loaded_entities) {
      for (std::shared_ptr<Mesh> m : e.m_mesh) {

      // back to solid + blending yippie
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_BLEND);

      m_active_scene->universal_point_shader.use();
      check_gl_error("after shader use (point cloud)");

      // bind meshes vao context
      glBindVertexArray(m_active_scene->shared_sdf_vao);
      if (glIsVertexArray(m_active_scene->shared_sdf_vao) == GL_FALSE) {
        Logger::log_error("no valid VAO id! cant render point.");
      }
      check_gl_error("after vao bind (point cloud)");

      upload_to_uniform(m_active_scene->universal_point_shader,
                        "camera_position",
                        m_active_scene->m_local_player->get_position());
      upload_to_uniform(m_active_scene->universal_point_shader,
                        "point_position", m->AABB_visualizer->min_corner);
      upload_to_uniform(m_active_scene->universal_point_shader, "radius",
                        0.2f);

      upload_to_uniform(
          m_active_scene->universal_point_shader, "view",
          m_active_scene->m_local_player->m_player_camera->get_view_matrix());
      upload_to_uniform(m_active_scene->universal_point_shader, "projection",
                        m_active_scene->m_local_player->m_player_camera
                            ->get_projection_matrix());

      // color AABB corner yellow
      glm::vec3 color = {0.9, 0.9, 0.1};
      upload_to_uniform(m_active_scene->universal_point_shader, "point_color",
                        color);

      upload_to_uniform(m_active_scene->universal_point_shader, "screen_width",
                        (float)m_active_scene->m_scene_framebuffer_width);
      upload_to_uniform(m_active_scene->universal_point_shader, "screen_height",
                        (float)m_active_scene->m_scene_framebuffer_height);
      //render first dot
      glDrawArrays(GL_TRIANGLES, 0, 3);

      //render second dot with dif color
      upload_to_uniform(m_active_scene->universal_point_shader,
                        "point_position", m->AABB_visualizer->max_corner);
      color = {0.1, 0.9, 0.9};
      upload_to_uniform(m_active_scene->universal_point_shader, "point_color",
                        color); 
      glDrawArrays(GL_TRIANGLES, 0, 3);

      check_gl_error("after glDrawArrays (point cloud)");
      }
    }
  }

  //////////////////////////////////////
  // rendering point cloud visualizer //
  //////////////////////////////////////

  for (std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {

      // back to solid + blending yippie
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_BLEND);
      
      m_active_scene->universal_point_shader.use();    
      check_gl_error("after shader use (point cloud)");

      // bind meshes vao context
      glBindVertexArray(m_active_scene->shared_sdf_vao);
      if (glIsVertexArray(m_active_scene->shared_sdf_vao) == GL_FALSE) {
        Logger::log_error("no valid VAO id! cant render point.");
      }      
      check_gl_error("after vao bind (point cloud)");


      upload_to_uniform(m_active_scene->universal_point_shader,"camera_position", m_active_scene->m_local_player->get_position());
      upload_to_uniform(m_active_scene->universal_point_shader,"point_position", p->get_position());
      upload_to_uniform(m_active_scene->universal_point_shader,"radius", p->phys_props.radius);

      upload_to_uniform(m_active_scene->universal_point_shader, "view",
                        m_active_scene->m_local_player->m_player_camera->get_view_matrix());
      upload_to_uniform(m_active_scene->universal_point_shader, "projection",
                        m_active_scene->m_local_player->m_player_camera->get_projection_matrix());

      // silly gimmich to show particles experiencing a force atm
      glm::vec3 color = {0.5,0.2,0.9};
      
      if(p->phys_props.force == glm::vec3(0,0,0))
	color = glm::vec3(0.8,0.0,0.4);
      
      if(p->phys_props.fixed)
	color = glm::vec3(0.0,0.0,0.9);

      if(p == m_active_scene->m_selectionstate->selected_point)
	color = glm::vec3(1.0,1.0,1.0);
      
      upload_to_uniform(m_active_scene->universal_point_shader,"point_color", color);

      upload_to_uniform(m_active_scene->universal_point_shader,"screen_width", (float)m_active_scene->m_scene_framebuffer_width);
      upload_to_uniform(m_active_scene->universal_point_shader,"screen_height", (float)m_active_scene->m_scene_framebuffer_height);
      
      glDrawArrays(GL_TRIANGLES, 0,3);
      check_gl_error("after glDrawArrays (point cloud)");

  }

  //////////////////////////////////////
  // rendering Spring visualizers     //
  //////////////////////////////////////
  
  for (std::shared_ptr<Spring>& s : m_active_scene->m_loaded_springs) {
    
    // back to solid + blending yippie
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    
    m_active_scene->universal_line_shader.use();
    
    // bind meshes vao context
    glBindVertexArray(m_active_scene->shared_sdf_vao);
    if (glIsVertexArray(m_active_scene->shared_sdf_vao) == GL_FALSE) {
      Logger::log_error("no valid VAO id! cant render mesh.");
    }
    
    // set uniforms
    upload_to_uniform(m_active_scene->universal_line_shader,"camera_position", m_active_scene->m_local_player->get_position());
    upload_to_uniform(m_active_scene->universal_line_shader,"line_position_min", s->link_A->get_position());
    upload_to_uniform(m_active_scene->universal_line_shader,"line_position_max", s->link_B->get_position());
    upload_to_uniform(m_active_scene->universal_line_shader,"radius", 10.0f);
    
    upload_to_uniform(m_active_scene->universal_line_shader, "view",
		      m_active_scene->m_local_player->m_player_camera->get_view_matrix());
    upload_to_uniform(m_active_scene->universal_line_shader, "projection",
		      m_active_scene->m_local_player->m_player_camera->get_projection_matrix());
    
    upload_to_uniform(m_active_scene->universal_line_shader,"screen_width", (float)m_active_scene->m_scene_framebuffer_width);
    upload_to_uniform(m_active_scene->universal_line_shader,"screen_height", (float)m_active_scene->m_scene_framebuffer_height);
     
    upload_to_uniform(m_active_scene->universal_line_shader,"box_color", glm::vec3(0.2,0.2,0.2));
    
    // render call
    glDrawArrays(GL_TRIANGLES, 0, 3);
    check_gl_error("after glDrawArrays (overlay pass hitboxes)");
    
  }  

  //////////////////////////////////////////////////////
  // rendering fixed ength constraint visualizers     //
  //////////////////////////////////////////////////////
  
  for (std::shared_ptr<Constraint>& c : m_active_scene->m_loaded_constraints) {

    //check wether constraint is a length one
    if (const std::shared_ptr<Fix_length_constraint> &lc =
	std::dynamic_pointer_cast<Fix_length_constraint>(c)) {

      m_active_scene->universal_dotted_line_shader.use();
      
      // bind meshes vao context
      glBindVertexArray(m_active_scene->shared_sdf_vao);
      if (glIsVertexArray(m_active_scene->shared_sdf_vao) == GL_FALSE) {
	Logger::log_error("no valid VAO id! cant render mesh.");
    }
      
      // set uniforms
      upload_to_uniform(m_active_scene->universal_dotted_line_shader,"camera_position", m_active_scene->m_local_player->get_position());
      upload_to_uniform(m_active_scene->universal_dotted_line_shader,"line_position_min", lc->point_a->get_position());
      upload_to_uniform(m_active_scene->universal_dotted_line_shader,"line_position_max", lc->point_b->get_position());
      upload_to_uniform(m_active_scene->universal_dotted_line_shader,"radius", 10.0f);
      
      upload_to_uniform(m_active_scene->universal_dotted_line_shader, "view",
			m_active_scene->m_local_player->m_player_camera->get_view_matrix());
      upload_to_uniform(m_active_scene->universal_dotted_line_shader, "projection",
			m_active_scene->m_local_player->m_player_camera->get_projection_matrix());
      
      upload_to_uniform(m_active_scene->universal_dotted_line_shader,"screen_width", (float)m_active_scene->m_scene_framebuffer_width);
      upload_to_uniform(m_active_scene->universal_dotted_line_shader,"screen_height", (float)m_active_scene->m_scene_framebuffer_height);
      
      upload_to_uniform(m_active_scene->universal_dotted_line_shader,"line_color", glm::vec3(0.6,0.3,0.3));

      upload_to_uniform(m_active_scene->universal_dotted_line_shader,"dot_spacing_px", 20.0f);
      upload_to_uniform(m_active_scene->universal_dotted_line_shader,"dot_radius_px", 20.0f);

      
      // render call
      glDrawArrays(GL_TRIANGLES, 0, 3);
      check_gl_error("after glDrawArrays (overlay pass hitboxes)");
      
    }
  }  
  
  
  ////////////////////
  // render text overlay
  ////////////////////

  for(std::shared_ptr<Overlay_Element> oe : m_active_scene->m_loaded_overlay_elements) {

    // back to solid + blending yippie
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND);
    
    m_active_scene->universal_text_shader.use();
   
    // bind meshes vao context
    glBindVertexArray(oe->text_vao);
    if (glIsVertexArray(oe->text_vao) == GL_FALSE) {
      Logger::log_error("no valid VAO id! cant render mesh.");
    }
    check_gl_error("after glDrawArrays (overlay pass text b4 everything)");
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, oe->texture_glid);
    upload_to_uniform(m_active_scene->universal_text_shader, "fontAtlas", 0);
    upload_to_uniform(m_active_scene->universal_text_shader, "text_color" , oe->element_color);
    check_gl_error("after glDrawArrays (uniforms overlay pass text)");
    
    glDrawArrays(GL_TRIANGLES, 0, oe->text_vert_coords_screen_space.size()/2);
    check_gl_error("after glDrawArrays (overlay pass text)");
    
  }
  
}

void Shadow_Map_Pipeline::render_frame() {
  
  if (m_active_scene == nullptr)
    return;
  
  if(pipeline_is_setup) {
    render_depth_pass();
    render_color_pass();
    render_overlay_pass();
    
    //TODO move this from here to input need this here atm for updated cam view mat n scene pointer access
    if(m_active_scene->m_selectionstate->launch_picker)
      handle_pick();
    
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
  
  depth_shader = std::make_shared<Shader>("src/shaders/shader_src/depth.vert",
                            "src/shaders/shader_src/depth.frag");
}

void Shadow_Map_Pipeline::init_color_pass() {

}

void Shadow_Map_Pipeline::handle_pick() {

  float mouse_ndc_x = (2.0f * m_active_scene->m_selectionstate->mouse_pos_x) / m_active_scene->m_scene_framebuffer_width - 1.0f;
  float mouse_ndc_y = 1.0f - (2.0f * m_active_scene->m_selectionstate->mouse_pos_y) / m_active_scene->m_scene_framebuffer_height;

  float aspect_ratio = (float)m_active_scene->m_scene_framebuffer_width / (float)m_active_scene->m_scene_framebuffer_height;
  float fov_rads = glm::radians(m_active_scene->m_local_player->m_player_camera->fov);
  float half_fov = tan(fov_rads / 2.0f);

  //raycast
  glm::vec3 ray_camera_space(
		      mouse_ndc_x * half_fov * aspect_ratio,
		      mouse_ndc_y * half_fov,
		      -1.0f
		      );
  glm::mat4 inv_view_mat = glm::inverse(m_active_scene->m_local_player->m_player_camera->get_view_matrix());
  glm::vec4 dir4 = inv_view_mat * glm::vec4(ray_camera_space, 0.0f);
  glm::vec3 ray_world_space = glm::normalize(glm::vec3(dir4));
  
  glm::vec3 current_pos_b = m_active_scene->m_local_player->get_position();
  glm::vec3 look_at_c = ray_world_space + current_pos_b;

  //carry floats
  float lowest_distance = 100000.0f;
  float max_radius = 2.0f;
  bool new_selection_found = false;
  
  for(std::shared_ptr<Point> p_a : m_active_scene->m_loaded_points) {

    glm::vec3 d = (look_at_c - current_pos_b) / glm::distance(look_at_c, current_pos_b);
    glm::vec3 v = p_a->get_position() - current_pos_b;
    float t = glm::dot(v,d);
    glm::vec3 P = current_pos_b + t * d;
    
    float distance = glm::distance(P,p_a->get_position());
    float scaled_radius = max_radius / ((p_a->get_position() - current_pos_b).length());
    
    if( distance < lowest_distance && distance < scaled_radius) {
      m_active_scene->m_selectionstate->selected_point = p_a;
      lowest_distance = distance;
      new_selection_found = true;
    }
  }
  
  if(!new_selection_found)
    m_active_scene->m_selectionstate->selected_point = nullptr;

  m_active_scene->m_selectionstate->launch_picker = false;
}

void Shadow_Map_Pipeline::update_scene(std::shared_ptr<Scene> new_scene) { m_active_scene = new_scene; }
void Shadow_Map_Pipeline::update_time(float new_time) { }

/// Raytrace impl coming soon xd

void Ray_Traced_Pipeline::render_frame() {}
void Ray_Traced_Pipeline::update_scene() {}
void Ray_Traced_Pipeline::update_time() {}

/// Pipeline boilerplate empty impl

void Pipeline::render_frame() {}
void Pipeline::init_pipeline() {}

