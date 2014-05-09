#include "day_night_interaction.h"
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/shader.h"
#include "graphics/shaders.h"
#include "graphics/shader_setup.h"
#include "graphics/material.h"

void DayNightInteraction::drawStart(Shader& shader, const UniformLocationMap& uniform_locations,
                       const glm::mat4& viewMatrix) {
   glm::mat4 transform = glm::translate(glm::mat4(1.0), glm::vec3(-30.0, -6.0, -30.0));
   setupModelView(shader, uniform_locations, transform, viewMatrix, true);
   mesh_.material = Material(glm::vec3(0.5f, 0.7f, 0.5f));
   mesh_.material.sendMaterial(shader, uniform_locations);
   shader.drawMesh(mesh_);
}
void DayNightInteraction::drawStop(Shader& shader, const UniformLocationMap& uniform_locations,
                       const glm::mat4& viewMatrix) {
   glm::mat4 transform = glm::translate(glm::mat4(1.0), glm::vec3(20.0, -6.0, 20.0));
   setupModelView(shader, uniform_locations, transform, viewMatrix, true);
   mesh_.material = Material(glm::vec3(0.7f, 0.5f, 0.5f));
   mesh_.material.sendMaterial(shader, uniform_locations);
   shader.drawMesh(mesh_);
}

void DayNightInteraction::shadowDrawGreen(Shader& shader, const UniformLocationMap& uniform_locations,
      glm::vec3 sunDir) {
   glm::mat4 model_matrix = glm::translate(glm::mat4(1.0), glm::vec3(-30.0, -6.0, -30.0));
   setupShadowShader(shader, uniform_locations, sunDir, model_matrix);
   shader.drawMesh(mesh_);
}

void DayNightInteraction::shadowDrawRed(Shader& shader, const UniformLocationMap& uniform_locations,
      glm::vec3 sunDir) {
   glm::mat4 model_matrix = glm::translate(glm::mat4(1.0), glm::vec3(20.0, -6.0, 20.0));
   setupShadowShader(shader, uniform_locations, sunDir, model_matrix);
   shader.drawMesh(mesh_);
}