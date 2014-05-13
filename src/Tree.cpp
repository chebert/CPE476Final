/*
   Tree.cpp
   Katie Keim
   CPE 476 - Deer
*/
#include "Tree.h"

const int TREE_SIZE = 20;
const int TREE_SCALE = 5;

const int BOUNDING_SIZE = 4;
const float BOUNDING_ERR_X = 7.26;
const float BOUNDING_ERR_Z = -1.75;

const int groundSize = GroundPlane::GROUND_SCALE / 2;

Tree::Tree(glm::vec3 position, Mesh mesh) :
   position(position),
   mesh(mesh),
   bRect(BoundingRectangle(glm::vec2(position.x + BOUNDING_ERR_X, position.z + BOUNDING_ERR_Z), 
      glm::vec2(BOUNDING_SIZE, BOUNDING_SIZE), 0.0f))
{}

void Tree::draw(Shader& shader, const UniformLocationMap& uniform_location_map, const glm::mat4& view_matrix) const{
   glm::mat4 rotateTreeUp;
   glm::mat4 translate;
   glm::mat4 scale;
   glm::mat4 model_matrix;
   mesh.material.sendMaterial(shader, uniform_location_map);

   scale = glm::scale(glm::mat4(1.0f), glm::vec3(TREE_SCALE));

   rotateTreeUp = glm::rotate(
               glm::mat4(1.0f),
               (float)(-90),
               glm::vec3(1, 0, 0));

   translate = glm::translate(
                glm::mat4(1.0f),
               glm::vec3(position.x, position.y, position.z));
   model_matrix = translate * scale * rotateTreeUp;

   setupModelView(shader, uniform_location_map, model_matrix,
                           view_matrix, true);
   shader.drawMesh(mesh);
   bRect.draw(uniform_location_map, shader, 0, view_matrix);
}

bool Tree::isBlocker() {
   return true;
}

void Tree::shadowDraw(Shader& shader, const UniformLocationMap& uniform_locations,
      glm::vec3 sunDir, glm::vec3 deerPos, bool betterShadow) {
   const auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(TREE_SCALE));

   const auto rotateTreeUp = glm::rotate(
         glm::mat4(1.0f),
         (float)(-90),
         glm::vec3(1, 0, 0));

   const auto translate = glm::translate(
         glm::mat4(1.0f),
         glm::vec3(position.x, position.y, position.z));
   const glm::mat4 model_matrix(translate * scale * rotateTreeUp);

   if(betterShadow)
      setupBetterShadowShader(shader, uniform_locations, sunDir, deerPos, model_matrix);
   else
      setupShadowShader(shader, uniform_locations, sunDir, deerPos, model_matrix);
   shader.drawMesh(mesh);
}

void Tree::performObjectHit(SoundEngine& sound_engine) {
}