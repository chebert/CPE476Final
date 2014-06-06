#include "game.h"
#include "graphics/mesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include <graphics/material.h>
#include <iostream>
#include "frustum.h"
#include "timer.h"

#include "graphics/texture.h"

namespace {
   bool showTreeShadows = false;
   bool draw_collision_box = false;
   bool switchBlinnPhongShading = false;
   bool eatFlower = false;
   bool useDeferredNotTexture = false;

   int lighting = 0;
   int raining = 0;

   float countLightning = 0.0;
   int numLightning = 0;
   Timer timer;
}

Game::Game() :
   attribute_location_map_({draw_shader_.getShaders().getAttributeLocationMap()}),
   ground_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::GROUND))),
   deer_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::DEER_WALK)),
         Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::DEER_EAT)),
         Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::DEER_SLEEP)),
         Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::DEER_POUNCE)),
         glm::vec3()),
   day_night_boxes_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::TIME_STONE)), ground_),
   treeGen(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::TREE)), 
            Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::LEAF))),
   bushGen(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::BUSH)),
         ground_),

   /* temporary solution to two flower meshes and textures */
   daisyGen(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::DAISY)), 
            Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::EATEN_DAISY)), TextureType::DAISY,
            ground_),
   roseGen(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::ROSE)), 
            Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::EATEN_ROSE)), TextureType::ROSE,
            ground_),

   cardinal_bird_sound_(SoundEngine::SoundEffect::CARDINAL_BIRD, 10000),
   canary_bird_sound_(SoundEngine::SoundEffect::CANARY0, 4000),
   canary2_bird_sound_(SoundEngine::SoundEffect::CANARY1, 7000),
   woodpecker_bird_sound_(SoundEngine::SoundEffect::WOODPECKER0, 3000),

   /* temporary solution to three butterfly textures */
   butterfly_system_red_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::BUTTERFLY)), TextureType::BUTTERFLY_RED,
         glm::vec3(0.0f), 10),
   butterfly_system_pink_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::BUTTERFLY)), TextureType::BUTTERFLY_PINK,
         glm::vec3(40.0f, 0.f, 50.0f), 10),
   butterfly_system_blue_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::BUTTERFLY)), TextureType::BUTTERFLY_BLUE,
         glm::vec3(-60.0f, 0.f, -70.0f), 10),

   rain_system_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::RAIN)),
            glm::vec3(0.0f, 100.0f, 0.0f), 2000),

   lightning_trigger_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::LIGHTNING)), 
            glm::vec3(52.0f, 10.0f, 50.0f), 0.5),

   god_rays_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::GOD_RAYS)), 
            glm::vec3(0.0f, 10.0f, 0.0f), 2.0),

   skybox(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::SKYBOX))),
 
   deerCam(Camera(glm::vec3(150.0f, 150.0f, 150.0f), glm::vec3(0.0f))),

   deferred_fbo_(kScreenWidth, kScreenHeight),

   shadow_map_fbo_(kScreenWidth, kScreenHeight, SHADOW_MAP_TEXTURE, FBOType::DEPTH),
   water_(Mesh::fromAssimpMesh(attribute_location_map_, mesh_loader_.loadMesh(MeshType::GROUND))),
   song_path_(sound_engine_.loadSong(SoundEngine::Song::DAY_SONG),
         Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::GEM))),
   pinecone_(Mesh::fromAssimpMesh(attribute_location_map_,
            mesh_loader_.loadMesh(MeshType::PINECONE)),
         ground_,
         glm::vec2(40))
{
   BoundingRectangle::loadBoundingMesh(mesh_loader_, attribute_location_map_);

   std::vector<GameObject*> objects;

   for (auto& tree : treeGen.getTrees()) {
      objects.push_back(&tree);
   }
   for (auto& bush : bushGen.getBushes()) {
      objects.push_back(&bush);
   }
   for (auto& flower : daisyGen.getFlowers()) {
      objects.push_back(&flower);
   }
   for (auto& flower : roseGen.getFlowers()) {
      objects.push_back(&flower);
   }

   objTree.calculateTree(objects);
}

