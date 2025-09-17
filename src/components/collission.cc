#include "collission.hh"
#include <glm/ext/quaternion_geometric.hpp>

void Collission::resolve_collission() {}

Point_Point_Collission::Point_Point_Collission() {

  contact_normal = point_a->get_position() - point_b->get_position();
  contact_normal = glm::normalize(contact_normal);
  
}

void Point_Point_Collission::resolve_collission(){};


