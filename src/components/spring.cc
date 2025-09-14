#include "spring.hh"

Spring::Spring(std::shared_ptr<Point> from, std::shared_ptr<Point> to,
               float new_strength)
  : link_A(from),
    link_B(to),
    strength(new_strength) {}
