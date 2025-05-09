#pragma once

#include "../components/logging.hh"
#include "../components/mesh.hh"
#include "../shaders/shader.hh"
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

// Tex to slot
GLuint bind_texture_to_slot(std::string to_load, unsigned int slot) {
  printf("trying to load texture into slot :%d\n", slot);
  int width, height, nrChannels;
  unsigned char *data =
      stbi_load(to_load.c_str(), &width, &height, &nrChannels, 0);

  unsigned int texture;
  glGenTextures(1, &texture);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, texture);

  if (data) {
    printf("deserialized image successfully.\n");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    //    glGenerateMipmap(GL_TEXTURE_2D); //later
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data);
  glBindTexture(GL_TEXTURE_2D, texture);

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

  // loading material data
  for (const auto &material : model.materials) {
    std::cout << "Material Name: " << material.name << std::endl;
    // Base color texture
    if (material.values.find("baseColorTexture") != material.values.end()) {
      const auto &baseColorTexture =
          material.values.at("baseColorTexture").TextureIndex();
      if (baseColorTexture >= 0) {
        const auto &texture = model.textures[baseColorTexture];
        std::cout << "Base Color Texture: " << texture.source << std::endl;
      }
    }

    // Normal texture
    if (material.values.find("normalTexture") != material.values.end()) {
      const auto &normalTexture =
          material.values.at("normalTexture").TextureIndex();
      if (normalTexture >= 0) {
        const auto &texture = model.textures[normalTexture];
        std::cout << "Normal Texture: " << texture.source << std::endl;
      }
    }

    // Metallic roughness texture
    if (material.values.find("metallicRoughnessTexture") !=
        material.values.end()) {
      const auto &metallicRoughnessTexture =
          material.values.at("metallicRoughnessTexture").TextureIndex();
      if (metallicRoughnessTexture >= 0) {
        const auto &texture = model.textures[metallicRoughnessTexture];
        std::cout << "Metallic Roughness Texture: " << texture.source
                  << std::endl;
      }
    }

    // Occlusion texture
    if (material.values.find("occlusionTexture") != material.values.end()) {
      const auto &occlusionTexture =
          material.values.at("occlusionTexture").TextureIndex();
      if (occlusionTexture >= 0) {
        const auto &texture = model.textures[occlusionTexture];
        std::cout << "Occlusion Texture: " << texture.source << std::endl;
      }
    }

    // Additional properties can be accessed similarly
  }

  // loading mesh data
  // TMP load flat shading
  Shader shader_to_use("src/shaders/shader_src/flat.vert",
                       "src/shaders/shader_src/flat.frag",
                       "");
  Material mat_to_use(E_FACE, shader_to_use);
  Mesh primitive_mesh(mat_to_use);
  for (auto &found_mesh : model.meshes) {

    std::cout << "found a  mesh in file, deserializing..." << std::endl;

    for (auto &mesh_primitives : found_mesh.primitives) {

      std::cout << "found a  primitive in mesh , deserializing..." << std::endl;

      if (mesh_primitives.attributes.find("POSITION") !=
          mesh_primitives.attributes.end()) {
        int accessorIndex = mesh_primitives.attributes.at("POSITION");
        const tinygltf::Accessor &accessor = model.accessors[accessorIndex];
        const tinygltf::BufferView &bufferView =
            model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        size_t vertexCount = accessor.count;
        size_t byteStride = accessor.ByteStride(bufferView);
        const float *data = reinterpret_cast<const float *>(
            &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

        primitive_mesh.m_vertices_array.reserve(vertexCount * 3 + 1);
        primitive_mesh.m_vertices_array.assign(
            data,
            data + vertexCount * 3); // Assuming 3 floats per vertex (x, y, z)
      }

      if (mesh_primitives.attributes.find("NORMAL") !=
          mesh_primitives.attributes.end()) {
        int normalAccessorIndex = mesh_primitives.attributes.at("NORMAL");
        const tinygltf::Accessor &normalAccessor =
            model.accessors[normalAccessorIndex];
        const tinygltf::BufferView &normalBufferView =
            model.bufferViews[normalAccessor.bufferView];
        const tinygltf::Buffer &normalBuffer =
            model.buffers[normalBufferView.buffer];

        // Calculate the number of normals
        size_t normalCount = normalAccessor.count;
        const float *normalDataPtr = reinterpret_cast<const float *>(
            &normalBuffer.data[normalBufferView.byteOffset +
                               normalAccessor.byteOffset]);

        // Copy normal data into the vector
        primitive_mesh.m_normals_array.reserve(normalCount * 3 + 1);
        primitive_mesh.m_normals_array.assign(
            normalDataPtr,
            normalDataPtr +
                normalCount * 3); // Assuming 3 floats per normal (nx, ny, nz)
      }
    }
  }

  return primitive_mesh;
}
