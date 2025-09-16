#pragma once

#include "../renderer.hh"

void spawn_pyramid(Renderer& main_renderer) {

  //bottom points
  main_renderer.add_point_to_scene(1,5,0);
  main_renderer.add_point_to_scene(0,5,0);
  main_renderer.add_point_to_scene(0,5,1);
  //top point
  main_renderer.add_point_to_scene(0,7,0);
  
  //bottom triangle constraints
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[0],main_renderer.m_active_scene->m_loaded_points[1], 50.0f, 4.0f);
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[1],main_renderer.m_active_scene->m_loaded_points[2], 50.0f, 4.0f);
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[2],main_renderer.m_active_scene->m_loaded_points[0], 50.0f, 4.0f);
  //side tiangle constraints
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[0],main_renderer.m_active_scene->m_loaded_points[3], 50.0f, 4.0f);
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[1],main_renderer.m_active_scene->m_loaded_points[3], 50.0f, 4.0f);
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[2],main_renderer.m_active_scene->m_loaded_points[3], 50.0f, 4.0f);

  main_renderer.create_fixed_constraint(main_renderer.m_active_scene->m_loaded_points[3], true);
  
};

void spawn_point_collission(Renderer& main_renderer) {
  
  main_renderer.add_point_to_scene(3,5,0);
  main_renderer.add_point_to_scene(3,0,0);

  main_renderer.m_active_scene->m_loaded_points[0]->phys_props.velocity = {0,0,0};
  main_renderer.m_active_scene->m_loaded_points[0]->phys_props.set_mass(35.0f);
  main_renderer.m_active_scene->m_loaded_points[1]->phys_props.velocity = {0,0,0};
  main_renderer.m_active_scene->m_loaded_points[1]->phys_props.set_mass(10.0f);
}

void spawn_single_link(Renderer& main_renderer) {

  //bottom points
  main_renderer.add_point_to_scene(4,5,0);
  main_renderer.add_point_to_scene(0,5,0);
  
  //bottom triangle constraints
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[0],main_renderer.m_active_scene->m_loaded_points[1], 10.0f, 4.0f);

  main_renderer.create_fixed_constraint(main_renderer.m_active_scene->m_loaded_points[0], true);
  
};
