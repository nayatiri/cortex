#include "visibility_manager.hh"

int Visibility_Manager::get_sign_of_point_relative_plane(glm::vec3 plp1,
                                                         glm::vec3 plp2,
                                                         glm::vec3 plp3,
                                                         glm::vec3 point) {


  
  
  return -1;
  
}

inline float Visibility_Manager::fastDeterminant(const glm::mat3& m) {
    return m[0].x * (m[1].y * m[2].z - m[2].y * m[1].z) -
           m[1].x * (m[0].y * m[2].z - m[2].y * m[0].z) +
           m[2].x * (m[0].y * m[1].z - m[1].y * m[0].z);
}
