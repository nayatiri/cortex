#include "renderer.hh"

//defines
#define window_width 1920
#define window_height 1080
int main () {

  Renderer main_renderer(1920,1080);
  main_renderer.init_scene("models/test_scene/test_scene.gltf");
  main_renderer.add_point_to_scene(1,8,0);
  main_renderer.add_point_to_scene(0,5,0);
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[1],main_renderer.m_active_scene->m_loaded_points[0], 20.0f, 4.0f);
  
  while(!main_renderer.m_input_manager->m_should_shutdown) {
    main_renderer.render_frame();
  }

  log_success("shutdown signal recieved, ended gracefully.");
  glfwTerminate();
  
}
