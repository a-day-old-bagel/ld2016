/*
 * Copyright (c) 2016 Jonathan Glines, Galen Cochrane
 * Jonathan Glines <jonathan@glines.net>
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

#include "../../common/debug.h"
#include "../../common/game.h"
#include "../../common/meshObject.h"
#include "../../common/scene.h"
#include "../../common/wasdCamera.h"

#include "../../common/ecs/ecsHelpers.h"
#include "../../common/ecs/ecsSystem_movement.h"
#include "../../common/ecs/ecsSystem_controls.h"

using namespace ld2016;
using namespace ecs;

static glm::vec3 pyrFireWiggle(const glm::vec3& initialScale, uint32_t time) {
  return { initialScale.x, initialScale.y, initialScale.z * (1.f + sin(time * 0.1f)) };
}

class AnimationDemo : public Game {
  private:
    std::shared_ptr<WasdCamera> m_camera;
    std::shared_ptr<MeshObject> m_pyrBottom, m_pyrTop, m_pyrThrusters, m_pyrFire;
    std::shared_ptr<SceneObject> m_camGimbal;
    ControlSystem wasdSystem;
    MovementSystem movementSystem;
  public:
    Delegate<bool(SDL_Event&)> systemsHandlerDlgt;
    AnimationDemo(int argc, char **argv)
        : Game(argc, argv, "Animation Demo"), wasdSystem(&state), movementSystem(&state) {
      systemsHandlerDlgt = DELEGATE(&AnimationDemo::systemsHandler, this);
    }
    EcsResult init() {
      assert(wasdSystem.init());
      assert(movementSystem.init());

      // Populate the graphics scene
      m_camera = std::shared_ptr<WasdCamera>(
          new WasdCamera( state,
                          80.0f * ((float) M_PI / 180.0f),  // fovy
                          0.1f,  // near
                          1000.0f,  // far
                          glm::vec3(0.0f, -3.f, 0.0f),  // position
                          glm::angleAxis(
                              (float) M_PI * 0.5f,
                              glm::vec3(1.0f, 0.0f, 0.0f))
          ));
      m_pyrBottom = std::shared_ptr<MeshObject>(
          new MeshObject(state, "assets/models/pyramid_bottom.dae", "assets/textures/pyramid_bottom.png"));
      m_pyrTop = std::shared_ptr<MeshObject>(
          new MeshObject(state, "assets/models/pyramid_top.dae", "assets/textures/pyramid_top.png"));
      m_pyrThrusters = std::shared_ptr<MeshObject>(
          new MeshObject(state, "assets/models/pyramid_thrusters.dae", "assets/textures/thrusters.png"));
      m_pyrFire = std::shared_ptr<MeshObject>(
          new MeshObject(state, "assets/models/pyramid_thruster_flames.dae", "assets/textures/pyramid_flames.png",
                         {0.f, 0.f, -0.85f},
                         glm::angleAxis((float) M_PI, glm::vec3(1.0f, 0.0f, 0.0f)),
                         {0.105f, 0.105f, 0.25f}));
      m_camGimbal = std::shared_ptr<SceneObject>(
          new SceneObject(state));
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

      /*this->scene()->addObject(m_pyrBottom);
      m_pyrBottom->addChild(m_pyrTop);
      m_pyrBottom->addChild(m_pyrThrusters);
      m_pyrBottom->addChild(m_pyrFire);

      entityId fireId = m_pyrFire->getId();
      state.addScalarMultFunc(fireId, DELEGATE_NOCLASS(pyrFireWiggle));

      this->scene()->addObject(m_camera);
      this->setCamera(m_camera);

      entityId topId = m_pyrTop->getId();
      state.addAngularVel(topId, glm::rotate(glm::quat(), 0.1f, {0.f, 0.f, 1.f}));*/

      this->scene()->addObject(m_pyrBottom);
      this->setCamera(m_camera);

      m_pyrBottom->addChild(m_camGimbal);
      m_pyrBottom->addChild(m_pyrTop);
      m_pyrBottom->addChild(m_pyrThrusters);
      m_pyrBottom->addChild(m_pyrFire);

      m_camGimbal->addChild(m_camera);

      entityId gimbalId = m_camGimbal->getId();
      state.addPosition(gimbalId, {0.f, 0.f, 1.f});
      state.addOrientation(gimbalId, glm::quat());
      state.addMouseControls(gimbalId, false, false);

      entityId bottomId = m_pyrBottom->getId();
      state.addLinearVel(bottomId, glm::vec3());
      state.addWasdControls(bottomId, gimbalId, WasdControls::ROTATE_ABOUT_Z);

      entityId camId = m_camera->getId();
      state.remWasdControls(camId);
      state.remMouseControls(camId);
      state.remLinearVel(camId);

      entityId topId = m_pyrTop->getId();
      state.addAngularVel(topId, glm::rotate(glm::quat(), 0.1f, {0.f, 0.f, 1.f}));

      entityId fireId = m_pyrFire->getId();
      state.addScalarMultFunc(fireId, DELEGATE_NOCLASS(pyrFireWiggle));

      Debug::drawLine(state,
          glm::vec3(10 * delta, 0.f, 4.f),
          glm::vec3(11 * delta, 0.f, 2.f),
          glm::vec3(0.f, 1.f, 0.f));
      Debug::drawLine(state,
          glm::vec3(11 * delta, 0.f, 2.f),
          glm::vec3(12 * delta, 0.f, 4.f),
          glm::vec3(0.f, 1.f, 0.f));

      Debug::drawLine(state,
          glm::vec3(13 * delta, 0.f, 2.f),
          glm::vec3(13 * delta, 0.f, 4.f),
          glm::vec3(0.f, 1.f, 0.f));

      Debug::drawLine(state,
          glm::vec3(14 * delta, 0.f, 2.f),
          glm::vec3(14 * delta, 0.f, 4.f),
          glm::vec3(0.f, 1.f, 0.f));
      Debug::drawLine(state,
          glm::vec3(14 * delta, 0.f, 4.f),
          glm::vec3(15 * delta, 0.f, 3.5f),
          glm::vec3(0.f, 1.f, 0.f));
      Debug::drawLine(state,
          glm::vec3(15 * delta, 0.f, 3.5f),
          glm::vec3(14 * delta, 0.f, 3.f),
          glm::vec3(0.f, 1.f, 0.f));
      Debug::drawLine(state,
          glm::vec3(14 * delta, 0.f, 3.f),
          glm::vec3(15 * delta, 0.f, 2.f),
          glm::vec3(0.f, 1.f, 0.f));

      Debug::drawLine(state,
          glm::vec3(16 * delta, 0.f, 2.f),
          glm::vec3(16 * delta, 0.f, 4.f),
          glm::vec3(0.f, 1.f, 0.f));
      Debug::drawLine(state,
          glm::vec3(16 * delta, 0.f, 2.f),
          glm::vec3(17 * delta, 0.f, 2.f),
          glm::vec3(0.f, 1.f, 0.f));
      Debug::drawLine(state,
          glm::vec3(17 * delta, 0.f, 2.f),
          glm::vec3(17 * delta, 0.f, 4.f),
          glm::vec3(0.f, 1.f, 0.f));

      Debug::drawLine(state,
          glm::vec3(19 * delta, 0.f, 4.f),
          glm::vec3(18 * delta, 0.f, 3.3f),
          glm::vec3(0.f, 1.f, 0.f));
      Debug::drawLine(state,
          glm::vec3(18 * delta, 0.f, 3.3f),
          glm::vec3(19 * delta, 0.f, 2.6f),
          glm::vec3(0.f, 1.f, 0.f));
      Debug::drawLine(state,
          glm::vec3(19 * delta, 0.f, 2.6f),
          glm::vec3(18 * delta, 0.f, 2.f),
          glm::vec3(0.f, 1.f, 0.f));
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
  AnimationDemo *demo = (AnimationDemo *) instance;
  float dt = demo->mainLoop(demo->systemsHandlerDlgt, <#initializer#>);
  demo->tick(dt);
}

int main(int argc, char **argv) {
  AnimationDemo demo(argc, argv);
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
