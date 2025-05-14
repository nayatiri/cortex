
#include <iostream>
#include <cstdint>
#include <vector>

struct bitmap_char {

  uint32_t s_char_width;
  uint32_t s_char_height;

  std::vector<uint8_t> s_bit_table;
  
};

struct bitmap_font {

  uint32_t s_num_chars;

  std::vector<bitmap_char> s_font_char_map_data;
  std::vector<char> s_font_char_map_symbols;
  
};
