#include "constraint.hh"

Fix_length_constraint::Fix_length_constraint(std::shared_ptr<Point> from,
                                             std::shared_ptr<Point> to,
                                             float set_distance) : point_a(from),point_b(to),distance(set_distance) {};
