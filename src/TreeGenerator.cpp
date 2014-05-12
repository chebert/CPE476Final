/* 
   TreeGenerator.cpp
   Katie Keim
   Deer - CPE 476
*/
#include "TreeGenerator.h"

const int TREE_SIZE = 20;
const int TREE_DENSITY = 4; //Higher numbers here will mean less trees.
const int TREE_SCALE = 5;

const float BOUNDING_ERR_X = 7.26;
const float BOUNDING_ERR_Z = -1.75;

TreeGenerator::TreeGenerator(const Mesh& mesh) {
   treeMesh1 = mesh;
   treeMesh1.material = Material(glm::vec3(1.2) * glm::vec3(0.45, 0.24, 0.15));
}

//Generate the trees
void TreeGenerator::generate() {
   int size = GroundPlane::GROUND_SCALE / TREE_SIZE; 
   int groundSize = GroundPlane::GROUND_SCALE / 2;

   for (int row = 0; row < size; row++) {
      for (int col = 0; col < size; col++) {
         if (rand() % TREE_DENSITY == 0) {
            trees.push_back(Tree(glm::vec3(row * TREE_SIZE - groundSize, TREE_SCALE * TREE_SIZE / 2, col * TREE_SIZE - groundSize), treeMesh1));
         }
      }
   }
}

std::vector<Tree>& TreeGenerator::getTrees() {
   return trees;
}

void TreeGenerator::shadowDraw(Shader& shader, const UniformLocationMap& uniform_locations,
      glm::vec3 sunDir, glm::vec3 deerPos, bool betterShadow) {
   for (auto& tree : trees) {
      tree.shadowDraw(shader, uniform_locations, sunDir, deerPos, betterShadow);
   }
}

void TreeGenerator::drawTrees(Shader& shader, const UniformLocationMap& uniform_location_map, const glm::mat4& view_matrix) {
   for (auto& tree : trees) {
      tree.draw(shader, uniform_location_map, view_matrix);
   }
}
