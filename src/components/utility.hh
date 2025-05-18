#pragma once

#include "../components/logging.hh"
#include "../components/mesh.hh"
#include "../shaders/shaderclass.hh"
#include "material.hh"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// stdlib
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "../libs/tiny_gltf.h"

struct tan_bin_glob {

  std::vector<float> vert_tangents;
  std::vector<float> vert_binormals;
};

std::vector<float> calculate_vert_normals(std::vector<float> mesh_vertices) {
  std::vector<glm::vec3> vertexNormals; // buffer
  std::vector<float> mesh_normals;
  size_t numVertices = mesh_vertices.size() / 3;
  vertexNormals.resize(numVertices, glm::vec3(0.0f));
  mesh_normals.reserve(numVertices * 3);
  for (size_t i = 0; i < mesh_vertices.size(); i += 9) {

    glm::vec3 v0(mesh_vertices[i], mesh_vertices[i + 1],
                 mesh_vertices[i + 2]); // Vertex 1
    glm::vec3 v1(mesh_vertices[i + 3], mesh_vertices[i + 4],
                 mesh_vertices[i + 5]); // Vertex 2
    glm::vec3 v2(mesh_vertices[i + 6], mesh_vertices[i + 7],
                 mesh_vertices[i + 8]); // Vertex 3

    // Calculate the normal for the face
    glm::vec3 faceNormal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

    // Accumulate the face normal to each vertex's normal
    vertexNormals[i / 3] += faceNormal;     // Vertex 1
    vertexNormals[i / 3 + 1] += faceNormal; // Vertex 2
    vertexNormals[i / 3 + 2] += faceNormal; // Vertex 3
  }

  for (size_t i = 0; i < vertexNormals.size(); ++i) {
    vertexNormals[i] = glm::normalize(vertexNormals[i]);
  }

  for (const auto &normal : vertexNormals) {
    mesh_normals.push_back(normal.x);
    mesh_normals.push_back(normal.y);
    mesh_normals.push_back(normal.z);
  }

  std::cout << "done calculating " << mesh_normals.size() << " normals"
            << std::endl;
  return mesh_normals;
}

tan_bin_glob calculate_vert_tan_bin(std::vector<float> mesh_vertices,
                                    std::vector<float> mesh_normals,
                                    std::vector<float> texture_coordinates) {

  std::vector<float> vert_tangents;
  std::vector<float> vert_binormals;

  tan_bin_glob return_glob;

  vert_tangents.resize(mesh_vertices.size());
  vert_binormals.resize(mesh_vertices.size());

  for (size_t i = 0, j = 0; i < mesh_vertices.size(); i += 9, j += 6) {

    glm::vec3 v0(mesh_vertices[i], mesh_vertices[i + 1],
                 mesh_vertices[i + 2]); // Vertex 1
    glm::vec3 v1(mesh_vertices[i + 3], mesh_vertices[i + 4],
                 mesh_vertices[i + 5]); // Vertex 2
    glm::vec3 v2(mesh_vertices[i + 6], mesh_vertices[i + 7],
                 mesh_vertices[i + 8]); // Vertex 3

    glm::vec2 t0(texture_coordinates[j],
                 texture_coordinates[j + 1]); // Vertex 1
    glm::vec2 t1(texture_coordinates[j + 2],
                 texture_coordinates[j + 3]); // Vertex 2
    glm::vec2 t2(texture_coordinates[j + 4],
                 texture_coordinates[j + 5]); // Vertex 3

    glm::vec3 e1(v1 - v0);
    glm::vec3 e2(v2 - v0);

    glm::vec2 delta_uv_1(t1 - t0);
    glm::vec2 delta_uv_2(t2 - t0);

    // shoutout wikipedia
    float f =
        1 / ((delta_uv_1.x * delta_uv_2.y) - (delta_uv_1.y * delta_uv_2.x));

    glm::vec3 tangent = f * ((delta_uv_2.y * e1) - (delta_uv_1.y * e2));
    glm::vec3 bitangent = f * ((-delta_uv_1.x * e1) + (delta_uv_1.x * e2));

    for (int vert_id = 0; vert_id < 3; vert_id++) {

      vert_tangents[i + (vert_id * 3)] += tangent.x;
      vert_tangents[i + (vert_id * 3) + 1] += tangent.y;
      vert_tangents[i + (vert_id * 3) + 2] += tangent.z;

      vert_binormals[i + (vert_id * 3)] += bitangent.x;
      vert_binormals[i + (vert_id * 3) + 1] += bitangent.y;
      vert_binormals[i + (vert_id * 3) + 2] += bitangent.z;
    }
  }

  // normalize tangents
  for (int i = 0; i < vert_tangents.size(); i = i + 3) {

    glm::vec3 to_norm(vert_tangents[i], vert_tangents[i + 1],
                      vert_tangents[i + 2]);
    to_norm = glm::normalize(to_norm);
    vert_tangents[i] = to_norm.x;
    vert_tangents[i + 1] = to_norm.y;
    vert_tangents[i + 2] = to_norm.z;
  }

  // normalize binormals
  for (int i = 0; i < vert_binormals.size(); i = i + 3) {

    glm::vec3 to_norm(vert_binormals[i], vert_binormals[i + 1],
                      vert_binormals[i + 2]);
    to_norm = glm::normalize(to_norm);
    vert_binormals[i] = to_norm.x;
    vert_binormals[i + 1] = to_norm.y;
    vert_binormals[i + 2] = to_norm.z;
  }

  for (size_t i = 0; i < vert_tangents.size(); i += 3) {
    glm::vec3 normal(mesh_normals[i], mesh_normals[i + 1], mesh_normals[i + 2]);
    glm::vec3 tangent(vert_tangents[i], vert_tangents[i + 1],
                      vert_tangents[i + 2]);

    // Orthogonalize tangent
    tangent -= normal * glm::dot(normal, tangent);
    tangent = glm::normalize(tangent);

    // Recalculate bitangent
    glm::vec3 bitangent = glm::cross(normal, tangent);

    vert_tangents[i] = tangent.x;
    vert_tangents[i + 1] = tangent.y;
    vert_tangents[i + 2] = tangent.z;

    vert_binormals[i] = bitangent.x;
    vert_binormals[i + 1] = bitangent.y;
    vert_binormals[i + 2] = bitangent.z;
  }

  return_glob.vert_binormals = vert_binormals;
  return_glob.vert_tangents = vert_tangents;

  return return_glob;
}

