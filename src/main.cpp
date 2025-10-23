#include "components/examples.hh"
#include "renderer.hh"

int main () {

  /*
    NOTES

    Done  write text rendering shader with bitmap texture shader thingy

    TODO make importer more streamline

    TODO move more shit into shared_ptr from static member vars

    TODO (botched fix atm) support transparent materials (detection + rendering)

    TODO add lens simulation (renderbuffer into fsquad w/ projection shader)

    Done fix leaky animation system 
    
   */
  
  Renderer main_renderer(1920,1080);
  main_renderer.set_fps_target(200);
  main_renderer.init_scene("models/test_scene/test_scene.gltf");
  //main_renderer.init_scene("models/showroom/showroom.gltf");
  //main_renderer.add_model_to_scene("models/midnight_coup/midnight_coup.gltf");
  //main_renderer.add_model_to_scene("models/cube/cube.gltf");
  //  main_renderer.add_model_to_scene("models/levi/model.gltf");
  //main_renderer.add_model_to_player_hand("models/ikelos/ikelos.gltf");
  //  main_renderer.add_model_to_scene("models/normal_ball/normal_ball.gltf");
  //main_renderer.add_model_to_scene("models/eow/eow.gltf");
  //main_renderer.add_model_to_scene("models/lament/lament.gltf");
  //main_renderer.add_model_to_scene("models/pbr_cube/pbr_cube.gltf");
  main_renderer.add_model_to_scene("models/benz/scene.gltf");

  //    spawn_pyramid(main_renderer);
  //  spawn_point_collission(main_renderer);
  //  spawn_single_link(main_renderer);
  //  spawn_box(main_renderer);

  //spawn_noodle_loop(main_renderer);

  main_renderer.add_text_to_overlay("fps:", 10 ,40 );
  main_renderer.add_text_to_overlay("cortex - dev build", 10 ,60 );
  
  while(!main_renderer.m_input_manager->m_should_shutdown) {
    main_renderer.render_frame();
  }

  log_success("shutdown signal recieved, ended gracefully.");
  glfwTerminate();
  
}
