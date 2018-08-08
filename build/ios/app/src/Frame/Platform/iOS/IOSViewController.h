#import <UIKit/UIKit.h>

@interface IOSViewController : UIViewController
{
}

-(void)MakeCurrent;
-(void)SwapBuffers;

@property(retain, nonatomic)EAGLContext* context;

@end
