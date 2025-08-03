#pragma once

#include "entity.hh"
#include "light.hh"
#include "camera.hh"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stdlib
#include <memory>
#include <vector>

class Scene {
public:

  void add_entity_to_scene(Entity to_add);
  void add_light_to_scene(Light to_add);
  
  std::vector<Entity> m_loaded_entities; 
  std::vector<Light> m_loaded_lights;

  std::unique_ptr<Camera> m_camera;
  
};
