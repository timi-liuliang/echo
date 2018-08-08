#pragma once

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>

@interface IOSView : UIView
{
@public
    EAGLContext*    context;
}

- (bool)CreateFrameBuffer;


@end
