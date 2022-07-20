//
//  SimpleFragmentShader.h
//  Finda
//
//  Created by Hongjian Zhu on 2022/7/17.
//

#pragma once

#include "Defines.h"

const char *SimpleFragmentShader = STRINGIFY(

    varying lowp vec4 destinationColor;

    void main() {
        gl_FragColor = destinationColor;
    }

);