GLuint bind_texture_to_slot(std::string to_load, unsigned int slot) {
    printf("trying to load texture into slot: %d\n", slot);
    int width, height, nrChannels;

    //    stbi_set_flip_vertically_on_load(true); // fuck u
    unsigned char *data = stbi_load(to_load.c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        std::cerr << "failed to load texture: " << to_load << std::endl;
        return 0;
    }

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    else {
        std::cerr << "unsupported number of channels: " << nrChannels << std::endl;
        stbi_image_free(data);
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    // glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texture;
}

Mesh load_primitive_mesh_from_gltf(const std::string file_path) {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;

  std::string err;
  std::string warn;

  bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file_path);

  if (!warn.empty()) {
    printf("Warn: %s\n", warn.c_str());
  }

  if (!err.empty()) {
    printf("Err: %s\n", err.c_str());
  }

  for (const auto &material : model.materials) {
    std::cout << "Material Name: " << material.name << std::endl;

    if (material.values.find("baseColorTexture") != material.values.end()) {
      int idx = material.values.at("baseColorTexture").TextureIndex();
      if (idx >= 0)
        std::cout << "Base Color Texture: " << model.textures[idx].source
                  << std::endl;
    }

    if (material.values.find("normalTexture") != material.values.end()) {
      int idx = material.values.at("normalTexture").TextureIndex();
      if (idx >= 0)
        std::cout << "Normal Texture: " << model.textures[idx].source
                  << std::endl;
    }

    if (material.values.find("metallicRoughnessTexture") !=
        material.values.end()) {
      int idx = material.values.at("metallicRoughnessTexture").TextureIndex();
      if (idx >= 0)
        std::cout << "Metallic Roughness Texture: "
                  << model.textures[idx].source << std::endl;
    }

    if (material.values.find("occlusionTexture") != material.values.end()) {
      int idx = material.values.at("occlusionTexture").TextureIndex();
      if (idx >= 0)
        std::cout << "Occlusion Texture: " << model.textures[idx].source
                  << std::endl;
    }
  }

  Shader shader_to_use("src/shaders/shader_src/phong.vert",
                       "src/shaders/shader_src/phong.frag");
  Material mat_to_use(E_FACE, shader_to_use);
  Mesh primitive_mesh(mat_to_use);

  for (auto &found_mesh : model.meshes) {
    std::cout << "Found mesh, deserializing..." << std::endl;

    for (auto &primitive : found_mesh.primitives) {
      std::cout << "Found primitive, deserializing..." << std::endl;

      const auto &posAccessor =
          model.accessors[primitive.attributes["POSITION"]];
      const auto &posBufferView = model.bufferViews[posAccessor.bufferView];
      const auto &posBuffer = model.buffers[posBufferView.buffer];
      const float *positions = reinterpret_cast<const float *>(
          &posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

      const float *normals = nullptr;
      if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
        const auto &normalAccessor =
            model.accessors[primitive.attributes["NORMAL"]];
        const auto &normalBufferView =
            model.bufferViews[normalAccessor.bufferView];
        const auto &normalBuffer = model.buffers[normalBufferView.buffer];
        normals = reinterpret_cast<const float *>(
            &normalBuffer.data[normalBufferView.byteOffset +
                               normalAccessor.byteOffset]);
      }

      std::vector<float> final_vertices;
      std::vector<float> final_normals;

      if (primitive.indices >= 0) {
        const auto &indexAccessor = model.accessors[primitive.indices];
        const auto &indexBufferView =
            model.bufferViews[indexAccessor.bufferView];
        const auto &indexBuffer = model.buffers[indexBufferView.buffer];
        const void *indexData =
            &indexBuffer
                 .data[indexBufferView.byteOffset + indexAccessor.byteOffset];

        for (size_t i = 0; i < indexAccessor.count; ++i) {
          uint32_t index = 0;

          // ich krig nen knax
          switch (indexAccessor.componentType) {
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            index = reinterpret_cast<const uint8_t *>(indexData)[i];
            break;
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            index = reinterpret_cast<const uint16_t *>(indexData)[i];
            break;
          case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            index = reinterpret_cast<const uint32_t *>(indexData)[i];
            break;
          default:
            std::cerr << "Unsupported index type in GLTF" << std::endl;
            return primitive_mesh;
          }

          final_vertices.push_back(positions[index * 3 + 0]);
          final_vertices.push_back(positions[index * 3 + 1]);
          final_vertices.push_back(positions[index * 3 + 2]);

          if (normals) {
            final_normals.push_back(normals[index * 3 + 0]);
            final_normals.push_back(normals[index * 3 + 1]);
            final_normals.push_back(normals[index * 3 + 2]);
          }
        }

      } else {

        final_vertices.assign(positions, positions + posAccessor.count * 3);
        if (normals) {
          const auto &normalAccessor =
              model.accessors[primitive.attributes["NORMAL"]];
          final_normals.assign(normals, normals + normalAccessor.count * 3);
        }
      }

      primitive_mesh.m_vertices_array = std::move(final_vertices);
      primitive_mesh.m_normals_array = std::move(final_normals);
    }
  }

  log_success(
      "gltf file successfully loaded with de-indexed data, returning mesh!");
  return primitive_mesh;
}

