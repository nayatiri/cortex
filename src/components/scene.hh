#pragma once

#include "entity.hh"
#include "force_generator.hh"
#include "light.hh"
#include "camera.hh"
#include "point.hh"
#include "spring.hh"

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
  void add_point_to_scene(std::shared_ptr<Point> to_add);

  Scene();
  
  std::vector<Entity> m_loaded_entities = {};
  std::vector<Light> m_loaded_lights = {};
  std::vector<std::shared_ptr<Spring>> m_loaded_springs = {};
  std::vector<std::shared_ptr<Point>> m_loaded_points = {};
  std::vector<std::shared_ptr<Force_generator>> m_loaded_force_generators = {};

  std::unique_ptr<Camera> m_camera;

  bool m_scene_vbos_need_refresh = false;

  float m_scene_deltatime = 0;
  float m_scene_abstime = 0;

  //TODO: make this not be here xd
  int m_scene_framebuffer_width;
  int m_scene_framebuffer_height;

  //universal shaders (sdf magic) :3
  Shader universal_hitbox_shader;
  Shader universal_line_shader;
  Shader universal_point_shader;
  
};
