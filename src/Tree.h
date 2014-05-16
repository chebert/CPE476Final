/*
   Tree.h
   Katie Keim
   CPE 476 - Deer
*/
#ifndef TREE_H_
#define TREE_H_

#include <cstdlib>
#include "graphics/shader.h"
#include "graphics/location_maps.h"
#include "ground_plane.h"
#include "bounding_rectangle.h"
#include "GameObject.h"
#include "graphics/mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include "graphics/shader_setup.h"
#include "graphics/draw_template.h"

struct Tree : public GameObject {
   Tree(glm::vec3 position, Mesh mesh);

   void draw(Shader& shader, const UniformLocationMap& uniform_location_map, const glm::mat4& view_matrix) const;

   BoundingRectangle getBoundingRectangle() {
      return bRect;
   }

   bool isBlocker();
   void performObjectHit(SoundEngine& sound_engine);
   void shadowDraw(Shader& shader, const UniformLocationMap& uniform_locations,
      glm::vec3 sunDir, bool betterShadow);

   glm::mat4 calculateModel() const;
   Drawable drawable() const;

   private:
      BoundingRectangle bRect;
      glm::vec3 position;
      DrawTemplate draw_template_; 
};

#endif //TREE_H_
