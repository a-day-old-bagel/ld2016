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

#include <glm/gtc/matrix_transform.hpp>

#include "transformRAII.h"

#include "sceneObject.h"

namespace ld2016 {
  SceneObject::SceneObject(ecs::State& state) : state(&state) {
    ecs::CompOpReturn status;
    status = this->state->createEntity(&id);
    assert(status == ecs::SUCCESS);
  }
  SceneObject::~SceneObject() {
    ecs::CompOpReturn status;
    status = state->deleteEntity(id);
    assert(status == ecs::SUCCESS);
  }
  void SceneObject::addChild(std::shared_ptr<SceneObject> child) {
    m_children.insert({child.get(), child});
    child.get()->m_parent = this;
  }
  void SceneObject::removeChild(const SceneObject *address) {
    auto iterator = m_children.find(address);
    assert(iterator != m_children.end());
    // TODO: Set the m_parent member of this child to nullptr (SceneObjects
    // do not have m_parent members at the time of this writing).
    m_children.erase(iterator);
  }
  bool SceneObject::hasChild(const SceneObject *address) {
    return m_children.find(address) != m_children.end();
  }

  void SceneObject::m_draw(Transform &modelWorld, const glm::mat4 &worldView,
      const glm::mat4 &projection, float alpha, bool debug)
  {
    TransformRAII mw(modelWorld);

    ecs::Existence* existence;
    ecs::CompOpReturn status = state->getExistence(id, &existence);
    assert(status == ecs::SUCCESS);

    if (existence->flagIsOn(ecs::ENUM_Position)) {
      ecs::Position *position;
      state->getPosition(id, &position);
      // Translate the object into position
      mw *= glm::translate(glm::mat4(), position->getVec(alpha));
    }
    if (existence->flagIsOn(ecs::ENUM_Orientation)) {
      ecs::Orientation *orientation;
      state->getOrientation(id, &orientation);
      // Apply the object orientation as a rotation
      mw *= glm::mat4_cast(orientation->getQuat(alpha));
    }

    // Delegate the actual drawing to derived classes
    this->draw(mw.peek(), worldView, projection, alpha, debug);

    // Draw our children
    for (auto child : m_children) {
      child.second->m_draw(mw, worldView, projection, alpha, debug);
    }
  }

  void SceneObject::reverseTransformLookup(glm::mat4 &wv, float alpha) const {

    ecs::Existence* existence;
    ecs::CompOpReturn status = state->getExistence(id, &existence);

    if (existence->flagIsOn(ecs::ENUM_Orientation)) {
      ecs::Orientation *orientation;
      state->getOrientation(id, &orientation);
      wv *= glm::mat4_cast(glm::inverse(orientation->getQuat(alpha)));
    }
    if (existence->flagIsOn(ecs::ENUM_Position)) {
      ecs::Position *position;
      state->getPosition(id, &position);
      wv *= glm::translate(glm::mat4(), -1.f * position->getVec(alpha));
    }

    if (m_parent != NULL) {
      m_parent->reverseTransformLookup(wv, alpha);
    }

  }

  bool SceneObject::handleEvent(const SDL_Event &event) { return false; }
  void
  SceneObject::draw(const glm::mat4 &modelWorld, const glm::mat4 &worldView, const glm::mat4 &projection, float alpha,
                    bool debug) { }
  ecs::entityId SceneObject::getId() const {
    return id;
  }
}
