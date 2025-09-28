#pragma once
#include <string>
#include <vector>

enum E_Overlay_Type {

  E_TEXT_ELEMENT
  
};

class Overlay_Element {
public:

  Overlay_Element(std::string to_add, float anchor_x_ndc, float anchor_y_ndc);
  
  unsigned int text_vertices_vbo = 0;
  std::vector<float> text_vert_coords_screen_space;
  unsigned int text_uv_vbo = 0;
  std::vector<float> uv_coords;
  unsigned int text_vao = 0;

  bool element_needs_vbo_update = true;

  unsigned int texture_glid;
  
  E_Overlay_Type overlay_type;

  std::string text;

  float anchor_pos_norm_x = 50.0f,anchor_pos_norm_y = 50.0f;

  float size = 1.0f;
  
};
