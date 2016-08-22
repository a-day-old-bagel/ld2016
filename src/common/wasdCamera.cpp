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

#include "wasdCamera.h"

#define CAMERA_MAX_VELOCITY 0.00001f
#define CAMERA_ACCELERATION 0.0001f
#define CAMERA_FRICTION_INVERSE 10000.f     // less is more
#define MOUSE_SENSITIVITY 0.1f

namespace ld2016 {
  WasdCamera::WasdCamera(float fovy, float near, float far,
      const glm::vec3 &position, const glm::quat &orientation)
    : PerspectiveCamera(fovy, near, far, position, orientation),
      m_accel(0.0f), m_vel(0.0f)
  {
  }

#if !SDL_VERSION_ATLEAST(3, 0, 0)
  SDL_Window *grabbedWindow = nullptr;
  SDL_Window *SDL_GetGrabbedWindow() {
    return grabbedWindow;
  }
#endif

  bool WasdCamera::handleEvent(const SDL_Event &event) {
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
#if !SDL_VERSION_ATLEAST(3, 0, 0)
          grabbedWindow = window;
#endif
          SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        break;
      case SDL_MOUSEMOTION:
        {
          SDL_Window *window = SDL_GetGrabbedWindow();
          if (SDL_GetGrabbedWindow() != nullptr) {
            // NOTE: Keep the mouse cursor in the center of the window? Not
            // necessary, since SDL_SetRelativeMouseMode() does it for us.
          }
          if (SDL_GetRelativeMouseMode()) {
            // Rotate the camera orientation according to the mouse motion
            // delta
            // FIXME: These camera controls are not very responsive
            glm::quat rotation;
            this->setOrientation(
                glm::angleAxis(
                  -(float)event.motion.xrel * MOUSE_SENSITIVITY * ((float)M_PI / 180.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f))
                * this->orientation());
            this->setOrientation(
                this->orientation() *
                glm::angleAxis(
                  -(float)event.motion.yrel * MOUSE_SENSITIVITY * ((float)M_PI / 180.0f),
                  glm::vec3(1.0f, 0.0f, 0.0f)));

          }
        }
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.scancode) {
          case SDL_SCANCODE_ESCAPE:
            {
              SDL_Window *window = SDL_GetGrabbedWindow();
              if (window == nullptr)
                break;
              // Release the mouse cursor from the window on escape pressed
              SDL_SetWindowGrab(window, SDL_FALSE);
              #if !SDL_VERSION_ATLEAST(3, 0, 0)
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

  void WasdCamera::tick(float dt) {
      m_accel = glm::vec3();
      handleKeyState(dt);
      updateVelocity(dt);
  }

    template<typename lastKeyCode>
    static bool anyPressed(const Uint8* keyStates, lastKeyCode key) {
        return keyStates[key];
    }
    template<typename firstKeyCode, typename... keyCode>
    static bool anyPressed(const Uint8* keyStates, firstKeyCode firstKey, keyCode... keys) {
        return keyStates[firstKey] || anyPressed(keyStates, keys...);
    }
    void WasdCamera::handleKeyState(float dt) {
        // Get current keyboard state and apply actions accordingly
        const Uint8* keyStates = SDL_GetKeyboardState(NULL);
        #define DO_ON_KEYS(action, keys...) if(anyPressed(keyStates, keys)) { action; }
        DO_ON_KEYS( m_accel += glm::vec3( 0.0f, 0.0f, -1.0f), SDL_SCANCODE_W, SDL_SCANCODE_UP)
        DO_ON_KEYS( m_accel += glm::vec3( 0.0f, 0.0f,  1.0f), SDL_SCANCODE_S, SDL_SCANCODE_DOWN)
        DO_ON_KEYS( m_accel += glm::vec3(-1.0f, 0.0f,  0.0f), SDL_SCANCODE_A, SDL_SCANCODE_LEFT)
        DO_ON_KEYS( m_accel += glm::vec3( 1.0f, 0.0f,  0.0f), SDL_SCANCODE_D, SDL_SCANCODE_RIGHT)
        DO_ON_KEYS( m_accel += glm::vec3( 0.0f,-1.0f,  0.0f), SDL_SCANCODE_LCTRL)
        DO_ON_KEYS( m_accel += glm::vec3( 0.0f, 1.0f,  0.0f), SDL_SCANCODE_SPACE)
        #undef DO_ON_KEYS
    }

    void WasdCamera::updateVelocity(float dt) {
        // Rotate the movement axis to the correct orientation
        m_accel = glm::mat3_cast(this->orientation()) * m_accel;
        if (length(m_accel) > 0.0f) {
            m_accel = CAMERA_ACCELERATION * glm::normalize(m_accel);
        }
        // Update the camera velocity based on the acceleration
        m_vel += dt * m_accel;
        // Cap the camera velocity
        if (glm::length(m_vel) > CAMERA_MAX_VELOCITY) {
            m_vel = CAMERA_MAX_VELOCITY * glm::normalize(m_vel);
        }
        // Apply simple brakes
        m_vel *= std::min(1.f, CAMERA_FRICTION_INVERSE / dt);
        // Apply the velocity to the camera's position
        this->setPosition(this->position() + dt * m_vel);
    }
}
