#pragma once

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

  Glyph get_matching_glyph(const char to_check);

  void load_glyph_table(const char* filepath);

  std::tuple<unsigned int, unsigned int> get_glyph_sequence_for_string(const char* filepath);
  
};
