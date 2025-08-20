#include "physicsmanager.hh"
#include <array>
#include <memory>

Physics_Manager::Physics_Manager(std::shared_ptr<Scene> set_scene) {
  m_active_scene = set_scene;
};

Mesh Physics_Manager::create_collision_box_mesh(const AABB &box) {
  std::array<glm::vec3, 8> corners = {
      glm::vec3(box.min.x, box.min.y, box.min.z),
      glm::vec3(box.max.x, box.min.y, box.min.z),
      glm::vec3(box.max.x, box.max.y, box.min.z),
      glm::vec3(box.min.x, box.max.y, box.min.z),
      glm::vec3(box.min.x, box.min.y, box.max.z),
      glm::vec3(box.max.x, box.min.y, box.max.z),
      glm::vec3(box.max.x, box.max.y, box.max.z),
      glm::vec3(box.min.x, box.max.y, box.max.z)};

  std::array<int, 24> edge_indices = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6,
                                      6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

  std::vector<float> vertices;
  vertices.reserve(24 * 3);

  for (int i = 0; i < 24; i += 2) {
    glm::vec3 v1 = corners[edge_indices[i]];
    glm::vec3 v2 = corners[edge_indices[i + 1]];

    vertices.insert(vertices.end(), {v1.x, v1.y, v1.z});
    vertices.insert(vertices.end(), {v2.x, v2.y, v2.z});
  }

  Shader wireframe_shader("src/shaders/shader_src/wireframe.vert",
                          "src/shaders/shader_src/wireframe.frag");
  Material material(E_PHONG, wireframe_shader);

  Mesh mesh(material);
  mesh.m_vertices_array = std::move(vertices);
  mesh.m_render_mode = E_WIREFRAME;
  mesh.m_type = E_COL_BOX;

  return mesh;
}

AABB Physics_Manager::compute_world_space_aabb(const Mesh &mesh,
                                               const glm::mat4 &transform) {
  AABB bbox{{10000.0f, 10000.0f, 10000.0f}, {-10000.0f, -10000.0f, -10000.0f}};

  const glm::mat4 mesh_transform = transform * mesh.m_model_matrix;

  for (size_t i = 0; i + 2 < mesh.m_vertices_array.size(); i += 3) {
    glm::vec4 vertex{mesh.m_vertices_array[i], mesh.m_vertices_array[i + 1],
                     mesh.m_vertices_array[i + 2], 1.0f};

    vertex = mesh_transform * vertex;

    bbox.min.x = std::min(bbox.min.x, vertex.x);
    bbox.min.y = std::min(bbox.min.y, vertex.y);
    bbox.min.z = std::min(bbox.min.z, vertex.z);

    bbox.max.x = std::max(bbox.max.x, vertex.x);
    bbox.max.y = std::max(bbox.max.y, vertex.y);
    bbox.max.z = std::max(bbox.max.z, vertex.z);
  }

  return bbox;
}

void Physics_Manager::calculate_phys_boxes() {
  std::vector<Mesh> mesh_buffer;

  std::cout << "Scene contains entities: "
            << m_active_scene->m_loaded_entities.size() << std::endl;

  for (Entity &entity : m_active_scene->m_loaded_entities) {
    glm::mat4 entity_transform = entity.m_model_matrix;
    std::cout << "Entity contains meshes: " << entity.m_mesh.size()
              << std::endl;

    for (Mesh &mesh : entity.m_mesh) {
      if (mesh.m_type != E_MESH) {
        log_error("Mesh does'nt seem to be a Mesh lol. skipping.");
        continue;
      };

      if (mesh.m_vertices_array.size() < 3) {
        log_error("Attempted to create hitbox for mesh with insufficient "
                  "vertices (less than 3). Skipping.");
        continue;
      }

      AABB bbox = compute_world_space_aabb(mesh, entity_transform);
      Mesh col_box = create_collision_box_mesh(bbox);
      mesh_buffer.push_back(col_box);

      log_success("Calculated hitbox for mesh!");
    }
  }

  for(Mesh m : mesh_buffer) {
    m_active_scene->m_loaded_entities[0].m_mesh.push_back(m);
  }

  m_active_scene->m_scene_vbos_need_refresh = true;
  
}

void Physics_Manager::handle_scene_physics() {

  if (!m_phys_boxes_initialized) {
    calculate_phys_boxes();
    m_phys_boxes_initialized = true;
  }
}
