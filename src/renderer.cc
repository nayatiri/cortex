#include "renderer.hh"

// third party libraries
#include "./glad/glad.h"
#include "./libs/tiny_gltf.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// stdlib
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

// components
#include "components/AABB.hh"
#include "components/constraint.hh"
#include "components/entity.hh"
#include "components/force_generator.hh"
#include "components/input.hh"
#include "components/light.hh"
#include "components/logging.hh"
#include "components/overlay_element.hh"
#include "components/point.hh"
#include "components/mesh.hh"
#include "components/scene.hh"
#include "components/animationmanager.hh"
#include "components/importer.hh"
#include "components/pipeline.hh"

#define DEF_NEAR_CLIP_PLANE 0.01f
#define DEF_FAR_CLIP_PLANE 10000.0f

void Renderer::setup_render_properties() {
  // render mode
  if (m_render_mode_wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  
  //TMP make light move in a circle
  m_active_scene->m_loaded_lights[0].set_light_look_at(-10,0,0);
  m_active_scene->m_loaded_lights[0].set_light_position(-10 + 10*sin(m_application_current_time/10),10,10*cos(m_application_current_time/10));
  
  // m_active_scene->m_loaded_entities[0].m_mesh[8]->change_rotation(0.0f,glm::radians(m_deltaTime*500.0f), 0.0f);
  //m_active_scene->m_loaded_entities[0].change_rotation(0.0f,glm::radians(m_deltaTime*25.0f), 0.0f);
  
}

void Renderer::framebuffer_size_callback(GLFWwindow *window, int width,
                                         int height) {
  Logger::log_success("framebuffer resized.");
  glViewport(0, 0, width, height);

  //TODO move all the 15 different locations where i store size of FB to one, that gets updated here

}

void Renderer::update_scene_time() {
  // bungie employees hate this simple trick
  float currentFrame = glfwGetTime();
  m_deltaTime = currentFrame - m_application_current_time;
  m_application_current_time = currentFrame;
  //update dT in activeScene.
  m_active_scene->m_scene_deltatime = m_deltaTime;
  m_active_scene->m_scene_abstime = m_application_current_time;
}

void Renderer::abstract_render() {
  //now render frame
  if(m_active_scene->render_properties.cull_scene)
    m_culling_manager->cull_scene();

  m_pipeline->render_frame();

  }

void Renderer::render_frame() {
  
  if (!m_active_scene->m_local_player) {
    Logger::log_error("no local player with camera in scene! stopping render!");
    return;
  }
  if (m_active_scene->m_loaded_lights.size() < 1) {
    Logger::log_error(
        "not enough lights loaded for shader to function. stopping render.");
    return;
  }
  if (m_active_scene->m_loaded_entities.size() < 1) {
    Logger::log_error(
        "not enough lights loaded for shader to function. stopping render.");
    return;
  }

  update_scene_time();

  // handle all abstracted stuff that changes the scene somehow
  setup_render_properties();

  // run external modules
  m_animation_manager->handle_scene_animations(m_application_current_time);
  m_physics_manager->handle_scene_physics();
  m_input_manager->process_input(associated_window, m_application_current_time, m_deltaTime);

  // handle frametime + fps counter
  m_frame_count++;
  double current_time = glfwGetTime();
  if (current_time - m_last_fps_time >= 1.0) {
    m_fps = static_cast<float>(m_frame_count) / static_cast<float>(current_time - m_last_fps_time);
    m_frame_count = 0;
    m_last_fps_time = current_time;
    m_fps_dirty = true;
  }
  if (m_fps_dirty) {
    m_fps_display_text = "FPS: " + std::to_string(static_cast<int>(m_fps));
    m_fps_dirty = false;
    m_active_scene->m_scene_vbos_need_refresh = true;
  }
  if(m_active_scene->m_loaded_overlay_elements.size() > 0)
    m_active_scene->m_loaded_overlay_elements[0]->edit_text(m_fps_display_text);
  else
    Logger::log_error("overlay doesnt have fps element");
  
  // make sure data changes from anim / get reflected in VRAM
  if(m_active_scene->m_scene_vbos_need_refresh)
    init_scene_vbos();
  
  // setup constants for render pass
  glfwGetWindowSize(associated_window, &m_active_scene->m_scene_framebuffer_width, &m_active_scene->m_scene_framebuffer_height);
  
  //launch depth render pass impl
  abstract_render();

  // draw to screen
  glfwSwapBuffers(associated_window);
  glfwPollEvents();

  // hit fps target 
  /* if(m_deltaTime < fps_target_ms)
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(fps_target_ms) - (int)(m_deltaTime)));
  */}

void Renderer::create_angle_constraint(std::shared_ptr<Point> p, std::shared_ptr<Point> q,
				       std::shared_ptr<Point> hinge, float angle) {
  
  m_active_scene->m_loaded_constraints.push_back(std::make_shared<Fix_angle_constraint>(p,q,hinge,angle));
  
};

void Renderer::create_length_constraint(std::shared_ptr<Point> p,
                              std::shared_ptr<Point> q, float distance) {

  m_active_scene->m_loaded_constraints.push_back(std::make_shared<Fix_length_constraint>(p,q,distance));
  
};

void Renderer::add_model_to_scene(const char* filepath) {

  Entity load_entity;
  load_entity.m_mesh = Importer::load_all_meshes_from_gltf(filepath, num_loaded_textures, m_texture_map);
  m_active_scene->add_entity_to_scene(load_entity);

  m_active_scene->m_scene_vbos_need_refresh = true;
  
}

void Renderer::add_skybox_to_scene(const char* filepath) {

  Entity load_entity;
  load_entity.m_mesh = Importer::load_all_meshes_from_gltf(filepath, num_loaded_textures, m_texture_map);
  for(auto& me : load_entity.m_mesh)
    me->m_mesh_type = E_SKYBOX;
  m_active_scene->add_entity_to_scene(load_entity);

  m_active_scene->m_scene_vbos_need_refresh = true;
  
}

void Renderer::add_model_to_player_hand(const char* filepath) {

  Entity load_entity;
  load_entity.m_mesh = std::move(
				 Importer::load_all_meshes_from_gltf(filepath, num_loaded_textures, m_texture_map));
  load_entity.is_held_by_localplayer = true;
  m_active_scene->add_entity_to_scene(load_entity);

  m_active_scene->m_scene_vbos_need_refresh = true;
}


void Renderer::init_scene(const char *scene_fp) {
  
  m_pipeline = std::make_unique<Shadow_Map_Pipeline>();
  
  Entity load_entity;
  load_entity.m_mesh = std::move(
				 Importer::load_all_meshes_from_gltf(scene_fp, num_loaded_textures, m_texture_map));

  // vsync xd
  //glfwSwapInterval(1);
  glfwSwapInterval(0);

  //initialize scene + components
  m_active_scene = std::make_shared<Scene>();
  m_input_manager->m_active_scene = m_active_scene;
  m_animation_manager->m_active_scene = m_active_scene;
  m_physics_manager->m_active_scene = m_active_scene;
  m_pipeline->m_active_scene = m_active_scene;
  m_culling_manager = std::make_unique<Culler>(m_active_scene);

  Logger::log_success("set Scene ptr to IM, AM and PM");

  //make player (localplayer) their camera
  add_player_to_scene(true);
  
  std::vector<std::shared_ptr<Mesh>> light_vec = Importer::load_all_meshes_from_gltf(
										     "models/light/scene.gltf", num_loaded_textures, m_texture_map);
  
  
  Light main_light(light_vec[0]);
  main_light.m_light_type = E_POINT_LIGHT;
  main_light.m_color = 0xFFFFFF;
  main_light.m_strength = 7.0f;
  main_light.light_width = 25.0f;

  main_light.set_light_position(10,8,5);
  
  m_active_scene->add_entity_to_scene(load_entity);
  m_active_scene->add_light_to_scene(main_light);

  // initialize scene vbos
  init_scene_vbos();

  // TODO check if init scenes shader programs needed?
  Logger::log_success("Finished initialization for Shader Programs");

  //TMP setup vbos n shi
  m_pipeline->init_pipeline();
  
  Logger::log_success("done initializing renderer.");
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
    Logger::log_error("Scene doesn't contain at least one light + entity, not initializing VBOs");
    return;
  }

  Logger::log_debug("Initializing/Updating VBOs for scene...");

  /////////////////////////////////////////
  // Update Light VBOs (for visualizers) //
  /////////////////////////////////////////
  for (auto &light : m_active_scene->m_loaded_lights) {
    if ( light.m_light_visualizer_mesh == nullptr || !light.m_light_visualizer_mesh->m_mesh_vbo_needs_refresh) // TODO make sure light vis mesh ptr isnt nullptr
      continue;
    
    auto &mesh = light.m_light_visualizer_mesh;

    // Clean up existing GL resources if they exist
    cleanup_mesh_vbos(*mesh);

    // Generate new VAO + VBOs
    glGenVertexArrays(1, &mesh->m_mesh_vao);
    glBindVertexArray(mesh->m_mesh_vao);

    // Vertices
    if (!mesh->m_vertices_array.empty()) {
      glGenBuffers(1, &mesh->m_vertices_glid);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vertices_glid);
      glBufferData(GL_ARRAY_BUFFER,
                   mesh->m_vertices_array.size() * sizeof(float),
                   mesh->m_vertices_array.data(),
                   GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);
    }

    // TexCoords (optional)
    if (!mesh->m_tex_coords_array.empty()) {
      glGenBuffers(1, &mesh->m_tex_coords_glid);
      glBindBuffer(GL_ARRAY_BUFFER, mesh->m_tex_coords_glid);
      glBufferData(GL_ARRAY_BUFFER,
                   mesh->m_tex_coords_array.size() * sizeof(float),
                   mesh->m_tex_coords_array.data(),
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(1);
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh->m_mesh_vbo_needs_refresh = false;
  }

  /////////////////////////////
  // Update Entity Mesh VBOs //
  /////////////////////////////
  for (Entity &entity : m_active_scene->m_loaded_entities) {
    for (std::shared_ptr<Mesh> &mesh : entity.m_mesh) {
      
      if (!mesh->m_mesh_vbo_needs_refresh)
        continue;

      // Clean up old buffers to prevent leaks
      cleanup_mesh_vbos(*mesh);

      // Recalculate normals if missing
      if (mesh->m_normals_array.empty()) {
        Logger::log_debug("Mesh missing normals, recalculating...");
        mesh->m_normals_array = Importer::calculate_vert_normals(mesh->m_vertices_array);
      }

      // Recalculate tangents/binormals if needed and texcoords exist
      if (!mesh->m_tex_coords_array.empty()) {
        if (mesh->m_tangents_array.empty() ) {
          Logger::log_debug("Missing tangents, calculating...");
	  Importer::tan_bin_glob tb = Importer::calculate_vert_tan_bin(
              mesh->m_vertices_array, mesh->m_normals_array, mesh->m_tex_coords_array);
          mesh->m_tangents_array = tb.vert_tangents;
        }
      } else {
        Logger::log_error("Mesh has no UVs; using zeroed tangents");
        mesh->m_tangents_array.resize(mesh->m_vertices_array.size(), 0.0f);
      }

      // Create VAO
      glGenVertexArrays(1, &mesh->m_mesh_vao);
      glBindVertexArray(mesh->m_mesh_vao);

      // verts
      if (!mesh->m_vertices_array.empty()) {
        glGenBuffers(1, &mesh->m_vertices_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vertices_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh->m_vertices_array.size() * sizeof(float),
                     mesh->m_vertices_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
      }

      // tex coords
      if (!mesh->m_tex_coords_array.empty()) {
        glGenBuffers(1, &mesh->m_tex_coords_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_tex_coords_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh->m_tex_coords_array.size() * sizeof(float),
                     mesh->m_tex_coords_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
      }

      // normals
      if (!mesh->m_normals_array.empty()) {
        glGenBuffers(1, &mesh->m_normals_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_normals_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh->m_normals_array.size() * sizeof(float),
                     mesh->m_normals_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
      }

      // tangents
      if (!mesh->m_tangents_array.empty()) {
        glGenBuffers(1, &mesh->m_tangents_glid);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_tangents_glid);
        glBufferData(GL_ARRAY_BUFFER,
                     mesh->m_tangents_array.size() * sizeof(float),
                     mesh->m_tangents_array.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(3);
      }

      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      mesh->m_mesh_vbo_needs_refresh = false;
    }
  }

  ///////////////////
  // Initialize text vbos
  ///////////////////
  for(std::shared_ptr<Overlay_Element>& oe : m_active_scene->m_loaded_overlay_elements) {

    if(!oe->element_needs_vbo_update && !m_active_scene->reinit_text_vbos)
      continue;

    if (oe->text_vao != 0) {
      glDeleteVertexArrays(1, &oe->text_vao);
      oe->text_vao = 0;
    }
    if (oe->text_vertices_vbo != 0) {
      glDeleteBuffers(1, &oe->text_vertices_vbo);
      oe->text_vertices_vbo = 0;
    }
    if (oe->text_uv_vbo != 0) {
      glDeleteBuffers(1, &oe->text_uv_vbo);
      oe->text_uv_vbo = 0;
    }

    // this is a bit of a mess but itll do
    glGenVertexArrays(1, &oe->text_vao);
    glBindVertexArray(oe->text_vao);
    
    oe->uv_coords = m_active_scene->texture_atlas.get_glyph_UV_sequence_for_string(oe->text);
    oe->text_vert_coords_screen_space = m_active_scene->texture_atlas.get_glyph_vert_cords_for_string(oe->text, m_active_scene->m_scene_framebuffer_width, m_active_scene->m_scene_framebuffer_height, oe);

    glGenBuffers(1, &oe->text_vertices_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, oe->text_vertices_vbo);
    glBufferData(GL_ARRAY_BUFFER,
		 oe->text_vert_coords_screen_space.size() * sizeof(float), oe->text_vert_coords_screen_space.data() , GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &oe->text_uv_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, oe->text_uv_vbo);
    glBufferData(GL_ARRAY_BUFFER,
		 oe->uv_coords.size() * sizeof(float), oe->uv_coords.data() , GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);    
    glEnableVertexAttribArray(1);

    Logger::log_error("INITIALIZEDD TEXT!!");

    oe->element_needs_vbo_update = false;
    
  }
  m_active_scene->reinit_text_vbos = false;
  
  ///////////////////
  // Update signed distance field shared vbo
  ///////////////////
  if(!m_active_scene->shared_sdf_vao_initialized)
    {
      if (m_active_scene->shared_sdf_vao != 0) {
	glDeleteVertexArrays(1, &m_active_scene->shared_sdf_vao);
	m_active_scene->shared_sdf_vao = 0;
      }
      if (m_active_scene->shared_sdf_vbo != 0) {
	glDeleteBuffers(1, &m_active_scene->shared_sdf_vbo);
	m_active_scene->shared_sdf_vbo = 0;
      }
      // Create VAO n load fullscreen tri xd
      glGenVertexArrays(1, &m_active_scene->shared_sdf_vao);
      glBindVertexArray(m_active_scene->shared_sdf_vao);
      std::vector<float> tmp = {-1.0f,-1.0f, -1.0f,
				3.0f, -1.0f, -1.0f,
				-1.0f, 3.0f, -1.0f};
      glGenBuffers(1, &m_active_scene->shared_sdf_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, m_active_scene->shared_sdf_vbo);
      glBufferData(GL_ARRAY_BUFFER,
		   tmp.size() * sizeof(float), tmp.data() , GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);
      
    }
  //done
  m_active_scene->m_scene_vbos_need_refresh = false;
  Logger::log_success("Successfully initialized/updated VBOs for all dirty meshes!");
}

Renderer::Renderer(uint window_width, uint window_height) {

  std::cout << R"(                     __                 
  ____  ____________/  |_  ____ ___  ___
_/ ___\/  _ \_  __ \   __\/ __ \\  \/  /
\  \__(  <_> )  | \/|  | \  ___/ >    < 
 \___  >____/|__|   |__|  \___  >__/\_ \
     \/                       \/      \/
========================================
 * an engine coded by leander hofmann *
========================================
)";

  Logger::log_debug("initializing window");

  // create input manager
  Logger::log_error("init IM with broken pointer");
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
    Logger::log_error("failed to create glfw window!");
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(associated_window);

  // initiate glad
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    Logger::log_error("failed to load glad!");
    return;
  }

  // setup
  glViewport(0, 0, 1920, 1080);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // format for class usage
  glfwSetFramebufferSizeCallback(associated_window,
                                 Renderer::framebuffer_size_callback);
  glfwSetWindowUserPointer(associated_window, m_input_manager.get());

  //FB resize callback
  glfwSetFramebufferSizeCallback(associated_window, [](GLFWwindow *w, int width,
                                              int height) {
    Input_Manager *imanager = static_cast<Input_Manager *>(glfwGetWindowUserPointer(w));
    
    if (imanager) {
      imanager->fb_resize_callback(w, width, height);
    }
  });
  
  // scroll callback
  glfwSetScrollCallback(associated_window, [](GLFWwindow *w, double xoffset,
                                              double yoffset) {
    Input_Manager *imanager = static_cast<Input_Manager *>(glfwGetWindowUserPointer(w));
    
    if (imanager) {
      imanager->scroll_callback(w, xoffset, yoffset);
    }
  });

  // mouse move callback
  glfwSetCursorPosCallback(associated_window, [](GLFWwindow *w, double xpos,
                                                 double ypos) {
    Input_Manager *imanager = static_cast<Input_Manager *>(glfwGetWindowUserPointer(w));
    if (imanager) {
      imanager->mouse_callback(w, xpos, ypos);
    }
  });

  glfwSetMouseButtonCallback(associated_window, [](GLFWwindow *w, int button, int action, int mods) {
    Input_Manager *imanager = static_cast<Input_Manager *>(glfwGetWindowUserPointer(w));
    if (imanager) {
      imanager->mouse_button_callback(w, button, action, mods);
    }
  });
  
  glfwSetInputMode(associated_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  return;
  
}

void Renderer::add_point_to_scene(float x, float y, float z) {
  std::shared_ptr<Point> to_add = std::make_shared<Point>(x,y,z);
  m_active_scene->add_point_to_scene(to_add);
}

void Renderer::create_spring_constraint(std::shared_ptr<Point> from,
                                        std::shared_ptr<Point> to,
                                        float strength,
					float rest_length) {
  
  m_active_scene->m_loaded_springs.push_back(std::make_shared<Spring>(from,to,strength));
  m_active_scene->m_loaded_force_generators.push_back(std::make_shared<Spring_force_generator>(from,to,strength,rest_length));
  
};

void Renderer::create_fixed_constraint(std::shared_ptr<Point> p, bool fixed) {
  p->phys_props.fixed = fixed;
}

void Renderer::add_player_to_scene(bool make_local_player) {

  m_active_scene->m_player_list.push_back(std::make_shared<Player>());
  
  if(make_local_player) {
    int index = m_active_scene->m_player_list.size();
    m_active_scene->m_local_player = m_active_scene->m_player_list[index - 1];
  }
  
};

void Renderer::add_text_to_overlay(std::string to_add, unsigned int anchor_x,
                                   unsigned int anchor_y) {

  glfwGetFramebufferSize(associated_window, &m_active_scene->m_scene_framebuffer_width, &m_active_scene->m_scene_framebuffer_height);
  
  m_active_scene->add_text_to_overlay(to_add,anchor_x,anchor_y,num_loaded_textures,m_texture_map);
  
};

void Renderer::set_fps_target(int new_target) {

  fps_target = new_target;
  fps_target_ms = 1000.0f/(float)fps_target;
  
};
