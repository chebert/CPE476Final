#include "deer.h"

#include <glm/gtx/vector_angle.hpp>

#include "graphics/shader.h"
#include "graphics/location_maps.h"

namespace {
   glm::vec2 xz(const glm::vec3& vec) {
      return std::move(glm::vec2(vec.x, vec.z));
   }
}

const float kSpeed = 0.010f;
const float kFriction = 0.005f;
const float kAcceleration = 0.00007f;

Deer::Deer(const Mesh& mesh, const glm::vec3& position) :
   mesh_(mesh),
   position_(xz(position)),
   velocity_(0.0f),
   walk_direction_(WalkDirection::NONE),
   strafe_direction_(StrafeDirection::NONE),
   bounding_rectangle_(position_, glm::vec2(3.0f))
      {}

void Deer::draw(Shader& shader, const UniformLocationMap& uniform_locations) const {
   const glm::mat4 rotate(
         glm::rotate(
            glm::mat4(1.0f),
            yRotation(),
            glm::vec3(0, 1, 0)));
   const glm::mat4 translate(
      glm::translate(
            glm::mat4(1.0f),
            glm::vec3(position_.x, 0.0f, position_.y)));
   const glm::mat4 model_matrix(translate * rotate);
   shader.sendUniform(Uniform::MODEL, uniform_locations, model_matrix);
   shader.drawMesh(mesh_);
   bounding_rectangle_.draw(uniform_locations, shader, 0.0f);
}

void Deer::step(units::MS dt, const Camera& camera) {
   if (walk_direction_ == WalkDirection::NONE && strafe_direction_ == StrafeDirection::NONE) {
      velocity_ -= velocity_ * (kFriction * dt);
      if (glm::length(velocity_) < kSpeed / 4.0f) {
         velocity_ = glm::vec2(0.0f);
      }
   } else {
      glm::vec2 acceleration(0.0f);
      { // If walking add in walk based on camera's forward.
         const glm::vec2 forward(xz(camera.getCamForwardVec()));
         if (walk_direction_ == WalkDirection::FORWARD) {
            acceleration = forward;
         } else if (walk_direction_ == WalkDirection::BACKWARD) {
            acceleration = -forward;
         }
      }

      { // Add in strafe from camera, if strafing.
         const glm::vec2 left(xz(camera.getCamLeftVec()));
         if (strafe_direction_ == StrafeDirection::LEFT) {
            acceleration += left;
         } else if (strafe_direction_ == StrafeDirection::RIGHT) {
            acceleration -= left;
         }
      }
      { // Accelerate velocity, capping at kSpeed.
         velocity_ += glm::normalize(acceleration) * (kAcceleration * dt);
         if (glm::length(velocity_) > kSpeed) {
            velocity_ = glm::normalize(velocity_) * kSpeed;
         }
      }
   }
   position_ += velocity_ * static_cast<float>(dt);
   bounding_rectangle_.set_position(position_);
}

void Deer::walkForward() {
   walk_direction_ = WalkDirection::FORWARD;
}
void Deer::walkBackward() {
   walk_direction_ = WalkDirection::BACKWARD;
}
void Deer::stopWalking() {
   walk_direction_ = WalkDirection::NONE;
}

void Deer::strafeLeft() {
   strafe_direction_ = StrafeDirection::LEFT;
}
void Deer::strafeRight() {
   strafe_direction_ = StrafeDirection::RIGHT;
}
void Deer::stopStrafing() {
   strafe_direction_ = StrafeDirection::NONE;
}

float Deer::yRotation() const {
   float angle = glm::angle(glm::vec2(0, 1), glm::normalize(velocity_));
   if (velocity_.x <= 0)
      return 360.0f - angle;
   return angle;
}
