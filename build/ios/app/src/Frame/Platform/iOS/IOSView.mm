#import "IOSView.h"
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIGeometry.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

@implementation IOSView


+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        CGRect layerBounds = frame;
        layerBounds.size.width = frame.size.width * [UIScreen mainScreen].scale;
        layerBounds.size.height = frame.size.height * [UIScreen mainScreen].scale;
        CAEAGLLayer* eagl_layer = (CAEAGLLayer*)self.layer;
        [eagl_layer setBounds:layerBounds];
        eagl_layer.opacity = YES;
        NSMutableDictionary* dict = [NSMutableDictionary dictionary];
        [dict setValue:[NSNumber numberWithBool:NO] forKey:kEAGLDrawablePropertyRetainedBacking];
        [dict setValue:kEAGLColorFormatRGBA8 forKey:kEAGLDrawablePropertyColorFormat];
        eagl_layer.drawableProperties = dict;
    }
    return self;
}

- (bool)CreateFrameBuffer
{
    int gl_err = 0;
    
    GLuint render_buffer = 0;
    glGenRenderbuffers(1, &render_buffer);
    gl_err = glGetError();
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
    gl_err = glGetError();
    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer*)self.layer];
    
    GLint screen_width = 0;
    GLint screen_height = 0;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &screen_width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &screen_height);
    
    GLuint depth_buffer = 0;
    glGenRenderbuffers(1, &depth_buffer);
    gl_err = glGetError();
    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, screen_width, screen_height);
    
    GLuint frame_buffer = 0;
    glGenFramebuffers(1, &frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, render_buffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
    gl_err = glGetError();
   
    return true;
}



@end
