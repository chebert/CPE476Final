/* 
   TreeGenerator.cpp
   Katie Keim
   Deer - CPE 476
*/
#include "TreeGenerator.h"

const int TREE_SIZE = 20;
const int TREE_DENSITY = 4; //Higher numbers here will mean less trees.
const int TREE_SCALE = 5;
const int TREE_VARIANCE = 40;

TreeGenerator::TreeGenerator(const Mesh& mesh, const Mesh& leaf) :
   draw_template_({
         ShaderType::DEFERRED,
         mesh, 
         Material(),
         Texture(TextureType::TREE, DIFFUSE_TEXTURE),
         boost::none,
         EffectSet({EffectType::CASTS_REFLECTION, EffectType::VARY_MATERIAL})
         }),
   leaf_(leaf)
{
   draw_template_.material = Material(glm::vec3(1.2) * glm::vec3(0.45, 0.24, 0.15));
   generate();
}

//Generate the trees
void TreeGenerator::generate() {
   int size = GroundPlane::GROUND_SCALE / TREE_SIZE; 
   int groundSize = GroundPlane::GROUND_SCALE / 2;

   for (int row = 0; row < size; row++) {
      for (int col = 0; col < size; col++) {
         if (rand() % TREE_DENSITY == 0) {
            float height =  ((rand() % TREE_VARIANCE) / (float)TREE_VARIANCE) + 1.0f;
            float width =  ((rand() % TREE_VARIANCE) / (float)TREE_VARIANCE) + 1.0f;
            float angleRot = rand() % 360;
            float x = row * TREE_SIZE - groundSize + rand() % TREE_SIZE;
            float y = height * TREE_SCALE * TREE_SIZE / 2;
            float z = col * TREE_SIZE - groundSize + rand() % TREE_SIZE;

           if (!(x < 80.0f && x > -80.0f && z < 80.0f && z > -80.0f)) {
               trees.push_back(Tree(glm::vec3(x, y, z), height, width, angleRot, leaf_));
           }
         }
      }
   }
}

void TreeGenerator::includeInShadows(bool value) {
   if (value)
      draw_template_.effects.insert(EffectType::CASTS_SHADOW);
   else
      draw_template_.effects.erase(EffectType::CASTS_SHADOW);
}

std::vector<Tree>& TreeGenerator::getTrees() {
   return trees;
}

Drawable TreeGenerator::drawable() const {
   std::vector<DrawInstance> model_matrices;
   for (auto& tree : trees) {
      model_matrices.push_back(tree.draw_instance());
   }
   return Drawable({draw_template_, model_matrices});
}

Drawable TreeGenerator::leafDrawable() const {
   Drawable ret;
   for (auto& tree : trees) {
      const auto& draw_instances = tree.leaf_system_.drawable().draw_instances;
      ret.draw_template = tree.leaf_system_.drawable().draw_template;
      ret.draw_instances.insert(ret.draw_instances.end(),
            draw_instances.begin(),
            draw_instances.end());
   }
   return ret;
}
