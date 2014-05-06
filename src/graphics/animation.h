#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <glm/glm.hpp>
#include <vector>

#include "graphics/assimp/bone.h"
#include "units.h"

struct Animation {
   Animation(double duration=0.0, double ticks_per_second=25.0) :
      current_time_(0.0),
      duration_(duration),
      ticks_per_ms_(ticks_per_second / 1000.0) {}

   void step(units::MS dt) {
      current_time_ += dt * ticks_per_ms_;
      if (current_time_ > duration_)
         current_time_ = 0.0;
   }

   std::vector<glm::mat4> calculateBoneTransformations(
         const std::vector<Bone>& bones) const {
      return Bone::calculateBoneTransformations(bones, current_time_);
   }

  private:
   double current_time_;
   double duration_, ticks_per_ms_;
};

#endif // ANIMATION_H_
