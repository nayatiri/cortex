#include "texture_atlas.hh"
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

void Texture_Atlas::load_glyph_table(const char *to_load) {

  std::ifstream load_str(to_load);
  std::string str_buffer;
  
  while (std::getline(load_str,str_buffer)) {

    Glyph new_glyph;

    unsigned int id_end_index = str_buffer.find(' ');
    std::string glyph_id = str_buffer.substr(0, id_end_index);
    char str_buffer_char = str_buffer[str_buffer.length()-1]; 

    unsigned int glyph_row = std::floor(std::stoi(glyph_id)/glyph_table_stride);

    float rest = (std::stoi(glyph_id)/(float)glyph_table_stride) - glyph_row;

    unsigned int glyph_column = std::floor(rest * glyph_table_stride);
    
    std::cout << "string buffer: " << str_buffer << "loaded into row:" << glyph_row << " , column: " << glyph_column << " | representing char: " << str_buffer_char << std::endl;

    new_glyph.pos_x = glyph_column;
    new_glyph.pos_y = glyph_row;
    new_glyph.char_to_represent = str_buffer_char;
    
    glyph_table.push_back(new_glyph);
    
  }

  load_str.close();

}; // fuck windows users xd

