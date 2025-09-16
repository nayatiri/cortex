#include "collission.hh"
#include <glm/ext/quaternion_geometric.hpp>

Point_Point_Collission::Point_Point_Collission() {

  contact_normal = point_a->get_position() - point_b->get_position();
  contact_normal = glm::normalize(contact_normal);
  
}
