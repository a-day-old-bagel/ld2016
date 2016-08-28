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

namespace ld2016 {
  WasdCamera::WasdCamera(ecs::State& state, float fovy, float near, float far,
                         const glm::vec3 &position, const glm::quat &orientation)
      : PerspectiveCamera(state, fovy, near, far, position, orientation)
  {
    ecs::CompOpReturn status = state.addLinearVel(id, {0.f, 0.f, 0.f});
    assert(status == ecs::SUCCESS);
    status = state.addMouseControls(id, false, false);
    assert(status == ecs::SUCCESS);
    status = state.addWasdControls(id, 0, ecs::WasdControls::ROTATE_ALL_AXES);
    assert(status == ecs::SUCCESS);
  }
}
