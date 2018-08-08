#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface GLViewController : GLKViewController<UIApplicationDelegate,UITextFieldDelegate>

-(void) initialize;
-(void) destroy;
-(void) MakeCurrent;

@property (retain, nonatomic)EAGLContext* context;

@end
