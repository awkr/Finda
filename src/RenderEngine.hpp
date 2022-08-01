//
//  RenderEngine.hpp
//  Finda
//
//  Created by Hongjian Zhu on 2022/7/17.
//

#pragma once

#include <OpenGLES/gltypes.h>
#include "IRenderEngine.hpp"
#include <glm/gtc/quaternion.hpp>
#include <vector>

static const float RevolutionsPerSecond = 1;
static const float AnimationDuration = 0.25f;

struct Vertex {
    glm::vec3 position;
    glm::vec4 color;
};

struct Animation {
    glm::quat start;
    glm::quat end;
    glm::quat current;
    float elapsed; // seconds
    float duration; // seconds
};

class RenderEngine : public IRenderEngine {
public:
    RenderEngine();

    void initialize(unsigned int width, unsigned int height) override;

    void render() const override;

    void updateAnimation(float timeStep) override;

    void onRotate(DeviceOrientation orientation) override;

    void onFingerUp(const glm::vec2 &location) override;

    void onFingerDown(const glm::vec2 &location) override;

    void onFingerMove(const glm::vec2 &from, const glm::vec2 &to) override;

private:
    GLuint buildShader(const char *source, GLenum type) const;

    GLuint buildProgram(const char *vertexShaderSource, const char *fragmentShaderSource) const;

private:
    GLuint _framebuffer;
    GLuint _colorRenderbuffer;
    GLuint _depthRenderbuffer;
    GLuint _simpleProgram;
    GLuint _vertexBuffer;
    GLuint _indexBuffer;

    float _desiredAngle;
    float _currentAngle;
    float _scale;

    uint32_t _bodyIndexCount;
    uint32_t _diskIndexCount;
    Animation _animation;
};
