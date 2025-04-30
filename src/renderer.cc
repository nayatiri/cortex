#include "renderer.hh"

//components
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
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdint>

// assimp
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

//components (custom)
#include "./components/material.hh"
#include "./components/entity.hh"
#include "./components/importer.hh"
#include "./shaders/shader.hh"
#include "./components/utility.hh"

/////////////////////
// CALLBACK FUNCTIONS
/////////////////////

void Renderer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

  Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));

  std::cout << "Scroll offset: (" << xoffset << ", " << yoffset << ")\n";

  m_camera_base_speed = m_camera_base_speed + static_cast<float>(yoffset) * 0.1f;

  std::cout << m_camera_base_speed << std::endl;

  if(m_camera_base_speed < 0.1f) {m_camera_base_speed = 0.1f;}
  
}

void Renderer::mouse_callback(GLFWwindow *window, double xpos, double ypos) {

  std::cout << " mouse callback triggered \n";
  
  if(!m_is_mouse_grabbed) {return;}

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

  if(m_is_mouse_on_cooldown) {
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

void Renderer::framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  std::cout << "framebuffer resized!" << std::endl;
  glViewport(0, 0, width, height);
}

void Renderer::processInput(GLFWwindow *window) {

  if(window == nullptr)
    std::cout << "window is null!!! wont work" << std::endl;
  
  std::cout << "processInput call..." << std::endl;
  
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
    m_should_shutdown = true;
    std::cout << "shutdown signal recieved... propagating..." << std::endl;
  }

  float cameraSpeed = m_camera_base_speed * 10.0f * m_deltaTime;
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {m_cameraPos += cameraSpeed * glm::normalize(glm::vec3(m_cameraLookAt.x,0.0f,m_cameraLookAt.z)); }
    
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {m_cameraPos += cameraSpeed * glm::normalize(glm::vec3(-m_cameraLookAt.x,0.0f,-m_cameraLookAt.z)); }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    m_cameraPos -=
        glm::normalize(glm::cross(m_cameraLookAt, m_cameraUp)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    m_cameraPos +=
        glm::normalize(glm::cross(m_cameraLookAt, m_cameraUp)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    m_cameraPos +=
      glm::normalize(glm::vec3(0.0f,-1.0f,0.0f)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    m_cameraPos +=
      glm::normalize(glm::vec3(0.0f,1.0f,0.0f)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
    if (m_last_mouse_state == false) {
      m_is_mouse_grabbed = !m_is_mouse_grabbed; // Toggle the state
      if (m_is_mouse_grabbed) {
        m_is_mouse_on_cooldown = true;
        glfwSetInputMode(window, GLFW_CURSOR,
                         GLFW_CURSOR_DISABLED); // Grab the mouse
      } else {
        m_is_mouse_on_cooldown = true;
        glfwSetInputMode(window, GLFW_CURSOR,
                         GLFW_CURSOR_NORMAL); // Release the mouse
      }
      m_last_mouse_state = true;
    }
  }

  if (glfwGetKey(window,GLFW_KEY_G) != GLFW_PRESS) {

    m_last_mouse_state = false;

  }

  return;
  
}

Renderer::Renderer(uint window_width, uint window_height) {

    std::cout << "init window" << std::endl;

    // Create the window for this renderer
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow *window = glfwCreateWindow(window_width, window_height, "cortex - dev build", NULL, NULL);
    if (window == NULL) {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return;
    }
    glfwMakeContextCurrent(window);
    
    //initiate glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      std::cout << "Failed to initialize GLAD" << std::endl;
      return;
    }
    
    // setup
    glViewport(0, 0, window_width, window_height);
    glEnable(GL_DEPTH_TEST);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // format for class usage
    glfwSetFramebufferSizeCallback(window, Renderer::framebuffer_size_callback);
    glfwSetWindowUserPointer(window, this);
    
    // set callbacks using lambda functions
    glfwSetScrollCallback(window, [](GLFWwindow* w, double xoffset, double yoffset) {
      Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(w));
      if (renderer) {
	renderer->scroll_callback(w, xoffset, yoffset);
      }
    });
    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
      Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(w));
      if (renderer) {
	renderer->mouse_callback(w, xpos, ypos);
      }
    });
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //debug render mode
    if (m_render_mode_wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    /// tmp for texting area

    Material material(E_PBR);
    
    /// end tmp for texting

    //prepare vbos of loaded scene, if it contains necessary items
    //TODO: move into function that can be called at runtime to update vbos
    if(m_loaded_scene.m_loaded_entities.size() > 0 &&
       m_loaded_scene.m_loaded_lights.size() > 0 ) {
      
      for(auto entity_to_render : m_loaded_scene.m_loaded_entities) {
	
	for(auto mesh_of_entity : entity_to_render.m_mesh) {

	  ////////////////////////////
	  // GENERATE MISSING GEOMETRY
	  ////////////////////////////
	  
	  //calculate mesh vertex normals + track vertex count for lolz
	  
	  mesh_of_entity.m_normals_array = calculate_vert_normals(mesh_of_entity.m_vertices_array);
	  //total_vertices = total_vertices + sub_mesh.mesh_vertices.size();
	  
	  //calculate vertex tangents / binormals
	  tan_bin_glob retglob = calculate_vert_tan_bin(mesh_of_entity.m_vertices_array,
						        mesh_of_entity.m_normals_array,
						        mesh_of_entity.m_tex_coords_array);
	  
	  mesh_of_entity.m_binormals_array = retglob.vert_binormals;
	  mesh_of_entity.m_tangents_array = retglob.vert_tangents;
	  
	  // vao
	  glGenVertexArrays(1, &mesh_of_entity.m_mesh_vao);
	  glBindVertexArray(mesh_of_entity.m_mesh_vao);
	  
	  // vbo mesh (vertices)
	  glGenBuffers(1, &mesh_of_entity.m_vertices_glid);
	  glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_vertices_glid);
	  glBufferData(GL_ARRAY_BUFFER, mesh_of_entity.m_vertices_array.size() * sizeof(float),
		       mesh_of_entity.m_vertices_array.data(), GL_STATIC_DRAW);
	  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
				(void *)0);
	  glEnableVertexAttribArray(0);	  
	  
	  // vbo mesh (tex coords)
	  //TODO fix layout of vertexattribpointer
	  glGenBuffers(1, &mesh_of_entity.m_tex_coords_glid);
	  glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_tex_coords_glid);
	  glBufferData(GL_ARRAY_BUFFER, mesh_of_entity.m_tex_coords_array.size() * sizeof(float),
		       mesh_of_entity.m_tex_coords_array.data(), GL_STATIC_DRAW);
	  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
				(void *)0);
	  glEnableVertexAttribArray(1);	  

	  // vbo mesh (normals)
	  glGenBuffers(1, &mesh_of_entity.m_normals_glid);
	  glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_normals_glid);
	  glBufferData(GL_ARRAY_BUFFER, mesh_of_entity.m_normals_array.size() * sizeof(float),
		       mesh_of_entity.m_normals_array.data(), GL_STATIC_DRAW);
	  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
				(void *)0);
	  glEnableVertexAttribArray(2);

	  // vbo mesh (tangents)
	  glGenBuffers(1, &mesh_of_entity.m_tangents_glid);
	  glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_tangents_glid);
	  glBufferData(GL_ARRAY_BUFFER, mesh_of_entity.m_tangents_array.size() * sizeof(float),
		       mesh_of_entity.m_tangents_array.data(), GL_STATIC_DRAW);
	  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
				(void *)0);
	  glEnableVertexAttribArray(3);

	  // vbo mesh (bitangents)
	  glGenBuffers(1, &mesh_of_entity.m_binormals_glid);
	  glBindBuffer(GL_ARRAY_BUFFER, mesh_of_entity.m_binormals_glid);
	  glBufferData(GL_ARRAY_BUFFER, mesh_of_entity.m_binormals_array.size() * sizeof(float),
		       mesh_of_entity.m_binormals_array.data(), GL_STATIC_DRAW);
	  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
				(void *)0);
	  glEnableVertexAttribArray(4);	 
	}

      }

    }

    
    //main render loop
    while (!glfwWindowShouldClose(window)) {
      
      processInput(window);

      glfwPollEvents();
      
      std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }
    
    std::cout << "shutdown signal, shutting down!" << std::endl;

    return;
  
}
