#import <UIKit/UIKit.h>
#import <AppDelegate.h>

// 主函数
int main(int argc, char *argv[])
{
    @autoreleasepool
    {
        NSLog(NSStringFromClass([AppDelegate class]));
        
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
