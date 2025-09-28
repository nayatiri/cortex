#include "overlay_element.hh"

Overlay_Element::Overlay_Element(std::string to_add, float anchor_x_ndc, float anchor_y_ndc ) {

  text = to_add;

  overlay_type = E_TEXT_ELEMENT;
  
  anchor_pos_norm_x = anchor_x_ndc;
  anchor_pos_norm_y = anchor_y_ndc;
  
};
