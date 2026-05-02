#include "cli.hh"
#include "logging.hh"
#include <chrono>
#include <thread>

void Cortex_CLI::init_cli(std::shared_ptr<Scene> ns) {
  m_active_scene = ns;
  Logger::log_success("commandline shell initialized successfully!");
}

void Cortex_CLI::start_cli() {

  //TODO fix for should_shutdown
  while(true) {

    Logger::log_debug("processing line...");
    std::string to_process;
    std::cin >> to_process;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    }
  
}

void Cortex_CLI::execute_action() {}
void Cortex_CLI::process_line(std::string) {}
