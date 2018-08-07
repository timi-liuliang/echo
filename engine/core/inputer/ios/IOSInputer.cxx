/*
#if defined( __APPLE_CC__) && (LORD_PLATFORM == LORD_PLATFORM_MAC_IOS)

#include <stdio.h>
#include <vector>

#ifdef __OBJC__
#import <UIKit/UIKit.h>
#import <UIKit/UIApplication.h>
#import <CoreData/CoreData.h>
#include <CoreFoundation/CoreFoundation.h>
#endif


#include "IOSInputer.h"


std::vector<IOSInputer::TouchInput> IOSInputer::TouchInputStack;
UIViewController*  ViewControllerIns = nil;

IOSInputer::IOSInputer( )
{
    MessageListener = nil;
    MotionManager = nil;
    ReferenceAttitude = nil;
}


void IOSInputer::SetViewController( UIViewController* VC )
{
    ViewControllerIns = VC;
}

void IOSInputer::SetListener( TouchInputListener* listener )
{
    MessageListener = listener;
}

void IOSInputer::Tick( float DeltaTime )
{
}


void ModifyVectorByOrientation(FV3& Vec, bool bIsRotation)
{
    
    UIInterfaceOrientation Orientation = ViewControllerIns !=nil ? [ViewControllerIns interfaceOrientation] : [[UIApplication sharedApplication] statusBarOrientation];
    
    switch (Orientation)
    {
            
        case UIInterfaceOrientationPortraitUpsideDown:
            if (bIsRotation)
            {
                // negate roll and pitch
                Vec.X = -Vec.X;
                Vec.Z = -Vec.Z;
            }
            else
            {
                // negate x/y
                Vec.X = -Vec.X;
                Vec.Y = -Vec.Y;
            }
            break;
            
        case UIInterfaceOrientationLandscapeRight:
            if (bIsRotation)
            {
                // swap and negate (as needed) roll and pitch
                float Temp = Vec.X;
                Vec.X = -Vec.Z;
                Vec.Z = Temp;
            }
            else
            {
                // swap and negate (as needed) x and y
                float Temp = Vec.X;
                Vec.X = -Vec.Y;
                Vec.Y = Temp;
            }
            break;
            
        case UIInterfaceOrientationLandscapeLeft:
            if (bIsRotation)
            {
                // swap and negate (as needed) roll and pitch
                float Temp = Vec.X;
                Vec.X = Vec.Z;
                Vec.Z = -Temp;
            }
            else
            {
                // swap and negate (as needed) x and y
                float Temp = Vec.X;
                Vec.X = Vec.Y;
                Vec.Y = -Temp;
            }
            break;
        default:
            break;
    }
}

void IOSInputer::SendControllerEvents()
{
    //AutoLock(&Locker);
    
    for(size_t i = 0; i < IOSInputer::TouchInputStack.size(); ++i)
    {
        const TouchInput& Touch = IOSInputer::TouchInputStack[i];
        
        // send input to handler
        if (Touch.Type == TouchBegan)
        {
            MessageListener->OnTouchStarted( Touch.Position, Touch.Handle, 0);
        }
        else if (Touch.Type == TouchEnded)
        {
            MessageListener->OnTouchEnded(Touch.Position, Touch.Handle, 0);
        }
        else
        {
            MessageListener->OnTouchMoved(Touch.Position, Touch.Handle, 0);
        }
    }
    
    IOSInputer::TouchInputStack.clear();
    
    
    // Update motion controls.
    FV3 Attitude;
    FV3 RotationRate;
    FV3 Gravity;
    FV3 Acceleration;
    
    GetMovementData(Attitude, RotationRate, Gravity, Acceleration);
    
    
    
    MessageListener->OnMotionDetected(Attitude, RotationRate, Gravity, Acceleration);
}

void IOSInputer::QueueTouchInput(std::vector<IOSInputer::TouchInput> InTouchEvents)
{
    // AutoLock(&Locker);
    
    IOSInputer::TouchInputStack.insert(TouchInputStack.end(), InTouchEvents.begin(),InTouchEvents.end());
}

void IOSInputer::GetMovementData(FV3& Attitude, FV3& RotationRate, FV3& Gravity, FV3& Acceleration)
{
    // initialize on first use
    if (MotionManager == nil)
    {
        // Look to see if we can create the motion manager
        MotionManager = [[CMMotionManager alloc] init];
        
        // Check to see if the device supports full motion (gyro + accelerometer)
        if (MotionManager.deviceMotionAvailable)
        {
            MotionManager.deviceMotionUpdateInterval = 0.02;
            
            // Start the Device updating motion
            [MotionManager startDeviceMotionUpdates];
        }
        else
        {
            [MotionManager startAccelerometerUpdates];
            CenterPitch = CenterPitch = 0;
            bIsCalibrationRequested = false;
        }
    }
    
    // do we have full motion data?
    if (MotionManager.deviceMotionActive)
    {
        // Grab the values
        CMAttitude* CurrentAttitude = MotionManager.deviceMotion.attitude;
        CMRotationRate CurrentRotationRate = MotionManager.deviceMotion.rotationRate;
        CMAcceleration CurrentGravity = MotionManager.deviceMotion.gravity;
        CMAcceleration CurrentUserAcceleration = MotionManager.deviceMotion.userAcceleration;
        
        // apply a reference attitude if we have been calibrated away from default
        if (ReferenceAttitude)
        {
            [CurrentAttitude multiplyByInverseOfAttitude : ReferenceAttitude];
        }
        
        Attitude = FV3(float(CurrentAttitude.pitch), float(CurrentAttitude.yaw), float(CurrentAttitude.roll));
        RotationRate = FV3(float(CurrentRotationRate.x), float(CurrentRotationRate.y), float(CurrentRotationRate.z));
        Gravity = FV3(float(CurrentGravity.x), float(CurrentGravity.y), float(CurrentGravity.z));
        Acceleration = FV3(float(CurrentUserAcceleration.x), float(CurrentUserAcceleration.y), float(CurrentUserAcceleration.z));
    }
    else
    {
        // get the plain accleration
        CMAcceleration RawAcceleration = [MotionManager accelerometerData].acceleration;
        FV3 NewAcceleration(RawAcceleration.x, RawAcceleration.y, RawAcceleration.z);
        
        // storage for keeping the accelerometer values over time (for filtering)
        static bool bFirstAccel = true;
        
        // how much of the previous frame's acceleration to keep
        const float VectorFilter = bFirstAccel ? 0.0f : 0.85f;
        bFirstAccel = false;
        
        // apply new accelerometer values to last frames
        FilteredAccelerometer = FilteredAccelerometer * VectorFilter + (1.0f - VectorFilter) * NewAcceleration;
        
        // create an normalized acceleration vector
        FV3 FinalAcceleration = -FilteredAccelerometer.SafeNormal();
        
        // calculate Roll/Pitch
        float CurrentPitch = atan2(FinalAcceleration.Y, FinalAcceleration.Z);
        float CurrentRoll = -atan2(FinalAcceleration.X, FinalAcceleration.Z);
        
        // if we want to calibrate, use the current values as center
        if (bIsCalibrationRequested)
        {
            CenterPitch = CurrentPitch;
            CenterRoll = CurrentRoll;
            bIsCalibrationRequested = false;
        }
        
        CurrentPitch -= CenterPitch;
        CurrentRoll -= CenterRoll;
        
        Attitude = FV3(CurrentPitch, 0, CurrentRoll);
        RotationRate = FV3(LastPitch - CurrentPitch, 0, LastRoll - CurrentRoll);
        Gravity = FV3(0, 0, 0);
        
        // use the raw acceleration for acceleration
        Acceleration = NewAcceleration;
        
        // remember for next time (for rotation rate)
        LastPitch = CurrentPitch;
        LastRoll = CurrentRoll;
    }
    
    // Fix-up yaw to match directions
    Attitude.Y = -Attitude.Y;
    RotationRate.Y = -RotationRate.Y;
    
    // munge the vectors based on the orientation
    ModifyVectorByOrientation(Attitude, true);
    ModifyVectorByOrientation(RotationRate, true);
    ModifyVectorByOrientation(Gravity, false);
    ModifyVectorByOrientation(Acceleration, false);
}

void IOSInputer::CalibrateMotion()
{
    // If we are using the motion manager, grab a reference frame.  Note, once you set the Attitude Reference frame
    // all additional reference information will come from it
    if (MotionManager.deviceMotionActive)
    {
        ReferenceAttitude = [MotionManager.deviceMotion.attitude retain];
    }
    else
    {
        bIsCalibrationRequested = true;
    }
}

#endif //LORD_PLATFORM == LORD_PLATFORM_MAC_IOS
 */
