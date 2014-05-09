#include "game.h"
#include "graphics/mesh.h"
#include "graphics/shader_setup.h"
#include <glm/gtc/matrix_transform.hpp>
#include <graphics/material.h>
#include <iostream>

namespace {
   DeerCam deerCam;
}

Game::Game() :
   texture_(texture_path(Textures::GRASS)),
   attribute_location_map_(shaders_.getAttributeLocationMap()),
   uniform_location_map_(shaders_.getUniformLocationMap()),
   ground_(attribute_location_map_),
   deer_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh("../models/deer_butt.dae")), glm::vec3(0.0f)),
   day_night_boxes_(Mesh::fromAssimpMesh(attribute_location_map_, mesh_loader_.loadMesh("../models/cube.obj"))),
   treeGen(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh("../models/tree2.3ds"))),
   tree_mesh_(Mesh::fromAssimpMesh(
            attribute_location_map_,
            mesh_loader_.loadMesh("../models/tree.3ds"))),
   bushes_{
      Tree(tree_mesh_,
            glm::vec3(30 - 15, 0, 25 + 5),
            1.2f,
            300),
      Tree(tree_mesh_,
            glm::vec3(20 - 15, 0, 18 + 5),
            0.8f,
            450),
      Tree(tree_mesh_,
            glm::vec3(25 - 15, 0, 12 + 5),
            0.9f,
            150),
      Tree(tree_mesh_,
            glm::vec3(12 - 15, 0, 24 + 5),
            1.3f,
            400),
   }
{
   //glClearColor(0, 0, 0, 1); // Clear to solid blue.

   std::cout << "GL version " << glGetString(GL_VERSION) << std::endl;
   std::cout << "Shader version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
   glClearColor (0.05098 * 0.5, 0.6274509 * 0.5, 0.5, 1.0f);
   glClearDepth(1.0f);
   glDepthFunc(GL_LESS);
   glEnable(GL_DEPTH_TEST);// Enable Depth Testing
   glDisable(GL_LIGHTING);
   glShadeModel(GL_SMOOTH);
   glDisable(GL_LINE_SMOOTH);
   glEnable(GL_CULL_FACE);

   glPolygonMode(GL_FRONT, GL_LINE);
   glLineWidth(1.0);

   BoundingRectangle::loadBoundingMesh(mesh_loader_, attribute_location_map_);
   deerCam.initialize(deer_.getPosition());
   treeGen.generateTrees();
   //SDL_SetRelativeMouseMode(true);
}

void Game::step(units::MS dt) {
   bool treeColl = false;

   deer_.step(dt, deerCam);
   for (auto& tree : bushes_) {
      tree.step(dt);
   }

   if (deer_.isMoving()) {
      deerCam.move(deer_.getPosition());
      for (auto& tree : bushes_) {
         if (deer_.bounding_rectangle().collidesWith(tree.bounding_rectangle())) {
            tree.rustle();
         }
      }
   }

   for (auto& box : treeGen.getBoundingBoxes()) {
      treeColl = treeColl || deer_.bounding_rectangle().collidesWith(box);
   }

   if (treeColl)
      deer_.jump();

   if (deer_.bounding_rectangle().collidesWith(day_night_boxes_.bounding_rectangle_start())) {
      day_cycle_.on();
   }
   else if (deer_.bounding_rectangle().collidesWith(day_night_boxes_.bounding_rectangle_stop())) {
      day_cycle_.off();
   }

   day_cycle_.autoAdjustTime(dt);
}

