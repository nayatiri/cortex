#include "renderpass.hh"
#include  "iostream"

void Renderpass_Color::setup_vbos() {
  printf("setup vbos called from color!\n");
};
void Renderpass_Color::render_frame() {
  printf("render frame called from color!\n");
};


void Renderpass_Depth::setup_vbos() {
  printf("setup vbos called from depth!\n");
};
void Renderpass_Depth::render_frame() {
  printf("render frame called from depth!\n");
};

// only define needed.
void Renderpass_Object::setup_vbos() {};
void Renderpass_Object::render_frame() {};
