//
//  RenderEngine.cpp
//  Finda
//
//  Created by Hongjian Zhu on 2022/7/17.
//

#include "RenderEngine.hpp"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <OpenGLES/ES3/gl.h>
#include "shaders/Simple.vert.h"
#include "shaders/Simple.frag.h"

RenderEngine::RenderEngine() : _scale(1) {
    // create & bind the color buffer so that the called can allocate its space
    glGenRenderbuffers(1, &_colorRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);
}

void RenderEngine::initialize(unsigned int width, unsigned int height) {
    const float coneRadius = 0.5f;
    const float coneHeight = 1.866f;
    const int coneSlices = 40;
    const float d = glm::two_pi<float>() / coneSlices;
    const int vertexCount = coneSlices * 2 + 1;

    // Vertex generation

    std::vector<Vertex> coneVertices(vertexCount);
    std::vector<Vertex>::iterator vertex = coneVertices.begin();

    // Cone's body
    for (float theta = 0; vertex != coneVertices.end() - 1; theta += d) {
        // Grayscale gradient
        float brightness = abs(sin(theta));
        glm::vec4 color(brightness, brightness, brightness, 1);

        // Apex vertex
        vertex->position = {0, 1, 0};
        vertex->color = color;
        vertex++;

        // Rim vertex
        vertex->position = {coneRadius * cos(theta), 1 - coneHeight, coneRadius * sin(theta)};
        vertex->color = color;
        vertex++;
    }

    // Disk center
    vertex->position = {0, 1 - coneHeight, 0};
    vertex->color = {0.75, 0.75, 0.75, 1};

    // Index generation

    _bodyIndexCount = coneSlices * 3;
    _diskIndexCount = coneSlices * 3;

    std::vector<GLubyte> coneIndices(_bodyIndexCount + _diskIndexCount);
    std::vector<GLubyte>::iterator index = coneIndices.begin();

    // Body triangles
    for (uint8_t i = 0; i < coneSlices * 2; i += 2) {
        *index++ = i;
        *index++ = (i + 1) % (2 * coneSlices);
        *index++ = (i + 3) % (2 * coneSlices);
    }

    // Disk triangles
    const int diskCenterIndex = vertexCount - 1;
    for (uint8_t i = 1; i < coneSlices * 2 + 1; i += 2) {
        *index++ = diskCenterIndex;
        *index++ = i;
        *index++ = (i + 2) % (2 * coneSlices);
    }

    // create the depth buffer
    glGenRenderbuffers(1, &_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);

    // create the framebuffer object; attach the color and depth buffers
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderbuffer);

    // bind the color buffer for rendering
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer);

    // set up some GL states
    glViewport(0, 0, width, height); // 414 896
    glEnable(GL_DEPTH_TEST);

    // build the GLSL program
    _simpleProgram = buildProgram(SimpleVertexShader, SimpleFragmentShader);

    glUseProgram(_simpleProgram);

    // set the projection matrix
    auto projectionUniform = glGetUniformLocation(_simpleProgram, "projection");
    auto projectionMatrix = glm::frustum(-2.07f * .5f, 2.07f * .5f, -4.48f * .5f, 4.48f * .5f, 5.0f, 10.0f);
    glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Create the VBO for the vertices
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, coneVertices.size() * sizeof(Vertex), coneVertices.data(), GL_STATIC_DRAW);

    // Create the VBO for the indices
    glGenBuffers(1, &_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, coneIndices.size() * sizeof(GLubyte), coneIndices.data(), GL_STATIC_DRAW);
}

void RenderEngine::render() const {
    glClearColor(255.0f / 255.0f, 149.0f / 255.0f, 10.0f / 255.0f, 1.0f); // orange color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto positionSlot = (GLuint) glGetAttribLocation(_simpleProgram, "position");
    auto colorSlot = (GLuint) glGetAttribLocation(_simpleProgram, "color");

    glEnableVertexAttribArray(positionSlot);
    glEnableVertexAttribArray(colorSlot);

    // set the model-view matrix

    auto translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -7.0f));
    auto rotation = glm::mat4_cast(_animation.current);
    auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f) * _scale);
    auto modelViewMatrix = translation * rotation * scale;

    auto modelViewUniform = glGetUniformLocation(_simpleProgram, "modelView");
    glUniformMatrix4fv(modelViewUniform, 1, GL_FALSE, glm::value_ptr(modelViewMatrix));

    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    GLsizei stride = sizeof(Vertex);
    glVertexAttribPointer(positionSlot, 3, GL_FLOAT, GL_FALSE, stride, nullptr);
    glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, stride, (GLvoid *) offsetof(Vertex, color));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBuffer);

    const GLvoid *bodyOffset = 0;
    const auto diskOffset = (GLvoid *) (size_t) _bodyIndexCount;
    glDrawElements(GL_TRIANGLES, _bodyIndexCount, GL_UNSIGNED_BYTE, bodyOffset);
    glDrawElements(GL_TRIANGLES, _diskIndexCount, GL_UNSIGNED_BYTE, diskOffset);

    glDisableVertexAttribArray(positionSlot);
    glDisableVertexAttribArray(colorSlot);
}

void RenderEngine::updateAnimation(float timeStep) {
    if (_animation.current == _animation.end) {
        return;
    }

    _animation.elapsed += timeStep;
    if (_animation.elapsed >= AnimationDuration) {
        _animation.current = _animation.end;
    } else {
        float factor = _animation.elapsed / AnimationDuration;
        _animation.current = glm::slerp(_animation.start, _animation.end, factor);
    }
}

void RenderEngine::onRotate(DeviceOrientation orientation) {
    glm::vec3 direction;
    switch (orientation) {
        case DeviceOrientation::Unknown:
        case DeviceOrientation::Portrait:
            direction = glm::vec3(0, 1, 0);
            break;

        case DeviceOrientation::PortraitUpsideDown:
            direction = glm::vec3(0, -1, 0);
            break;

        case DeviceOrientation::FaceDown:
            direction = glm::vec3(0, 0, -1);
            break;

        case DeviceOrientation::FaceUp: {
            direction = glm::vec3(0, 0, 1);
        }
            break;

        case DeviceOrientation::LandscapeLeft:
            direction = glm::vec3(1, 0, 0);
            break;

        case DeviceOrientation::LandscapeRight:
            direction = glm::vec3(-1, 0, 0);
            break;
    }

    _animation.elapsed = 0;
    _animation.start = _animation.current = _animation.end;
    _animation.end = glm::rotation({0, 1, 0}, direction);
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

void RenderEngine::onFingerUp(const glm::vec2 &location) {
    _scale = 1;
}

void RenderEngine::onFingerDown(const glm::vec2 &location) {
    _scale = 1.05;
}

void RenderEngine::onFingerMove(const glm::vec2 &from, const glm::vec2 &to) {
}

IRenderEngine *createRenderer() {
    return new RenderEngine();
}
