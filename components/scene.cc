#include "scene.hh"

void Scene::add_entity_to_scene(Entity to_add) {

  m_loaded_entities.push_back(to_add);
  
}
