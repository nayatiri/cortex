#include "components/examples.hh"
#include "renderer.hh"

//defines
#define window_width 1920
#define window_height 1080
int main () {


  /*
    NOTES

    TODO write text rendering shader with bitmap texture shader thingy

    TODO make importer more streamline

    TODO move more shit into shared_ptr from static member vars

    TODO support transparent materials (detection + rendering)

    TODO add lens simulation (renderbuffer into fsquad w/ projection shader)

    TODO fix leaky animation system 
    
   */
  
  Renderer main_renderer(1920,1080);
  main_renderer.init_scene("models/test_scene/test_scene.gltf");
  main_renderer.init_scene("models/showroom/showroom.gltf");
  //main_renderer.add_model_to_scene("models/cube/cube.gltf");
  //main_renderer.add_model_to_scene("models/pbr_cube/pbr_cube.gltf");
  //main_renderer.add_model_to_scene("models/benz/scene.gltf");

  
  //  spawn_pyramid(main_renderer);
  //  spawn_point_collission(main_renderer);
  //  spawn_single_link(main_renderer);
  //  spawn_box(main_renderer);

  
  while(!main_renderer.m_input_manager->m_should_shutdown) {
    main_renderer.render_frame();
  }

  log_success("shutdown signal recieved, ended gracefully.");
  glfwTerminate();
  
}
