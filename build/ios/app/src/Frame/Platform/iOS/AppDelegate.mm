#import "AppDelegate.h"
#include "Foundation/Base/TypeDef.h"
#include <pthread.h>
#include <mach/thread_act.h>

void makeContextCurrent()
{
    [[AppDelegate GetDelegate].m_viewController MakeCurrent];
}

void PresentRenderBuffer()
{
    [[AppDelegate GetDelegate].m_viewController SwapBuffers];
}

void IOSSetThreadAffinityMask(LORD::ui64 mask)
{
    float osVersion = [[[UIDevice currentDevice] systemVersion] floatValue];
    if (osVersion >= 8)
    {
        thread_affinity_policy ap;
        ap.affinity_tag = static_cast<integer_t>(mask);
        thread_policy_set(pthread_mach_thread_np(pthread_self()), THREAD_AFFINITY_POLICY, (integer_t*)&ap, THREAD_AFFINITY_POLICY_COUNT);
    }
}

@implementation AppDelegate

@synthesize m_viewController = _viewController;

- (void) applicationDidFinishLaunching: (UIApplication*) application
{
    CGRect screenBounds = [[UIScreen mainScreen] bounds];
    
    m_window = [[UIWindow alloc] initWithFrame: screenBounds];
    self.m_viewController = [[IOSViewController alloc] initWithNibName:nil bundle:nil];
    [m_window setRootViewController : self.m_viewController];
    [m_window makeKeyAndVisible];
}

- (void) dealloc
{
    //[m_viewController   release];
    [m_window release];
    [super    dealloc];
}

+ (AppDelegate*)GetDelegate
{
    return (AppDelegate*)[UIApplication sharedApplication].delegate;
}

@end
