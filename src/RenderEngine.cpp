//
//  RenderEngine.cpp
//  Finda
//
//  Created by Hongjian Zhu on 2022/7/17.
//

#include "RenderEngine.hpp"
#include <cmath>
#include <iostream>
#include <OpenGLES/ES3/gl.h>
#include "SimpleVertexShader.h"
#include "SimpleFragmentShader.h"

struct Vertex {
    float position[2];
    float color[4];
};

// clang-format off
const Vertex vertices[] = {
        {{-0.5, -0.5}, {  1,   0, 0, 1}},
        {{ 0.5, -0.5}, {  0,   1, 0, 1}},
        {{   0,  0.5}, {  0,   0, 1, 1}},
};
// clang-format on

RenderEngine::RenderEngine() {
    glGenRenderbuffers(1, &_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _renderbuffer);
}

void RenderEngine::initialize(unsigned int width, unsigned int height) {
    // create the framebuffer object and attach the color buffer
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderbuffer);

    glViewport(0, 0, width, height);

    _simpleProgram = buildProgram(SimpleVertexShader, SimpleFragmentShader);

    glUseProgram(_simpleProgram);

    // initialize the projection matrix
    applyOrtho(2, 3);

    // initialize rotation animation state
    onRotate(DeviceOrientation::Portrait);
}

void RenderEngine::render() const {
    glClearColor(255.0f / 255.0f, 149.0f / 255.0f, 10.0f / 255.0f, 1.0f); // orange color
    glClear(GL_COLOR_BUFFER_BIT);
    
    applyRotation(_currentAngle);

    auto positionSlot = glGetAttribLocation(_simpleProgram, "position");
    auto colorSlot = glGetAttribLocation(_simpleProgram, "color");
    
    glEnableVertexAttribArray(positionSlot);
    glEnableVertexAttribArray(colorSlot);
    
    GLsizei stride = sizeof(Vertex);
    const GLvoid *positions = &(vertices[0].position[0]);
    const GLvoid *colors = &(vertices[0].color[0]);
    
    glVertexAttribPointer(positionSlot, 2, GL_FLOAT, GL_FALSE, stride, positions);
    glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, colors);
    
    GLsizei vertexCount = sizeof(vertices) / sizeof(Vertex);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    
    glDisableVertexAttribArray(positionSlot);
    glDisableVertexAttribArray(colorSlot);
}

void RenderEngine::updateAnimation(float timeStep) {}

void RenderEngine::onRotate(DeviceOrientation orientation) {
    // float angle = 0;
    // switch (orientation) {
    //     case DeviceOrientation::LandscapeLeft:
    //         angle = 270;
    //         break;
    //     case DeviceOrientation::LandscapeRight:
    //         angle = 90;
    //         break;
    //     case DeviceOrientation::PortraitUpsideDown:
    //         angle = 180;
    //         break;
    // }
    // _currentAngle = angle;
}

void RenderEngine::applyOrtho(float maxX, float maxY) const {
    float a = 1.0f / maxX;
    float b = 1.0f / maxY;
    float ortho[16] = {
        a, 0,  0, 0,
        0, b,  0, 0,
        0, 0, -1, 0,
        0, 0,  0, 1
    };
    GLint projectionUniform = glGetUniformLocation(_simpleProgram, "projection");
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, &ortho[0]);
}

void RenderEngine::applyRotation(float degrees) const {
    float radians = degrees * 3.14159f / 180.0f;
    float s = std::sin(radians);
    float c = std::cos(radians);
    float zRotation[16] = {
         c, s, 0, 0,
        -s, c, 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1,
    };
    GLint modelViewUniform = glGetUniformLocation(_simpleProgram, "modelView");
    glUniformMatrix4fv(modelViewUniform, 1, GL_FALSE, &zRotation[0]);
}

GLuint RenderEngine::buildShader(const char *source, GLenum type) const {
    auto handle = glCreateShader(type);
    glShaderSource(handle, 1, &source, nullptr);
    glCompileShader(handle);
    GLint compileStatus;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        GLchar message[512];
        glGetShaderInfoLog(handle, sizeof(message), nullptr, message);
        std::cerr << message;
        exit(EXIT_FAILURE);
    }
    return handle;
}

GLuint RenderEngine::buildProgram(const char *vertexShaderSource, const char *fragmentShaderSource) const {
    auto vertexShader = buildShader(vertexShaderSource, GL_VERTEX_SHADER);
    auto fragmentShader = buildShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    auto handle = glCreateProgram();
    glAttachShader(handle, vertexShader);
    glAttachShader(handle, fragmentShader);
    glLinkProgram(handle);
    GLint linkStatus;
    glGetProgramiv(handle, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE) {
        GLchar message[512];
        glGetProgramInfoLog(handle, sizeof(message), nullptr, message);
        std::cerr << message;
        exit(EXIT_FAILURE);
    }
    return handle;
}

IRenderEngine *createRenderer() {
    return new RenderEngine();
}
