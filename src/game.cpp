#include "game.h"
#include "graphics/mesh.h"
#include "graphics/shader_setup.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace {
   DeerCam deerCam;
   Mesh box;
}

Game::Game() :
   texture_(texture_path(Textures::GRASS)),
   deer_texture_(texture_path(Textures::DEER)),
   attribute_location_map_(shaders_.getAttributeLocationMap()),
   uniform_location_map_(shaders_.getUniformLocationMap()),
   ground_(attribute_location_map_),
   deer_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh("../models/Test_Deer2.dae")), glm::vec3(0.0f)),
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
   mouseDown = false;
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
   
   day_cycle_.autoAdjustTime(dt);
}

void Game::draw() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   if(deer_.getPosition().x < -27.0 && deer_.getPosition().x > -32.0 &&
         deer_.getPosition().z < -27.0 && deer_.getPosition().z > -32.0) {
      day_cycle_.on();
   }
   if(deer_.getPosition().x > 17.0 && deer_.getPosition().x < 22.0 &&
         deer_.getPosition().z > 17.0 && deer_.getPosition().z < 22.0) {
      day_cycle_.off();
   }

   float sunIntensity = day_cycle_.getSunIntensity();
   glm::vec3 sunDir = day_cycle_.getSunDir();
   glm::mat4 viewMatrix = deerCam.getViewMatrix();
   
   for (auto& shaderPair: shaders_.getMap()) {
      Shader& shader = shaderPair.second;
      shader.use();
      
      setupProjection(shader, uniform_location_map_);

      if(shaderPair.first == ShaderType::TEXTURE) {
         setupTextureShader(shader, uniform_location_map_, sunIntensity, texture_.textureID());
         texture_.enable();
   
         ground_.draw(shader, uniform_location_map_, viewMatrix);
         
         texture_.disable();
      }
      else if(shaderPair.first == ShaderType::SUN) {
         setupView(shader, uniform_location_map_, viewMatrix);
         setupSunShader(shader, uniform_location_map_, sunIntensity, sunDir);

         //ON BOX
         shader.sendUniform(Uniform::HAS_BONES, uniform_location_map_, 1);
         //shader.sendUniform(Uniform::BONES, uniform_location_map_, Bone::calculateBoneTransformations(box.bone_array));
         shader.sendUniform(Uniform::BONES, uniform_location_map_, std::vector<glm::mat4>(5, glm::mat4(1.0f)));
         setupModelView(shader, uniform_location_map_,
               glm::translate(glm::mat4(1.0), glm::vec3(-30.0, 3.0, -30.0)), viewMatrix, true);
         sendMaterial(shader, uniform_location_map_, glm::vec3(0.5f, 0.7f, 0.5f));
         shader.drawMesh(box);

         //OFF BOX
         //shader.sendUniform(Uniform::HAS_BONES, uniform_location_map_, 1);
         //shader.sendUniform(Uniform::BONES, uniform_location_map_, Bone::calculateBoneTransformations(box.bone_array));
         //setupModelView(shader, uniform_location_map_,
               //glm::translate(glm::mat4(1.0), glm::vec3(20.0, 3.0, 20.0)), viewMatrix, true);
         //sendMaterial(shader, uniform_location_map_, glm::vec3(0.7f, 0.5f, 0.5f));
         //shader.drawMesh(box);

         shader.sendUniform(Uniform::HAS_BONES, uniform_location_map_, 0);

         for (auto& bush : bushes_) {
            bush.draw(shader, uniform_location_map_, viewMatrix);
         }
         deer_.draw(shader, uniform_location_map_, viewMatrix);
         //treeGen.drawTrees(shader, uniform_location_map_, viewMatrix);
      }
      else if(shaderPair.first == ShaderType::WIREFRAME)
         setupWireframeShader(shader, uniform_location_map_, glm::vec4(1, 0, 0, 1));
      
   }
}

void Game::mainLoop() {
   box = Mesh::fromAssimpMesh(attribute_location_map_, mesh_loader_.loadMesh("../models/dance_cyl.dae"));
   Input input;
   int mX, mY;
   bool running = true;
   SDL_Event event;
   units::MS previous_time = SDL_GetTicks();

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
               //day_cycle_.switchBoolean();
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