void Game::draw() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   float sunIntensity = day_cycle_.getSunIntensity();
   glm::vec3 sunDir = day_cycle_.getSunDir();
   glm::mat4 viewMatrix = deerCam.getViewMatrix();

   for (auto& shaderPair: shaders_.getMap()) {
      Shader& shader = shaderPair.second;
      shader.use();

      setupProjection(shader, uniform_location_map_);

      if(shaderPair.first == ShaderType::SHADOW) {
         //shadow_map_fbo_.BindForWriting();
         //glClear(GL_DEPTH_BUFFER_BIT);

         deer_.shadowDraw(shader, uniform_location_map_, sunDir);
         day_night_boxes_.shadowDrawRed(shader, uniform_location_map_, sunDir);
         day_night_boxes_.shadowDrawGreen(shader, uniform_location_map_, sunDir);
         treeGen.shadowDraw(shader, uniform_location_map_, sunDir);
         for (auto& bush : bushes_) {
            bush.shadowDraw(shader, uniform_location_map_, sunDir);
         }

         //glBindFramebuffer(GL_FRAMEBUFFER, 0);
         //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         //shadow_map_fbo_.BindForReading();
      }
      else if(shaderPair.first == ShaderType::TEXTURE) {
         sendShadowInverseProjectionView(shader, uniform_location_map_, sunDir);
         setupView(shader, uniform_location_map_, viewMatrix);
         setupSunShader(shader, uniform_location_map_, sunIntensity, sunDir);
         setupTextureShader(shader, uniform_location_map_, texture_.textureID());
         texture_.enable();
         ground_.draw(shader, uniform_location_map_, viewMatrix);
         texture_.disable();

         deer_.draw(shader, uniform_location_map_, viewMatrix);
      }
      else if(shaderPair.first == ShaderType::SUN) {
         sendShadowInverseProjectionView(shader, uniform_location_map_, sunDir);
         setupView(shader, uniform_location_map_, viewMatrix);
         setupSunShader(shader, uniform_location_map_, sunIntensity, sunDir);

         day_night_boxes_.drawStop(shader, uniform_location_map_, viewMatrix);
         day_night_boxes_.drawStart(shader, uniform_location_map_, viewMatrix);

         for (auto& bush : bushes_) {
            bush.draw(shader, uniform_location_map_, viewMatrix);
         }
         treeGen.drawTrees(shader, uniform_location_map_, viewMatrix);
      }
      else if(shaderPair.first == ShaderType::WIREFRAME) {
         setupWireframeShader(shader, uniform_location_map_, glm::vec4(1, 0, 0, 1));
      }
      //If pixel is under ground draw as blue (water)?

   }
}

void Game::mainLoop() {
   Input input;
   int mX, mY;
   bool running = true;
   SDL_Event event;
   units::MS previous_time = SDL_GetTicks();

   printf("Before setup\n");
   /*
   if(!shadow_map_fbo_.setup(kScreenWidth, kScreenHeight)) {
      printf("FAILURE\n");
      return;
   }
   */
   SDL_WarpMouseInWindow(NULL, kScreenWidth / 2, kScreenHeight / 2);
   mousePos = glm::vec2(kScreenWidth / 2, kScreenHeight / 2);

   while (running) {
      {  // Collect input
         input.beginFrame();
         while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
               running = false;
            }

            if (event.type == SDL_KEYDOWN && event.key.repeat == false) {
               input.keyDown(event.key);
            } else if (event.type == SDL_KEYUP) {
               input.keyUp(event.key);
            }

            if (event.type == SDL_MOUSEMOTION) {
               SDL_GetRelativeMouseState(&mX, &mY);
               deerCam.rotatePositionWithDrag(mX, mY, kScreenWidth, kScreenHeight);
               mousePos = glm::vec2(mX, mY);
            }
         }
      }
      { // Handle input
         if (input.wasKeyPressed(SDL_SCANCODE_ESCAPE)) {
            running = false;
         }
         { // handle walk forward/backward for deer.
            const auto key_forward = SDL_SCANCODE_W;
            const auto key_backward = SDL_SCANCODE_S;
            if (input.isKeyHeld(key_forward) && !input.isKeyHeld(key_backward)) {
               deer_.walkForward();
            } else if (!input.isKeyHeld(key_forward) && input.isKeyHeld(key_backward)) {
               deer_.walkBackward();
            } else {
               deer_.stopWalking();
            }
         }
         { // handle strafe left/right for deer.
            const auto key_left = SDL_SCANCODE_A;
            const auto key_right = SDL_SCANCODE_D;
            if (input.isKeyHeld(key_left) && !input.isKeyHeld(key_right)) {
               deer_.strafeLeft();
            } else if (!input.isKeyHeld(key_left) && input.isKeyHeld(key_right)) {
               deer_.strafeRight();
            } else {
               deer_.stopStrafing();
            }
         }
         { // handle jumping
            const auto key_jump = SDL_SCANCODE_J;
            if (input.wasKeyPressed(key_jump)) {
               deer_.jump();
               day_cycle_.switchBoolean();
            }
         }
         { //handle quit
            const auto key_quit = SDL_SCANCODE_Q;
            if (input.wasKeyPressed(key_quit)) {
               running = false;
            }
         }
      }

      {
         const units::MS current_time = SDL_GetTicks();
         const units::MS dt = current_time - previous_time;
         step(dt);
         previous_time = current_time;
      }

      {
         draw();
         engine_.swapWindow();
      }

      SDL_Delay(5);
   }
}
