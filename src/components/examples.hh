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
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[0],main_renderer.m_active_scene->m_loaded_points[1], 100.0f, 4.0f);
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[1],main_renderer.m_active_scene->m_loaded_points[2], 100.0f, 4.0f);
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[2],main_renderer.m_active_scene->m_loaded_points[0], 100.0f, 4.0f);
  //side tiangle constraints
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[0],main_renderer.m_active_scene->m_loaded_points[3], 100.0f, 4.0f);
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[1],main_renderer.m_active_scene->m_loaded_points[3], 100.0f, 4.0f);
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[2],main_renderer.m_active_scene->m_loaded_points[3], 100.0f, 4.0f);

  main_renderer.create_fixed_constraint(main_renderer.m_active_scene->m_loaded_points[3], true);
};

void spawn_noodle_loop(Renderer& main_renderer) {
    const int num_points = 50;
    const float radius = 3.0f;        
    const float rest_length = 0.4f;   
    const float stiffness = 120.0f;   

    for (int i = 0; i < num_points; ++i) {
        float angle = 2.0f * M_PI * static_cast<float>(i) / num_points;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        main_renderer.add_point_to_scene(x, 5.0f, z);
    }

    auto& points = main_renderer.m_active_scene->m_loaded_points;

    for (int i = 0; i < num_points; ++i) {
        int next = (i + 1) % num_points; 
        main_renderer.create_spring_constraint(points[i], points[next], stiffness, rest_length);
    }

}

void spawn_point_collission(Renderer& main_renderer) {
  
  main_renderer.add_point_to_scene(0,5,0);
  main_renderer.add_point_to_scene(0,3,0);

  main_renderer.m_active_scene->m_loaded_points[0]->phys_props.velocity = {0,5.0f,0};
  main_renderer.m_active_scene->m_loaded_points[1]->phys_props.velocity = {0,8.0f,0};
}

void spawn_single_link(Renderer& main_renderer) {

  //bottom points
  main_renderer.add_point_to_scene(20,5,0);
  main_renderer.add_point_to_scene(0,5,0);
  
  //bottom triangle constraints
  main_renderer.create_spring_constraint(main_renderer.m_active_scene->m_loaded_points[0],main_renderer.m_active_scene->m_loaded_points[1], 200.0f, 20.0f);

  main_renderer.create_fixed_constraint(main_renderer.m_active_scene->m_loaded_points[0], true);
  
};

//shoutout ai xd
void spawn_box(Renderer& main_renderer) {

  // Define 8 corners of a unit box (you can scale/translate as needed)
  // Order: Bottom layer (counter-clockwise), then top layer above them
  main_renderer.add_point_to_scene(0,0,0); // P0
  main_renderer.add_point_to_scene(1,0,0); // P1
  main_renderer.add_point_to_scene(1,0,1); // P2
  main_renderer.add_point_to_scene(0,0,1); // P3

  main_renderer.add_point_to_scene(0,1,0); // P4
  main_renderer.add_point_to_scene(1,1,0); // P5
  main_renderer.add_point_to_scene(1,1,1); // P6
  main_renderer.add_point_to_scene(0,1,1); // P7

  auto& points = main_renderer.m_active_scene->m_loaded_points;

  float stiffness = 80.0f;
  float edge_length = 5.0f;
  float diag_face_length = edge_length * sqrtf(2.0f);     // e.g., across square face
  float diag_space_length = edge_length * sqrtf(3.0f);   // optional: space diagonals for extra rigidity

  // ========== EDGES ========== //
  // Bottom face
  main_renderer.create_spring_constraint(points[0], points[1], stiffness, edge_length);
  main_renderer.create_spring_constraint(points[1], points[2], stiffness, edge_length);
  main_renderer.create_spring_constraint(points[2], points[3], stiffness, edge_length);
  main_renderer.create_spring_constraint(points[3], points[0], stiffness, edge_length);

  // Top face
  main_renderer.create_spring_constraint(points[4], points[5], stiffness, edge_length);
  main_renderer.create_spring_constraint(points[5], points[6], stiffness, edge_length);
  main_renderer.create_spring_constraint(points[6], points[7], stiffness, edge_length);
  main_renderer.create_spring_constraint(points[7], points[4], stiffness, edge_length);

  // Vertical edges
  main_renderer.create_spring_constraint(points[0], points[4], stiffness, edge_length);
  main_renderer.create_spring_constraint(points[1], points[5], stiffness, edge_length);
  main_renderer.create_spring_constraint(points[2], points[6], stiffness, edge_length);
  main_renderer.create_spring_constraint(points[3], points[7], stiffness, edge_length);

  // ========== FACE DIAGONALS (prevents collapse) ========== //
  // Bottom face diagonals
  main_renderer.create_spring_constraint(points[0], points[2], stiffness, diag_face_length);
  main_renderer.create_spring_constraint(points[1], points[3], stiffness, diag_face_length);

  // Top face diagonals
  main_renderer.create_spring_constraint(points[4], points[6], stiffness, diag_face_length);
  main_renderer.create_spring_constraint(points[5], points[7], stiffness, diag_face_length);

  // Front face (Z = 0): P0-P1-P5-P4
  main_renderer.create_spring_constraint(points[0], points[5], stiffness, diag_face_length);
  main_renderer.create_spring_constraint(points[1], points[4], stiffness, diag_face_length);

  // Back face (Z = 1): P2-P3-P7-P6
  main_renderer.create_spring_constraint(points[2], points[7], stiffness, diag_face_length);
  main_renderer.create_spring_constraint(points[3], points[6], stiffness, diag_face_length);

  // Left face (X = 0): P0-P3-P7-P4
  main_renderer.create_spring_constraint(points[0], points[7], stiffness, diag_face_length);
  main_renderer.create_spring_constraint(points[3], points[4], stiffness, diag_face_length);

  // Right face (X = 1): P1-P2-P6-P5
  main_renderer.create_spring_constraint(points[1], points[6], stiffness, diag_face_length);
  main_renderer.create_spring_constraint(points[2], points[5], stiffness, diag_face_length);

  // ========== Optional: Space diagonals for maximum rigidity ========== //
  // These are not strictly necessary if all faces are triangulated, but help with global stability

  main_renderer.create_spring_constraint(points[0], points[6], stiffness, diag_space_length);
  main_renderer.create_spring_constraint(points[1], points[7], stiffness, diag_space_length);
  main_renderer.create_spring_constraint(points[2], points[4], stiffness, diag_space_length);
  main_renderer.create_spring_constraint(points[3], points[5], stiffness, diag_space_length);

  // Fix one point to prevent drifting (e.g., bottom corner)
  main_renderer.create_fixed_constraint(points[0], true);
}