void Game::step(units::MS dt) {
   if (day_cycle_.isDaytime()) {
      cardinal_bird_sound_.step(dt, sound_engine_);
      canary_bird_sound_.step(dt, sound_engine_);
      canary2_bird_sound_.step(dt, sound_engine_);
      woodpecker_bird_sound_.step(dt, sound_engine_);
   }

   sound_engine_.set_listener_position(deer_.getPosition(), deer_.getFacing());

   butterfly_system_red_.step(dt);
   butterfly_system_pink_.step(dt);
   butterfly_system_blue_.step(dt);

   rain_system_.step(dt);

   if(numLightning) {
      countLightning += dt/100.0;
      if(countLightning < 0.0)
         lighting = 0;
      else if(countLightning >= 0.0 && countLightning <= 1.0)
         lighting = 1;
      else if(countLightning > 1.0) {
         countLightning = -2.0;
         numLightning--;
         if(numLightning == 1)
            countLightning = -7.5;
      }
   } else {
      lighting = 0;
   }

   bool deerBlocked = false;
   if (deer_.isMoving()) {
      BoundingRectangle nextDeerRect = deer_.getNextBoundingBox(dt);
      std::vector<GameObject*> collObjs = objTree.getCollidingObjects(nextDeerRect);
      for (auto collObj : collObjs) {
         collObj->performObjectHit(sound_engine_);
         deerBlocked = deerBlocked || collObj->isBlocker();
      }
   }

   if (deerBlocked) {
      deer_.block();
   } else {
      deer_.step(dt, ground_, sound_engine_);
   }

   song_path_.step(dt, deer_.bounding_rectangle());

   for (auto& bush : bushGen.getBushes()) {
      bush.step(dt);
   }

   for (auto& tree : treeGen.getTrees()) {
      tree.step(dt);
   }

   for(auto& flower : daisyGen.getFlowers()) {
      if(eatFlower && deer_.head_bounding_rectangle().collidesWith(flower.bounding_rectangle())) {
         deer_.eat(flower);
      }
   }
   for(auto& flower : roseGen.getFlowers()) {
      if(eatFlower && deer_.head_bounding_rectangle().collidesWith(flower.bounding_rectangle())) {
         deer_.eat(flower);
      }
   }
   eatFlower = false;

   if (!pinecone_.been_pounced() && deer_.bounding_rectangle().collidesWith(pinecone_.aoe_bounding_rectangle())) {
      deer_.pounce(pinecone_.bounding_rectangle().getCenter());
      pinecone_.deer_pounces();
   }

   if(deer_.bounding_rectangle().collidesWith(lightning_trigger_.bounding_rectangle())) {
      if (numLightning == 0) {
         if (!raining) {
            song_path_.set_song(sound_engine_.loadSong(SoundEngine::Song::STORM_SONG));
            song_path_.reset();
         }
         raining = 1;
         lighting = 1;
         numLightning = 3;
         sound_engine_.playSoundEffect(SoundEngine::SoundEffect::THUNDER_STRIKE, false, glm::vec3());
      }
   }

   if (deer_.bounding_rectangle().collidesWith(day_night_boxes_.bounding_rectangle_moon())) {
      if (day_cycle_.isDaytime()) {
         song_path_.set_song(sound_engine_.loadSong(SoundEngine::Song::NIGHT_SONG));
         song_path_.reset();
      }
      day_cycle_.dayToNight();
   }
   else if (deer_.bounding_rectangle().collidesWith(day_night_boxes_.bounding_rectangle_sun())) {
      if (!day_cycle_.isDaytime()) {
         song_path_.set_song(sound_engine_.loadSong(SoundEngine::Song::DAY_SONG));
         song_path_.reset();
      }
      day_cycle_.nightToDay();
   }
   day_cycle_.autoAdjustTime(dt);

   Camera::Position cam_pos;
   glm::vec3 target_pos(deer_.getPosition());
   if (deer_.is_sleeping()) {
      cam_pos = Camera::Position::FRONT_RIGHT;
   } else if (deer_.is_eating()) {
      cam_pos = Camera::Position::LEFT;
   } else {
      cam_pos = Camera::Position::BEHIND;
   }
   if (deer_.is_sleeping() || deer_.is_eating()) {
      glm::vec2 head_pos(deer_.head_bounding_rectangle().getCenter());
      target_pos.x = head_pos.x;
      target_pos.z = head_pos.y;
   }
   deerCam.step(dt, target_pos, deer_.getFacing(), cam_pos);
}

