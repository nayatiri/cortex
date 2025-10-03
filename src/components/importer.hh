#pragma once

#include <vector>
#include <atomic>

#include "mesh.hh"

namespace Importer {
  
  struct tan_bin_glob {  
    std::vector<float> vert_tangents;
    std::vector<float> vert_binormals;
  };
  
  GLuint bind_texture_to_slot(std::string to_load, unsigned int slot,
			      std::vector<std::tuple<std::string, unsigned int, GLuint>> &texture_map);
  
  std::vector<std::shared_ptr<Mesh>> load_all_meshes_from_gltf(const std::string &file_path,
					      std::atomic<unsigned int> &num_loaded_textures,
							       std::vector<std::tuple<std::string, unsigned int, GLuint>> &texture_map);

  std::vector<float> calculate_vert_normals(std::vector<float> mesh_vertices);

  std::vector<Mesh> load_all_meshes_from_gltf_temp(const std::string &file_path,
					      std::atomic<unsigned int> &num_loaded_textures,
						   std::vector<std::tuple<std::string, unsigned int, GLuint>> &texture_map,
						   bool flip_normals);
  
  Importer::tan_bin_glob calculate_vert_tan_bin(std::vector<float> mesh_vertices,
						std::vector<float> mesh_normals,
						std::vector<float> texture_coordinates);
  
};
