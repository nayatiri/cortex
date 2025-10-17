#include "texture_atlas.hh"
#include "overlay_element.hh"
#include "importer.hh"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>

void Texture_Atlas::load_glyph_table(const char *to_load, const char *to_load_texture_atlas, std::atomic<unsigned int> &num_loaded_textures,
    std::vector<std::tuple<std::string, unsigned int, unsigned int>> &texture_map) {

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

    new_glyph.pos_x = glyph_column;
    new_glyph.pos_y = glyph_row;
    new_glyph.char_to_represent = str_buffer_char;
    
    glyph_table.push_back(new_glyph);
    
  }

  texture_gluint = Importer::bind_texture_to_slot(to_load_texture_atlas,num_loaded_textures,texture_map);
  
  load_str.close();
  
}; // fuck windows users xd

std::vector<float> Texture_Atlas::get_glyph_UV_sequence_for_string(std::string input) {
    std::vector<float> uv_seq;
    float cell_size = 1.0f / static_cast<float>(glyph_table_stride);

    for (char c : input) {
        std::tuple<float, float> top_left = get_matching_glyph_UV(c);
        float u0 = std::get<0>(top_left);
        float v0 = std::get<1>(top_left);
        float u1 = u0 + cell_size;
        float v1 = v0 + cell_size;

        uv_seq.insert(uv_seq.end(), {
            u0, v1,
            u0, v0,
            u1, v1,

            u0, v0,
            u1, v0,
            u1, v1 
        });
    }
    return uv_seq;
}

std::tuple<float,float>
Texture_Atlas::get_matching_glyph_UV(const char to_check) {
 
  for(Glyph g : glyph_table) {
    if(g.char_to_represent == to_check){
      float u = static_cast<float>(g.pos_x) / static_cast<float>(glyph_table_stride);
      float v = static_cast<float>(g.pos_y) / static_cast<float>(glyph_table_stride);
      return {u, v};
    }
  }

  return {-1.0,-1.0};
  
}

std::vector<float> Texture_Atlas::get_glyph_vert_cords_for_string(
    std::string input, unsigned int viewport_width,
    unsigned int viewport_height, std::shared_ptr<Overlay_Element> oe) {

  std::vector<float> vertices;

  float scaled_glyph_w = static_cast<float>(glyph_dimension_x) * oe->size;
  float scaled_glyph_h = static_cast<float>(glyph_dimension_y) * oe->size;

  float base_x_px = oe->anchor_pos_norm_x * static_cast<float>(viewport_width);
  float base_y_px = oe->anchor_pos_norm_y * static_cast<float>(viewport_height);

  for (size_t i = 0; i < input.length(); ++i) {

    float x0_px = base_x_px + i * scaled_glyph_w;
    float y0_px = base_y_px; // top
    float x1_px = x0_px + scaled_glyph_w;
    float y1_px = y0_px + scaled_glyph_h; // bottom

    float x0 = x0_px / static_cast<float>(viewport_width);
    float y0 = y0_px / static_cast<float>(viewport_height);
    float x1 = x1_px / static_cast<float>(viewport_width);
    float y1 = y1_px / static_cast<float>(viewport_height);

    vertices.push_back(x0);
    vertices.push_back(y0); // top-left
    vertices.push_back(x0);
    vertices.push_back(y1); // bottom-left
    vertices.push_back(x1);
    vertices.push_back(y0); // top-right
    
    vertices.push_back(x0);
    vertices.push_back(y1); // bottom-left
    vertices.push_back(x1);
    vertices.push_back(y1); // bottom-right
    vertices.push_back(x1);
    vertices.push_back(y0); // top-right
  }

  return vertices;

}