void Game::draw() {
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   const auto sunIntensity = day_cycle_.getSunIntensity();
   glClearColor (0.05098 * sunIntensity,
         0.6274509 * sunIntensity,
         sunIntensity, 1.0f);

   std::vector<Drawable> drawables;
   if (draw_collision_box) {
      Drawable br_drawable;
      br_drawable.draw_template = BoundingRectangle::draw_template();
      br_drawable.model_transforms.push_back(deer_.front_feet_bounding_rectangle().model_matrix());
      /*
      br_drawable.model_transforms.push_back(lightning_trigger_.bounding_rectangle().model_matrix());
      br_drawable.model_transforms.push_back(day_night_boxes_.bounding_rectangle_sun().model_matrix());
      br_drawable.model_transforms.push_back(day_night_boxes_.bounding_rectangle_moon().model_matrix());
      for (auto& bush : bushGen.getBushes()) {
         br_drawable.model_transforms.push_back(bush.getBoundingRectangle().model_matrix());
      }
      for (auto& tree : treeGen.getTrees()) {
         br_drawable.model_transforms.push_back(tree.getBoundingRectangle().model_matrix());
      }
      for (auto& flower : daisyGen.getFlowers()) {
         br_drawable.model_transforms.push_back(flower.getBoundingRectangle().model_matrix());
      }
      for (auto& flower : roseGen.getFlowers()) {
         br_drawable.model_transforms.push_back(flower.getBoundingRectangle().model_matrix());
      }
      for (auto& br : song_path_.bounding_rectangles()) {
         br_drawable.model_transforms.push_back(br.model_matrix());
      }
      */
      br_drawable.model_transforms.push_back(pinecone_.bounding_rectangle().model_matrix());
      br_drawable.model_transforms.push_back(pinecone_.aoe_bounding_rectangle().model_matrix());
      drawables.push_back(br_drawable);
   }

   drawables.push_back(deer_.drawable());

   /*
   drawables.push_back(lightning_trigger_.drawable());
   drawables.push_back(day_night_boxes_.drawableSun());
   drawables.push_back(day_night_boxes_.drawableMoon());

   drawables.push_back(bushGen.drawable());
   drawables.push_back(treeGen.drawable());
   std::vector<Drawable> leaf_drawables = treeGen.leafDrawable();
   for (auto& leafDrawable : leaf_drawables) {
      drawables.push_back(leafDrawable);
   }

   drawables.push_back(daisyGen.drawable());
   drawables.push_back(daisyGen.drawableEaten());
   drawables.push_back(roseGen.drawable());
   drawables.push_back(roseGen.drawableEaten());

   drawables.push_back(song_path_.drawable());
   if (raining)
      drawables.push_back(rain_system_.drawable());

   drawables.push_back(butterfly_system_red_.drawable());
   drawables.push_back(butterfly_system_pink_.drawable());
   drawables.push_back(butterfly_system_blue_.drawable());

   drawables.push_back(ground_.drawable());
   drawables.push_back(water_.drawable());

   */
   drawables.push_back(pinecone_.drawable());

   //god_rays_.setRayPositions(song_path_.CurrentStonePosition(), song_path_.NextStonePosition());
   //god_rays_.setCurrentRayScale(song_path_.CurrentStoneRemainingRatio());
   //drawables.push_back(god_rays_.drawable());
   //br_drawable.model_transforms.push_back(god_rays_.bounding_rectangle().model_matrix());

   // View Frustum Culling
   auto viewMatrix = deerCam.getViewMatrix();
   const auto view_projection = kProjectionMatrix * viewMatrix;
   Frustum frustum(view_projection);
   auto culledDrawables = frustum.cullDrawables(drawables);

   // Skybox is never culled, so we add it after.
   culledDrawables.push_back(CulledDrawable::fromDrawable(skybox.drawable(day_cycle_.isDay())));

   if(useDeferredNotTexture) {
      for(auto& drawable : culledDrawables) {
         if(drawable.draw_template.shader_type == ShaderType::TEXTURE) 
            drawable.draw_template.shader_type = ShaderType::DEFERRED;
      }
   }

   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   const auto deerPos = deer_.getPosition();
   const auto sunDir = day_cycle_.getSunDir();
   draw_shader_.Draw(shadow_map_fbo_, water_.fbo(), deferred_fbo_, 
         culledDrawables, viewMatrix, switchBlinnPhongShading, 
         deerPos, sunDir, sunIntensity, lighting);
}

