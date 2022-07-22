//
//  IRenderEngine.hpp
//  Finda
//
//  Created by Hongjian Zhu on 2022/7/17.
//

#pragma once

#include <glm/glm.hpp>

enum class DeviceOrientation {
    Unknown,
    Portrait,
    PortraitUpsideDown,
    LandscapeLeft,
    LandscapeRight,
    FaceUp,
    FaceDown,
};

// interface to the OpenGL ES renderer; consumed by GLView
struct IRenderEngine {
    virtual void initialize(unsigned int width, unsigned int height) = 0;

    virtual void render() const = 0;

    virtual void updateAnimation(float timeStep) = 0;

    virtual void onRotate(DeviceOrientation orientation) = 0;

    virtual void onFingerUp(const glm::vec2 &location) = 0;

    virtual void onFingerDown(const glm::vec2 &location) = 0;

    virtual void onFingerMove(const glm::vec2 &from, const glm::vec2 &to) = 0;

    virtual ~IRenderEngine() {
    }
};

// create an instance of the renderer and set up various OpenGL state
IRenderEngine *createRenderer();
