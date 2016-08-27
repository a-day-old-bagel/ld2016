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
#include "ecsHelpers.h"

namespace ecs {
  #define GEN_CASE(e) case e: return #e
  std::string resolveErrorToString(CompOpReturn err) {
    switch (err) {
      GEN_CASE(SUCCESS);
      GEN_CASE(NONEXISTENT_ENT);
      GEN_CASE(NONEXISTENT_COMP);
      GEN_CASE(REDUNDANT);
      GEN_CASE(PREREQ_FAIL);
      GEN_CASE(DEPEND_FAIL);
      GEN_CASE(MAX_ID_REACHED);
      default:
        return "Unknown Error";
    }
  }
  #undef GEN_CASE

  EcsResult::EcsResult(const char* message /*= ""*/)
      : lineNumber(-1), errCode(SUCCESS), fileName(""), message(message) { }
  EcsResult::EcsResult(const char *fileName, int lineNumber, CompOpReturn errCode, const char *message /*= ""*/)
      : lineNumber(lineNumber), errCode(errCode), fileName(fileName), message(message) { }
  std::string EcsResult::toString() {
    return resolveErrorToString(errCode) + " from ECS in file \'" + fileName + "\' at line " +
        std::to_string(lineNumber) + (message.length() ? " with message: " : "") + message;
  }
}
