#include "renderer.hh"
#include <cmath>

//defines
#define window_width 1920
#define window_height 1080
int main () {

  Renderer main_renderer(1920,1080);
  main_renderer.init_scene("models/test_scene/test_scene.gltf");
  float height = 1.0f;
  for(float i = 0; i<10; i+=0.1) {
    main_renderer.add_point_to_scene(sin(i),height,cos(i));
    height +=0.1;
  }
  
  while(!main_renderer.m_input_manager->m_should_shutdown) {
    main_renderer.render_frame();
  }

  log_success("shutdown signal recieved, ended gracefully.");
  glfwTerminate();
  
}
