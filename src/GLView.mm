//
//  GLView.m
//  Finda
//
//  Created by Hongjian Zhu on 2022/7/17.
//

#import "GLView.h"
#import <OpenGLES/ES3/gl.h>
#import "RenderEngine.hpp"

@implementation GLView {
    EAGLContext *_context;
    IRenderEngine *_renderEngine;
}

- (id)initWithCoder:(NSCoder *)coder {
    if (self = [super initWithCoder:coder]) {
        [self setup:self.frame];
        [self drawView:nil];
    }
    return self;
}

- (void)setup:(CGRect)frame {
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *) self.layer;
    eaglLayer.opaque = YES;
    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    assert(_context && [EAGLContext setCurrentContext:_context]);

    _renderEngine = createRenderer();

    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];

    _renderEngine->initialize(CGRectGetWidth(frame), CGRectGetHeight(frame));

    CADisplayLink *displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];

    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didRotate:) name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)dealloc {
    if ([EAGLContext currentContext] == _context) {
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)drawView:(CADisplayLink *)displayLink {
    _renderEngine->render();
    [_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)didRotate:(NSNotification *)notification {
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    _renderEngine->onRotate((DeviceOrientation) orientation);
    [self drawView:nil];
}

+ (Class)layerClass {
    return [CAEAGLLayer class];
}

@end