void Game::mainLoop() {
   Input input;
   showTreeShadows = false;
   bool running = true;
   SDL_Event event;
   units::MS previous_time = SDL_GetTicks();

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
            } else if (input.isKeyHeld(key_backward) && !input.isKeyHeld(key_forward)) {
               deer_.walkBackward();
            } else {
               deer_.stopWalking();
            }
         }
         { // handle strafe left/right for deer.
            const auto key_left = SDL_SCANCODE_A;
            const auto key_right = SDL_SCANCODE_D;
            if (input.isKeyHeld(key_left) && !input.isKeyHeld(key_right)) {
               deer_.turnLeft();
            } else if (!input.isKeyHeld(key_left) && input.isKeyHeld(key_right)) {
               deer_.turnRight();
            } else {
               deer_.stopTurning();
            }
         }
         { // handle jumping
            const auto key_jump = SDL_SCANCODE_J;
            if (input.wasKeyPressed(key_jump)) {
               deer_.jump();
            }
         }
         { // handle jumping
            const auto key_eat = SDL_SCANCODE_E;
            if (input.wasKeyPressed(key_eat)) {
               eatFlower = true;
            }
         }
         { // show or hide tree shadows -- Katelyn
            const auto key_tree = SDL_SCANCODE_T;
            if (input.wasKeyPressed(key_tree)) {
               showTreeShadows = !showTreeShadows;
               treeGen.includeInShadows(showTreeShadows); 
            }
         }
         {
            const auto key_toNight = SDL_SCANCODE_3;
            if (input.wasKeyPressed(key_toNight)) {
               day_cycle_.dayToNight();
            }
         }
         {
            const auto key_toDay = SDL_SCANCODE_4;
            if (input.wasKeyPressed(key_toDay)) {
               day_cycle_.nightToDay();
            }
         }
         { // Lightning
            const auto key_lightning = SDL_SCANCODE_L;
            if (input.wasKeyPressed(key_lightning)) {
               raining = 1;
               lighting = 1;
               numLightning = 3;
               sound_engine_.playSoundEffect(SoundEngine::SoundEffect::THUNDER_STRIKE, false, glm::vec3());
            }
         }
         { // Rain
            const auto key_rain = SDL_SCANCODE_R;
            if (input.wasKeyPressed(key_rain)) {
               raining = !raining;
            }
         }
         { //Change shading models
            const auto key_blinn = SDL_SCANCODE_B;
            if (input.wasKeyPressed(key_blinn)) {
               switchBlinnPhongShading = !switchBlinnPhongShading; 
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
