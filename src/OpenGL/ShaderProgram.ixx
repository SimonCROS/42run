//
// Created by scros on 11/29/25.
//

module;

#include "glad/gl.h"

export module ShaderProgram;
import Utility.SlotSet;

export struct ShaderProgram
{
    SlotSetIndex index;
    SlotSetIndex vertexShaderIdx;
    SlotSetIndex fragmentShaderIdx;
    GLint id;
};
