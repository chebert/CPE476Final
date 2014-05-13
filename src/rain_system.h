#ifndef RAIN_SYSTEM_H_
#define RAIN_SYSTEM_H_

#include "particle.h"
#include "units.h"
#include "graphics/location_maps.h"
#include "graphics/assimp/mesh_loader.h"
#include "graphics/texture.h"

struct RainSystem {
   RainSystem(const glm::vec3& origin, int numParticles,
	       const AttributeLocationMap& attribute_location_map_, MeshLoader& mesh_loader_);

   void step(units::MS dt);

   void draw(Shader& shader,
         const UniformLocationMap& uniform_location_map,
         const glm::mat4& view_matrix);

   void reset();
   
   private:
      std::vector<Particle> particles_;
      Texture texture_;
      glm::vec3 origin_;
      float scale_;
      glm::vec3 velocity_;
      glm::vec3 acceleration_;
};

#endif // RAIN_SYSTEM_H_