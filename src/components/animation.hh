#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

class animation {
  
public:
  std::vector<glm::vec3> m_checkpoints;
  unsigned int m_last_checkpoint;
  
  bool m_trigger_animation;
  
  float m_start_time;
  float m_checkpoint_delta_time;
  float m_animation_speed;
  
};
