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
#include "ecsSystem_controls.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"

#define WASD_ACCELERATION 0.5f //1000.f
#define MOUSE_SENSITIVITY 0.1f

namespace ecs {

  #if !SDL_VERSION_ATLEAST(2, 0, 4)
  SDL_Window *grabbedWindow = nullptr;

  SDL_Window *SDL_GetGrabbedWindow() {
    return grabbedWindow;
  }
  #endif

  ControlSystem::ControlSystem(State *state) : System(state) {

  }
  bool ControlSystem::onInit() {
    return true;
  }
  template<typename lastKeyCode>
  static bool anyPressed(const Uint8 *keyStates, lastKeyCode key) {
    return keyStates[key];
  }
  template<typename firstKeyCode, typename... keyCode>
  static bool anyPressed(const Uint8 *keyStates, firstKeyCode firstKey, keyCode... keys) {
    return keyStates[firstKey] || anyPressed(keyStates, keys...);
  }
  void ControlSystem::onTick(float dt) {
    for (auto id : registries[0].ids) {
      MouseControls* mouseControls;
      state->getMouseControls(id, &mouseControls);
      Orientation* orientation;
      state->getOrientation(id, &orientation);

      for (auto event : queuedEvents) {
        switch(event.type) {
          case SDL_MOUSEMOTION: {
            SDL_Window *window = SDL_GetGrabbedWindow();
            if (SDL_GetGrabbedWindow() != nullptr) {
              // NOTE: Keep the mouse cursor in the center of the window? Not
              // necessary, since SDL_SetRelativeMouseMode() does it for us.
            }
            if (SDL_GetRelativeMouseMode()) {
              // Rotate object orientation according to the mouse motion
              // delta
              glm::quat rotation;
              orientation->quat = glm::angleAxis(
                  (float)event.motion.xrel * MOUSE_SENSITIVITY * ((float) M_PI / 180.0f) *
                      (mouseControls->invertedX ? 1.f : -1.f),
                  glm::vec3(0.0f, 0.0f, 1.0f)) * orientation->quat;
              orientation->quat = orientation->quat * glm::angleAxis(
                  (float)event.motion.yrel * MOUSE_SENSITIVITY * ((float) M_PI / 180.0f) *
                      (mouseControls->invertedY ? 1.f : -1.f),
                  glm::vec3(1.0f, 0.0f, 0.0f));
            }
            break;
          }
          default:
            break;
        }
      }
    }
    for (auto id : (registries[1].ids)) {
      WasdControls* wasdControls;
      state->getWasdControls(id, &wasdControls);
      Orientation* orientation;
      if (wasdControls->orientationProxy == 0) {
        state->getOrientation(id, &orientation);
      } else {
        CompOpReturn status = state->getOrientation(wasdControls->orientationProxy, &orientation);
        if (status != SUCCESS) {
          continue;
        }
      }

      // zero out acceleration
      wasdControls->accel = glm::vec3();

      // Get current keyboard state and apply actions accordingly
      const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
      #define DO_ON_KEYS(action, ...) if(anyPressed(keyStates, __VA_ARGS__)) { action; }
      DO_ON_KEYS(wasdControls->accel += glm::vec3( 0.0f,  1.0f,  0.0f), SDL_SCANCODE_W, SDL_SCANCODE_UP)
      DO_ON_KEYS(wasdControls->accel += glm::vec3( 0.0f, -1.0f,  0.0f), SDL_SCANCODE_S, SDL_SCANCODE_DOWN)
      DO_ON_KEYS(wasdControls->accel += glm::vec3(-1.0f,  0.0f,  0.0f), SDL_SCANCODE_A, SDL_SCANCODE_LEFT)
      DO_ON_KEYS(wasdControls->accel += glm::vec3( 1.0f,  0.0f,  0.0f), SDL_SCANCODE_D, SDL_SCANCODE_RIGHT)
      DO_ON_KEYS(wasdControls->accel += glm::vec3( 0.0f,  0.0f, -1.0f), SDL_SCANCODE_LCTRL, SDL_SCANCODE_LSHIFT)
      DO_ON_KEYS(wasdControls->accel += glm::vec3( 0.0f,  0.0f,  1.0f), SDL_SCANCODE_SPACE)
      #undef DO_ON_KEYS

      if (length(wasdControls->accel) > 0.0f) {
        glm::quat quat = orientation->getQuat(1.0);
        glm::mat3 rotMat;
        switch (wasdControls->style) {
          case WasdControls::ROTATE_ABOUT_Z: {
            glm::vec3 axis = glm::vec3(0.f, 0.f, 1.f);
            glm::vec3 orth0 = glm::vec3(0.f, 1.0, 0.f);
            glm::vec3 transformed = quat * orth0;
            glm::vec3 projected = transformed - (glm::dot(transformed, axis) * axis);
            projected = glm::normalize(projected);
            float rotZ = acosf(glm::dot(orth0, projected)) * (transformed.x < 0.f ? 1.f : -1.f);
            rotMat = glm::mat3(glm::rotate(rotZ, glm::vec3(0.f, 0.f, 1.f)));
            break;
          }
          case WasdControls::ROTATE_ALL_AXES:
            rotMat = glm::mat3_cast(quat);
          default:
            break;
        }
        // Rotate the movement axis to the correct orientation
        wasdControls->accel = rotMat * wasdControls->accel;
        wasdControls->accel = WASD_ACCELERATION * glm::normalize(wasdControls->accel);
      }
    }
    queuedEvents.clear();
  }

  bool ControlSystem::handleEvent(SDL_Event &event) {
    switch (event.type) {
      case SDL_MOUSEBUTTONDOWN:
        // Make sure the window has grabbed the mouse cursor
        if (SDL_GetGrabbedWindow() == nullptr) {
          // Somehow obtain a pointer for the window
          SDL_Window *window = SDL_GetWindowFromID(event.button.windowID);
          if (window == nullptr)
            break;
          // Grab the mouse cursor
          SDL_SetWindowGrab(window, SDL_TRUE);
          #if (!SDL_VERSION_ATLEAST(2, 0, 4))
            grabbedWindow = window;
          #endif
          SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        break;
      case SDL_MOUSEMOTION: {
        queuedEvents.push_back(event);
      }
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.scancode) {
          case SDL_SCANCODE_ESCAPE: {
            SDL_Window *window = SDL_GetGrabbedWindow();
            if (window == nullptr)
              break;
            // Release the mouse cursor from the window on escape pressed
            SDL_SetWindowGrab(window, SDL_FALSE);
            #if !SDL_VERSION_ATLEAST(2, 0, 4)
            grabbedWindow = nullptr;
            #endif
            SDL_SetRelativeMouseMode(SDL_FALSE);
          }
            break;
          default:
            return false;
        }
        break;
      default:
        return false;
    }
    return true;
  }
}