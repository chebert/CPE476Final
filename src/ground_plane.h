#ifndef GROUND_PLANE_H_
#define GROUND_PLANE_H_

#include "graphics/mesh.h"
#include "glm/glm.hpp"
#include "graphics/location_maps.h"

#include <ImageMagick-6/Magick++.h>

struct Shader;
struct Shaders;

struct GroundPlane {
   static const int GROUND_SCALE;
   GroundPlane(const Mesh& mesh);
   void draw(Shader& shader, const UniformLocationMap& uniform_locations,
             const glm::mat4& viewMatrix);

   float heightAt(const glm::vec3& position) const;

  private:
   glm::mat4 transform_;
   Texture texture_, height_map_;
   Mesh mesh_;
   Magick::Image height_map_image_;
};

#endif // GROUND_PLANE_H_
