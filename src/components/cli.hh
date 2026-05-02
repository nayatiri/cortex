#pragma once

#include <memory>
#include <string>
#include "scene.hh"

class Cortex_CLI {
public:
  std::shared_ptr<Scene> m_active_scene;

  void init_cli(std::shared_ptr<Scene>);
  void start_cli();
  
private:
  void execute_action();
  void process_line(std::string);
  
};
