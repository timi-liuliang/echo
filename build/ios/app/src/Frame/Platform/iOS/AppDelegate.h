#import "IOSViewController.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate>
{
@private
    UIWindow* m_window;
}

@property (retain, nonatomic) IOSViewController*     m_viewController;       // 游戏控制台OpenGLES
+ (AppDelegate*)GetDelegate;

@end

