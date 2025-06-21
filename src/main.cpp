#include "renderer.hh"

//defines
#define window_width 1920
#define window_height 1080
int main () {

  Renderer main_renderer(1920,1080);
  //  main_renderer.init_scene();

  while(!main_renderer.m_should_shutdown) {

    main_renderer.render_frame();
    
  }

  log_success("shutdown signal recieved, ended gracefully.");
  glfwTerminate();
  
}
