//
//  RenderEngine.cpp
//  Finda
//
//  Created by Hongjian Zhu on 2022/7/17.
//

#include "RenderEngine.hpp"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <OpenGLES/ES3/gl.h>
#include "SimpleVertexShader.h"
#include "SimpleFragmentShader.h"

struct Vertex {
    glm::vec2 position;
    glm::vec4 color;
};

// clang-format off
const Vertex vertices[3] = {
        {{-0.5, -0.5}, {  1,   0, 0, 1}},
        {{ 0.5, -0.5}, {  0,   1, 0, 1}},
        {{   0,  0.5}, {  0,   0, 1, 1}},
};

GLubyte indices[3] = {0, 1, 2};
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

    // create VBO for the vertices
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // create VBO for the indices
    glGenBuffers(1, &_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glViewport(0, 0, width, height);

    _simpleProgram = buildProgram(SimpleVertexShader, SimpleFragmentShader);

    glUseProgram(_simpleProgram);

    // initialize the projection matrix
    applyOrtho();

    // initialize rotation animation state
    onRotate(DeviceOrientation::Portrait);
    _currentAngle = _desiredAngle;
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
    glVertexAttribPointer(positionSlot, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid *) offsetof(Vertex, position));
    glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid *) offsetof(Vertex, color));

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, nullptr);

    glDisableVertexAttribArray(positionSlot);
    glDisableVertexAttribArray(colorSlot);
}

void RenderEngine::updateAnimation(float timeStep) {
    float direction = rotationDirection();
    if (direction == 0) {
        return;
    }

    float degrees = timeStep * 360 * RevolutionsPerSecond;
    _currentAngle += degrees * direction;

    // ensure the angle stays within [0, 360)
    if (_currentAngle >= 360) {
        _currentAngle -= 360;
    } else if (_currentAngle < 0) {
        _currentAngle += 360;
    }

    // if the rotation direction changed, then we overshot the desired angle
    if (rotationDirection() != direction) {
        _currentAngle = _desiredAngle;
    }
}

void RenderEngine::onRotate(DeviceOrientation orientation) {
    float angle = 0;
    switch (orientation) {
        case DeviceOrientation::LandscapeLeft:
            angle = 270;
            break;
        case DeviceOrientation::LandscapeRight:
            angle = 90;
            break;
        case DeviceOrientation::PortraitUpsideDown:
            angle = 180;
            break;
    }
    _desiredAngle = angle;
}

void RenderEngine::applyOrtho() const {
    glm::mat4 ortho = glm::ortho(-1, 1, -1, 1, -1, 1);
    std::cout << glm::to_string(ortho) << std::endl;
    GLint projectionUniform = glGetUniformLocation(_simpleProgram, "projection");
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, glm::value_ptr(ortho));
}

void RenderEngine::applyRotation(float degrees) const {
    auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(degrees), glm::vec3(0, 0, 1));
    GLint modelViewUniform = glGetUniformLocation(_simpleProgram, "modelView");
    glUniformMatrix4fv(modelViewUniform, 1, GL_FALSE, glm::value_ptr(rotation));
}

int8_t RenderEngine::rotationDirection() const {
    float delta = _desiredAngle - _currentAngle;
    if (delta == 0) {
        return 0;
    }
    bool counterclockwise = ((delta > 0 && delta <= 180) || (delta < -180));
    return counterclockwise ? 1 : -1;
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
