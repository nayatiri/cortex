#include "components/examples.hh"
#include "renderer.hh"

//defines
#define window_width 1920
#define window_height 1080
int main () {

  Renderer main_renderer(1920,1080);
  main_renderer.init_scene("models/test_scene/test_scene.gltf");
  
  //  spawn_pyramid(main_renderer);
  //  spawn_point_collission(main_renderer);
  spawn_single_link(main_renderer);
  
  while(!main_renderer.m_input_manager->m_should_shutdown) {
    main_renderer.render_frame();
  }

  log_success("shutdown signal recieved, ended gracefully.");
  glfwTerminate();
  
}
