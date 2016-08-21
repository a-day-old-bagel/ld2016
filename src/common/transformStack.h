/*
 * Copyright (c) 2016 Jonathan Glines
 * Jonathan Glines <jonathan@glines.net>
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

#ifndef LD2016_COMMON_TRANSFORM_STACK_H_
#define LD2016_COMMON_TRANSFORM_STACK_H_

#include <stack>

#include "transform.h"

namespace ld2016 {
  /**
   * Stores a stack of matrix transforms.
   *
   * The TransformStack class is meant to be used in conjuction with the
   * TransformRAII class. TransformStack is the basis of all transforms, since
   * TransformRAII can only be instantiated from an existing instance of a
   * Transform class.
   */
  class TransformStack : public Transform {
    private:
      std::stack<glm::mat4> m_stack;
    public:
      TransformStack();
      ~TransformStack();

      const Transform &operator*=(const glm::mat4 &matrix);

      void push(glm::mat4 matrix);
      void pop();
      const glm::mat4 &peek() const {
        return this->m_stack.top();
      }
      size_t size() { return m_stack.size(); }

    protected:
      Transform *getBase() { return this; }
      void unwind(size_t size);
  };
}

#endif
