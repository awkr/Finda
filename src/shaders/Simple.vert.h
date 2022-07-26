//
//  SimpleVertShader.h
//  Finda
//
//  Created by Hongjian Zhu on 2022/7/17.
//

#pragma once

#include "Defines.h"

const char *SimpleVertexShader = STRINGIFY(

    attribute vec3 position;
    attribute vec4 color;
    varying lowp vec4 destinationColor;
    uniform mat4 projection;
    uniform mat4 modelView;

    void main() {
        gl_Position = projection * modelView * vec4(position, 1);
        destinationColor = color;
    }

);
