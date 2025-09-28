#pragma once

#include "overlay_element.hh"
#include <memory>
#include <string>
#include <vector>

class Glyph {
public:
  //positions in texture atlas png
  unsigned int pos_x;
  unsigned int pos_y;

  char char_to_represent;
  
};

class Texture_Atlas {
public:
  std::string charset_file_path = "";

  std::vector<Glyph> glyph_table;
  unsigned int glyph_table_stride = 16;
  unsigned int glyph_dimension_x = 32;
  unsigned int glyph_dimension_y = 32;

  unsigned int texture_gluint = 0;
  
  std::tuple<float,float> get_matching_glyph_UV(const char to_check);

  void load_glyph_table(const char* filepath, const char* filepath_tex_atlas, std::atomic<unsigned int> &num_loaded_textures,
			std::vector<std::tuple<std::string, unsigned int, unsigned int>> &texture_map);

  std::vector<float> get_glyph_UV_sequence_for_string(std::string input);

  std::vector<float> get_glyph_vert_cords_for_string(std::string input, unsigned int viewport_width, unsigned int viewport_height,std::shared_ptr<Overlay_Element> oe);
  
};