glm::mat4 hipster_rotation_bullshit(float m_lastFrame) {
  float t = m_lastFrame;

  glm::vec3 axis =
      glm::normalize(glm::vec3(sin(t * 0.5f), cos(t * 0.3f), sin(t * 0.7f)));

  float angle = sin(t * 0.8f) * glm::radians(90.0f);

  return glm::rotate(glm::mat4(1.0f), angle, axis);
}

void check_gl_error(const char *context = "") {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    const char *errorStr = "Unknown error";
    switch (err) {
    case GL_INVALID_ENUM:
      errorStr = "GL_INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      errorStr = "GL_INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      errorStr = "GL_INVALID_OPERATION";
      break;
    case GL_OUT_OF_MEMORY:
      errorStr = "GL_OUT_OF_MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      errorStr = "GL_INVALID_FRAMEBUFFER_OPERATION";
      break;
    }

    if (context && *context)
      printf("OpenGL Error [%s]: %s (0x%X)\n", context, errorStr, err);
    else
      printf("OpenGL Error: %s (0x%X)\n", errorStr, err);
  }
}

std::vector<Mesh> load_all_meshes_from_gltf(const std::string &file_path, std::atomic<unsigned int>& num_loaded_textures) {
  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err, warn;

  log_success("importing a gltf file... loading ascii file...");
  bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, file_path);

  if (!warn.empty())
    printf("Warn: %s\n", warn.c_str());
  if (!err.empty())
    printf("Err: %s\n", err.c_str());

  std::vector<Mesh> meshes;

  auto get_index = [&](const tinygltf::Primitive &primitive,
                       int idx) -> uint32_t {
    const auto &indexAccessor = model.accessors[primitive.indices];
    const auto &indexView = model.bufferViews[indexAccessor.bufferView];
    const auto &indexBuffer = model.buffers[indexView.buffer];
    const uint8_t *base = indexBuffer.data.data() + indexView.byteOffset +
                          indexAccessor.byteOffset;

    switch (indexAccessor.componentType) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      return reinterpret_cast<const uint8_t *>(base)[idx];
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      return reinterpret_cast<const uint16_t *>(base)[idx];
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      return reinterpret_cast<const uint32_t *>(base)[idx];
    default:
      throw std::runtime_error("Unsupported index type");
    }
  };

  log_debug("starting to load gltf node tree...");

  std::function<void(int, glm::mat4)> process_node;
  process_node = [&](int node_idx, glm::mat4 parent_transform) {
    const auto &node = model.nodes[node_idx];
    glm::mat4 node_transform = glm::mat4(1.0f);

    if (node.matrix.size() == 16)
      node_transform = glm::make_mat4(node.matrix.data());
    else {
      if (node.translation.size() == 3)
        node_transform = glm::translate(
            node_transform, glm::vec3(node.translation[0], node.translation[1],
                                      node.translation[2]));
      if (node.rotation.size() == 4)
        node_transform *=
            glm::mat4_cast(glm::quat(node.rotation[3], node.rotation[0],
                                     node.rotation[1], node.rotation[2]));
      if (node.scale.size() == 3)
        node_transform =
            glm::scale(node_transform,
                       glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
    }

    glm::mat4 global_transform = parent_transform * node_transform;

    if (node.mesh >= 0) {
      const auto &found_mesh = model.meshes[node.mesh];

      for (const auto &primitive : found_mesh.primitives) {

        log_debug("importing primitive from node tree...");

        const auto &posAccessor =
            model.accessors[primitive.attributes.at("POSITION")];
        const auto &posBufferView = model.bufferViews[posAccessor.bufferView];
        const auto &posBuffer = model.buffers[posBufferView.buffer];
        const float *positions = reinterpret_cast<const float *>(
            &posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

        const float *normals = nullptr;
        if (primitive.attributes.count("NORMAL")) {
          const auto &accessor =
              model.accessors[primitive.attributes.at("NORMAL")];
          const auto &view = model.bufferViews[accessor.bufferView];
          normals = reinterpret_cast<const float *>(
              &model.buffers[view.buffer]
                   .data[view.byteOffset + accessor.byteOffset]);
        }

        const float *tangents = nullptr;
        if (primitive.attributes.count("TANGENT")) {
          const auto &accessor =
              model.accessors[primitive.attributes.at("TANGENT")];
          const auto &view = model.bufferViews[accessor.bufferView];
          tangents = reinterpret_cast<const float *>(
              &model.buffers[view.buffer]
                   .data[view.byteOffset + accessor.byteOffset]);
        }

        const float *texcoords = nullptr;
        if (primitive.attributes.count("TEXCOORD_0")) {
          const auto &accessor =
              model.accessors[primitive.attributes.at("TEXCOORD_0")];
          const auto &view = model.bufferViews[accessor.bufferView];
          texcoords = reinterpret_cast<const float *>(
              &model.buffers[view.buffer]
                   .data[view.byteOffset + accessor.byteOffset]);
        }

        log_debug("checking material for textures etc");

        const tinygltf::Material &material =
            model.materials[primitive.material];

        uint8_t shader_type_carry = 0;

	std::string texture_path_of_model;

        auto it = material.values.find("baseColorTexture");
        if (it != material.values.end() && it->second.TextureIndex() >= 0) {
          const tinygltf::Texture &texture =
              model.textures[it->second.TextureIndex()];
          const tinygltf::Image &image = model.images[texture.source];
          std::cout << "Texture path: " << image.uri << std::endl;
	  texture_path_of_model = image.uri;
          shader_type_carry = 2;
        } else {

          log_error("no materials in mesh! using phong shaders as a fallback.");
          shader_type_carry = 1;
        }

        // END TMP

        std::vector<float> final_vertices, final_normals, final_tangents,
            final_bitangents, final_texcoords;

        size_t vertex_count = posAccessor.count;
        if (primitive.indices >= 0) {
          const auto &indexAccessor = model.accessors[primitive.indices];
          for (size_t i = 0; i < indexAccessor.count; ++i) {
            uint32_t idx = get_index(primitive, i);

            final_vertices.insert(final_vertices.end(), &positions[idx * 3],
                                  &positions[idx * 3 + 3]);
            if (normals)
              final_normals.insert(final_normals.end(), &normals[idx * 3],
                                   &normals[idx * 3 + 3]);
            if (tangents)
              final_tangents.insert(final_tangents.end(), &tangents[idx * 4],
                                    &tangents[idx * 4 + 3]);
            if (texcoords)
              final_texcoords.insert(final_texcoords.end(), &texcoords[idx * 2],
                                     &texcoords[idx * 2 + 2]);
          }
        } else {
          for (size_t i = 0; i < vertex_count; ++i) {

            final_vertices.insert(final_vertices.end(), &positions[i * 3],
                                  &positions[i * 3 + 3]);
            if (normals)
              final_normals.insert(final_normals.end(), &normals[i * 3],
                                   &normals[i * 3 + 3]);
            if (tangents)
              final_tangents.insert(final_tangents.end(), &tangents[i * 4],
                                    &tangents[i * 4 + 3]);
            if (texcoords)
              final_texcoords.insert(final_texcoords.end(), &texcoords[i * 2],
                                     &texcoords[i * 2 + 2]);
          }
        }

        if (!final_tangents.empty() && !final_normals.empty()) {
          for (size_t i = 0; i < final_normals.size(); i += 3) {
            glm::vec3 N(final_normals[i], final_normals[i + 1],
                        final_normals[i + 2]);
            glm::vec3 T(final_tangents[i], final_tangents[i + 1],
                        final_tangents[i + 2]);
            glm::vec3 B = glm::normalize(glm::cross(N, T));
            final_bitangents.push_back(B.x);
            final_bitangents.push_back(B.y);
            final_bitangents.push_back(B.z);
          }
        }

        log_success("done importing models, loading shaders...");

        if (shader_type_carry == 2) {
          // use texture shading
          Shader shader_to_use("src/shaders/shader_src/pbr.vert",
                               "src/shaders/shader_src/pbr.frag");
          Material mat_to_use(E_FACE, shader_to_use);

          Mesh primitive_mesh(mat_to_use);
          primitive_mesh.m_vertices_array = std::move(final_vertices);
          primitive_mesh.m_normals_array = std::move(final_normals);
          primitive_mesh.m_tangents_array = std::move(final_tangents);
          primitive_mesh.m_binormals_array = std::move(final_bitangents);
          primitive_mesh.m_tex_coords_array = std::move(final_texcoords);
          primitive_mesh.m_model_matrix = global_transform;

	  //bind tex to num_loaded_tex and increment.
	  std::filesystem::path cwd = std::filesystem::current_path();
	  std::cout << "Current working directory: " << cwd.string() << std::endl;

	  // this is garbage hacky shit again TODO: clean shit up lol
	  std::filesystem::path model_path = file_path;
	  std::filesystem::path full_tex_path = cwd / model_path.parent_path() / texture_path_of_model;
	  std::string final_path = full_tex_path.lexically_normal().string();
	  
	  primitive_mesh.m_material.bound_texture_id = bind_texture_to_slot(full_tex_path,num_loaded_textures.load());
	  primitive_mesh.m_material.m_material_type = E_PBR_TEX;
          num_loaded_textures.fetch_add(1);

          meshes.push_back(std::move(primitive_mesh));

	  log_success("yay pbr mesh or so");
	  
        } else {
          // use phong shading (fallback)
          Shader shader_to_use("src/shaders/shader_src/phong.vert",
                               "src/shaders/shader_src/phong.frag");
          Material mat_to_use(E_FACE, shader_to_use);

          Mesh primitive_mesh(mat_to_use);
          primitive_mesh.m_vertices_array = std::move(final_vertices);
          primitive_mesh.m_normals_array = std::move(final_normals);
          primitive_mesh.m_tangents_array = std::move(final_tangents);
          primitive_mesh.m_binormals_array = std::move(final_bitangents);
          primitive_mesh.m_tex_coords_array = std::move(final_texcoords);
          primitive_mesh.m_model_matrix = global_transform;
	  
	  primitive_mesh.m_material.m_material_type = E_PHONG;

          meshes.push_back(std::move(primitive_mesh));
	  
	  log_success("shit phong mesh detected");
        }
      }
    }

    for (int child : node.children) {
      process_node(child, global_transform);
    }
  };

  int scene_index = model.defaultScene >= 0 ? model.defaultScene : 0;
  const auto &scene = model.scenes[scene_index];
  glm::mat4 identity = glm::mat4(1.0f);

  for (int node_idx : scene.nodes) {
    process_node(node_idx, identity);
  }

  log_success("GLTF scene fully loaded with multiple meshes!");

  for (auto mesh : meshes) {

    std::cout << "mesh in meshes with n vertices: "
              << mesh.m_vertices_array.size() << std::endl;
  }

  return meshes;
}
