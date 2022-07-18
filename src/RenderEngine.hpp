//
//  RenderEngine.hpp
//  Finda
//
//  Created by Hongjian Zhu on 2022/7/17.
//

#pragma once

#include <OpenGLES/gltypes.h>
#include "IRenderEngine.hpp"

class RenderEngine : public IRenderEngine {
public:
    RenderEngine();

    void initialize(unsigned int width, unsigned int height) override;

    void render() const override;

    void updateAnimation(float timeStep) override;

    void onRotate(DeviceOrientation orientation) override;

private:
    void applyOrtho() const;

    void applyRotation(float degrees) const;

    GLuint buildShader(const char *source, GLenum type) const;

    GLuint buildProgram(const char *vertexShaderSource, const char *fragmentShaderSource) const;

private:
    GLuint _framebuffer;
    GLuint _renderbuffer;
    GLuint _simpleProgram;
    GLuint _vertexBuffer;
    GLuint _indexBuffer;

    float _currentAngle;
};
