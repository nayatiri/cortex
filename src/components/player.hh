#include "camera.hh"
#include <memory>
class Player {
public:

  std::shared_ptr<Camera> m_player_camera;

  Player();
  
  glm::vec3 get_position();
  
};
