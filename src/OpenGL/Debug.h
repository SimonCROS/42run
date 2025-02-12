//
// Created by simon on 19/01/2025.
//

#ifndef DEBUG_H
#define DEBUG_H

#include "glad/gl.h"

auto glDebugOutput(GLenum source,
                   GLenum type,
                   unsigned int id,
                   GLenum severity,
                   GLsizei length,
                   const char* message,
                   const void* userParam) -> void;

#endif //DEBUG_H
