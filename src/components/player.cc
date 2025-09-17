#include "player.hh"

glm::vec3 Player::get_position() {

  if(m_player_camera != nullptr)
    return m_player_camera->m_cameraPos;
  else
    return {0,0,0};
}


Player::Player() {

  m_player_camera = std::make_shared<Camera>();
  
}
