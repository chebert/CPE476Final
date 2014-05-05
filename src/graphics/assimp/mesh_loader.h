#ifndef MESH_LOADER_H_
#define MESH_LOADER_H_

#include <assimp/anim.h>

#include <boost/serialization/strong_typedef.hpp>
#include <map>
#include <string>
#include <vector>

#include <glm/gtc/quaternion.hpp>

#include "graphics/assimp/bone.h"

struct AssimpMesh {
   typedef std::pair<BoneID, float> BoneIDAndWeight;
   std::vector<float> vertex_array;
   std::vector<float> normal_array;
   std::vector<float> uv_array;
   std::vector<unsigned short> index_array;

   // Array of bones for each vertex.
   std::vector<std::vector<BoneIDAndWeight>> bone_weights_array;

   // Canonical array of Bones. Access with BoneID's.
   // Length is the number of bones in the scene NOT the number of vertices.
   std::vector<Bone> bone_array;
   glm::mat4 global_inverse_transform;
};

struct MeshLoader {
   AssimpMesh loadMesh(const std::string& path);

  private:
   std::map<std::string, AssimpMesh> meshes_;
};

#endif // MESH_LOADER_H_
