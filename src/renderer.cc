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
#include <type_traits>
#include <vector>
#include <iostream>

// components
#include "components/entity.hh"
#include "components/force_generator.hh"
#include "components/input.hh"
#include "components/light.hh"
#include "components/logging.hh"
#include "components/point.hh"
#include "components/mesh.hh"
#include "components/scene.hh"
#include "components/animationmanager.hh"
#include "components/importer.hh"
#include "components/pipeline.hh"
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
}

void Renderer::framebuffer_size_callback(GLFWwindow *window, int width,
                                         int height) {
  log_success("framebuffer resized.");
  glViewport(0, 0, width, height);
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
  m_pipeline->render_frame();
}

void Renderer::render_frame() {
  
  if (!m_active_scene->m_local_player) {
    log_error("no local player with camera in scene! stopping render!");
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

  update_scene_time();

  // handle all abstracted stuff that changes the scene somehow
  setup_render_properties();

  // run external modules
  m_animation_manager->handle_scene_animations(m_application_current_time);
  m_physics_manager->handle_scene_physics();
  m_input_manager->process_input(associated_window, m_application_current_time, m_deltaTime);

  // make sure data changes from anim / get reflected in VRAM
  if(m_active_scene->m_scene_vbos_need_refresh)
    init_scene_vbos();
  
  // setup constants for render pass
  glfwGetWindowSize(associated_window, &m_viewport_width, &m_viewport_height);
  m_active_scene->m_scene_framebuffer_width = m_viewport_width;
  m_active_scene->m_scene_framebuffer_height = m_viewport_height;

  //TMP unclean
  m_pipeline->m_viewport_height = m_viewport_height;
  m_pipeline->m_viewport_width = m_viewport_width;
  
  //launch depth render pass impl
  abstract_render();

  // draw to screen
  glfwSwapBuffers(associated_window);
  glfwPollEvents();
}

void Renderer::add_model_to_scene(const char* filepath) {

  Entity load_entity;
  load_entity.m_mesh = std::move(
				 Importer::load_all_meshes_from_gltf(filepath, num_loaded_textures, m_texture_map));
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

  //  glDisable(GL_CULL_FACE);

  //initialize scene + components
  m_active_scene = std::make_shared<Scene>();
  m_input_manager->m_active_scene = m_active_scene;
  m_animation_manager->m_active_scene = m_active_scene;
  m_physics_manager->m_active_scene = m_active_scene;
  m_pipeline->m_active_scene = m_active_scene;

  log_success("set Scene ptr to IM, AM and PM");

  //make player (localplayer) their camera
  add_player_to_scene(true);
  
  Light main_light(std::move(Importer::load_all_meshes_from_gltf(
      "models/light/scene.gltf", num_loaded_textures, m_texture_map))[0]);
  main_light.m_light_type = E_POINT_LIGHT;
  main_light.m_color = 0xFFFFFF;
  main_light.m_strength = 10;

  main_light.set_light_position(10,8,5);
  
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

  //TMP setup vbos n shi
  m_pipeline->init_pipeline();
  
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

// TOOD: make one shared VBO / VAO for fullscreen triangle to render all SDFs
// too. wowza so vram light omg im creaming

void Renderer::init_scene_vbos() {
  if (m_active_scene->m_loaded_entities.empty() ||
      m_active_scene->m_loaded_lights.empty()) {
    log_error("Scene doesn't contain at least one light + entity, not initializing VBOs");
    return;
  }

  log_debug("Initializing/Updating VBOs for scene...");

  /////////////////////////////////////////
  // Update Light VBOs (for visualizers) //
  /////////////////////////////////////////
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

  /////////////////////////////
  // Update Entity Mesh VBOs //
  /////////////////////////////
  for (Entity &entity : m_active_scene->m_loaded_entities) {
    for (Mesh &mesh : entity.m_mesh) {
      
      if (!mesh.m_mesh_vbo_needs_refresh)
        continue;

      log_debug_sub("Reinitializing VBOs for mesh (needs refresh)");

      // Clean up old buffers to prevent leaks
      cleanup_mesh_vbos(mesh);

      // Recalculate normals if missing
      if (mesh.m_normals_array.empty()) {
        log_debug("Mesh missing normals, recalculating...");
        mesh.m_normals_array = Importer::calculate_vert_normals(mesh.m_vertices_array);
      }

      // Recalculate tangents/binormals if needed and texcoords exist
      if (!mesh.m_tex_coords_array.empty()) {
        if (mesh.m_tangents_array.empty() || mesh.m_binormals_array.empty()) {
          log_debug("Missing tangents/binormals, calculating...");
	  Importer::tan_bin_glob tb = Importer::calculate_vert_tan_bin(
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

  ///////////////////
  // Update Point VBOs
  ///////////////////
  for(std::shared_ptr<Point> p : m_active_scene->m_loaded_points) {
      //if entity is a point, we can simply init a mesh,
      //and init it with 3 0's as a vbo since we renderb
      //the dots as signed distance fields anyway and
      //dont need a vbo with vertex data.      
      log_debug_sub("Reinitializing VBOs for point (needs refresh [this shouldnt happen more than once lmao])");

      if (p->VAO_id != 0) {
	glDeleteVertexArrays(1, &p->VAO_id);
	p->VAO_id = 0;
      }
      if (p->VBO_vertices != 0) {
	glDeleteBuffers(1, &p->VBO_vertices);
	p->VBO_vertices = 0;
      }
      
      // Create VAO n load fullscreen tri xd
      glGenVertexArrays(1, &p->VAO_id);
      glBindVertexArray(p->VAO_id);
      std::vector<float> tmp = {-1.0f,-1.0f, -1.0f,
				3.0f, -1.0f, -1.0f,
				-1.0f, 3.0f, -1.0f};
      glGenBuffers(1, &p->VBO_vertices);
      glBindBuffer(GL_ARRAY_BUFFER, p->VBO_vertices);
      glBufferData(GL_ARRAY_BUFFER,
		   tmp.size() * sizeof(float), tmp.data() , GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);

  }

  ///////////////////
  // Update Spring VBOs
  ///////////////////
  for(std::shared_ptr<Spring> s : m_active_scene->m_loaded_springs) {
      //if entity is a point, we can simply init a mesh,
      //and init it with 3 0's as a vbo since we renderb
      //the dots as signed distance fields anyway and
      //dont need a vbo with vertex data.      
      log_debug_sub("Reinitializing VBOs for point (needs refresh [this shouldnt happen more than once lmao])");

      if (s->VAO_id != 0) {
	glDeleteVertexArrays(1, &s->VAO_id);
	s->VAO_id = 0;
      }
      if (s->VBO_vertices != 0) {
	glDeleteBuffers(1, &s->VBO_vertices);
	s->VBO_vertices = 0;
      }
      
      // Create VAO n load fullscreen tri xd
      glGenVertexArrays(1, &s->VAO_id);
      glBindVertexArray(s->VAO_id);
      std::vector<float> tmp = {-1.0f,-1.0f, -1.0f,
				3.0f, -1.0f, -1.0f,
				-1.0f, 3.0f, -1.0f};
      glGenBuffers(1, &s->VBO_vertices);
      glBindBuffer(GL_ARRAY_BUFFER, s->VBO_vertices);
      glBufferData(GL_ARRAY_BUFFER,
		   tmp.size() * sizeof(float), tmp.data() , GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);

  }

  ///////////////////////////////
  // Update Entity Hitbox VBOs //
  ///////////////////////////////
  for (Entity &entity : m_active_scene->m_loaded_entities) {
    for (Mesh &mesh : entity.m_mesh) {

      if(mesh.AABB_visualizer == nullptr)
	continue;
      
      log_debug_sub("Reinitializing VBOs for meshes hitbox (needs refresh)");

      AABB_Box* toadjust = mesh.AABB_visualizer.get();
      
      auto delete_vao = [](GLuint& vao_id) {
	if (vao_id != 0) {
	  glDeleteVertexArrays(1, &vao_id);
	  vao_id = 0;
	}
      };
      
      auto delete_vbo = [](GLuint& vbo_id) {
	if (vbo_id != 0) {
	  glDeleteBuffers(1, &vbo_id);
	  vbo_id = 0;
	}
      };
      
      delete_vao(toadjust->m_mesh_vao);
      delete_vbo(toadjust->m_vertices_glid);
      
      // Create VAO n load 0es xd
      glGenVertexArrays(1, &toadjust->m_mesh_vao);
      glBindVertexArray(toadjust->m_mesh_vao);
      std::vector<float> tmp = {-1.0f,-1.0f, -1.0f,
				3.0f, -1.0f, -1.0f,
				-1.0f, 3.0f, -1.0f};
      glGenBuffers(1, &toadjust->m_vertices_glid);
      glBindBuffer(GL_ARRAY_BUFFER, toadjust->m_vertices_glid);
      glBufferData(GL_ARRAY_BUFFER,
		   tmp.size() * sizeof(float), tmp.data() , GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
      glEnableVertexAttribArray(0);
      
    }
  }
  
  m_active_scene->m_scene_vbos_need_refresh = false;
  log_success("Successfully initialized/updated VBOs for all dirty meshes!");
}

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
