/*
 * Copyright (c) 2016 Galen Cochrane
 * Galen Cochrane <galencochrane@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <cstdlib>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "glm/gtc/quaternion.hpp"
#include "../../common/debug.h"
#include "../../common/scene.h"
#include "../../common/wasdCamera.h"
#include "../../common/game.h"
#include "../../common/meshObject.h"

#include "../../common/ecs/ecsHelpers.h"
#include "../../common/ecs/ecsSystem_movement.h"
#include "../../common/ecs/ecsSystem_controls.h"

using namespace ld2016;
using namespace ecs;

class EcsDemo : public Game {
  private:
    std::shared_ptr<WasdCamera> m_camera;
    std::shared_ptr<MeshObject> m_mesh;
    ControlSystem wasdSystem;
    MovementSystem movementSystem;
  public:
    Delegate<bool(SDL_Event&)> systemsHandlerDlgt;
    EcsDemo(int argc, char **argv)
        : Game(argc, argv, "Entity Component Sytem Demo"), wasdSystem(&state), movementSystem(&state) {
      systemsHandlerDlgt = DELEGATE(&EcsDemo::systemsHandler, this);
    }
    EcsResult init() {
      wasdSystem.init();
      movementSystem.init();

      // Populate the graphics scene
      m_camera = std::shared_ptr<WasdCamera>(
          new WasdCamera( state,
                          80.0f * ((float) M_PI / 180.0f),  // fovy
                          0.1f,  // near
                          100000.0f,  // far
                          glm::vec3(0.0f, 0.0f, 15.0f),  // position
                          glm::angleAxis(
                              (float) M_PI / 4.0f,
                              glm::vec3(1.0f, 0.0f, 0.0f))  // orientation
          ));
      this->scene()->addObject(m_camera);
      this->setCamera(m_camera);
      m_mesh = std::shared_ptr<MeshObject>(
          new MeshObject(state, "assets/models/sphere.dae", "assets/textures/rt_bunny.png"));
      this->scene()->addObject(m_mesh);
      float delta = 0.3f;
      for (int i = 0; i < 100; ++i) {
        Debug::drawLine( state,
                         glm::vec3((float) i * delta, 0.0f, 0.0f),
                         glm::vec3((float) i * delta, 1.0f, 0.0f),
                         glm::vec3(1.0f, 0.0f, 1.0f));
        Debug::drawLine( state,
                         glm::vec3(0.0f, (float) i * delta, 0.0f),
                         glm::vec3(1.0f, (float) i * delta, 0.0f),
                         glm::vec3(1.0f, 0.0f, 1.0f));
      }

      entityId meshId = m_mesh->getId();
      state.addPosition(meshId, {0.f, 0.f, -2.f});
      state.addLinearVel(meshId, {0.f, 0.f, 0.0000001f});
      state.addOrientation(meshId, glm::quat());
      state.addAngularVel(meshId, glm::rotate(glm::quat(), 0.1f, {0.f, 0.f, 1.f}));

      return ECS_SUCCESS;
    }

    bool systemsHandler(SDL_Event& event) {
      return wasdSystem.handleEvent(event);
    }

    void tick(float dt) {
      wasdSystem.tick(dt);
      movementSystem.tick(dt);
    }
};

void main_loop(void *instance) {
  EcsDemo *demo = (EcsDemo *) instance;
  float dt = demo->mainLoop(demo->systemsHandlerDlgt, <#initializer#>);
  demo->tick(dt);
}

int main(int argc, char **argv) {
  EcsDemo demo(argc, argv);
  EcsResult status = demo.init();
  if (status.isError()) { fprintf(stderr, "%s", status.toString().c_str()); }

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg(main_loop, (void*)&demo, 0, 1);
#else
  while (1) {
    main_loop(&demo);
    // TODO: Wait for VSync? Or should we poll input faster than that?
  }
#endif

  return EXIT_SUCCESS;
}
