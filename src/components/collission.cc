#include "collission.hh"
#include <glm/ext/quaternion_geometric.hpp>

void Collission::resolve_collission() {}

Point_Point_Collission::Point_Point_Collission( std::shared_ptr<Point> a, std::shared_ptr<Point> b, glm::vec3 c_normal ) {

  point_a = a;
  point_b = b;
  contact_normal = glm::normalize(c_normal);
  
}

void Point_Point_Collission::resolve_collission(){};


