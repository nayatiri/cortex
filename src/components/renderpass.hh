#pragma once

class Renderpass_Object {
public:
  virtual ~Renderpass_Object() = default;
  virtual void setup_vbos();
  virtual void render_frame();
};

class Renderpass_Depth: public Renderpass_Object {

  void setup_vbos();
  void render_frame();
  
};

class Renderpass_Color:public Renderpass_Object {

  void setup_vbos();
  void render_frame();
  
};
