//
//  IOSInputer.h
//
//  Created by Fantasy on 14-8-12.
//  Copyright (c) 2014年 Fantasy. All rights reserved.
//
//  !!!编译时需要对cpp文件加标记 ：-fno-objc-arc
//
#ifndef SS_IOSInputer_h
#define SS_IOSInputer_h

#if defined(__APPLE_CC__) && (LORD_PLATFORM == LORD_PLATFORM_MAC_IOS)

#import <CoreMotion/CoreMotion.h>

/* float vector 2D/3D
*********************************************************************************/
struct FV2
{
    float X;
    float Y;
};
struct FV3
{
    float X;
    float Y;
    float Z;
    FV3(){}
    FV3(float x, float y, float z):X(x),Y(y),Z(z){}
    FV3 operator*( float s ) const { return FV3(X*s, Y*s, Z*s); }
    FV3 operator-() const { return FV3(-X, -Y, -Z); }
    FV3 SafeNormal()
    {
        float squareLen = X*X+Y*Y+Z*Z;
        if(squareLen == 1.0f )
        {
            return *this;
        }
        else if( squareLen < 0.00000001f)
        {
            return FV3(0,0,0);
        }
        const float invLen = 1.0f / sqrtf(squareLen);
        return FV3( X*invLen, Y*invLen, Z*invLen);
    }
};
inline FV3 operator*( float s, FV3 const& v ) { return v*s; }
inline FV3 operator+( FV3 const& A, FV3 const& B ) { return FV3( A.X+B.X, A.Y+B.Y, A.Z+B.Z); }








/* Touch Inputer Listener
*********************************************************************************/
class TouchInputListener
{
public:
    virtual ~TouchInputListener(){}
    
    virtual bool OnTouchStarted( const FV2& Location, int TouchIndex, int ControllerId )
    {
        return false;
    }
    
    virtual bool OnTouchMoved( const FV2& Location, int TouchIndex, int ControllerId )
    {
        return false;
    }
    
    virtual bool OnTouchEnded( const FV2& Location, int TouchIndex, int ControllerId )
    {
        return false;
    }
    
    virtual bool OnMotionDetected( const FV3& Tilt, const FV3& RotationRate, const FV3& Gravity, const FV3& Acceleration )
    {
        return false;
    }

};





/* IOS Inputer
*********************************************************************************/
class IOSInputer
{
public:
    // 触控事件类型
    enum TouchType
    {
        TouchBegan,
        TouchMoved,
        TouchEnded,
    };
    
    // 触控输入数据
    struct TouchInput
    {
        int         Handle;
        TouchType   Type;
        FV2         LastPosition;
        FV2         Position;
    };
    
public:
    
    IOSInputer();
    
    virtual ~IOSInputer() {}
    

    void    Tick( float DeltaTime );
    

    // 设置控制器源
    void    SetViewController( UIViewController* VC );
    
    // 设置侦听器
    void    SetListener( TouchInputListener* listener );
    
    // 滚动发送控制消息到侦听对象
    void    SendControllerEvents();
    
    /**
     *  获取当前设备的移动参数
     *
     * @param Attitude      : The current Roll/Pitch/Yaw of the device
     * @param RotationRate  : The current rate of change of the attitude
     * @param Gravity       : A vector that describes the direction of gravity for the device
     * @param Acceleration  : returns the current acceleration of the device
     */
    void    GetMovementData(FV3& Attitude, FV3& RotationRate, FV3& Gravity, FV3& Acceleration);

    
    // 校准设备
    void CalibrateMotion();
    
    // 输入触控事件队列
    static void QueueTouchInput(std::vector<TouchInput> InTouchEvents);
    
private:
    
    static std::vector<TouchInput> TouchInputStack;
    

    // 线程同步互斥对象
    //static ThreadLocker Locker;
    
    // 侦听器
    TouchInputListener* MessageListener;
    
    
private:
    

    // IOS陀螺仪设备访问对象
    CMMotionManager*    MotionManager;
    
    // IOS设备倾斜信息访问对象
    CMAttitude*         ReferenceAttitude;
    

    // 记录最后一帧的横向滚动角，用于计算旋转速度
    float   LastRoll;
    
    // 记录最后一帧的仰/俯角，用于计算旋转速度
    float   LastPitch;
    
    // 是否需要校正仪器
    bool    bIsCalibrationRequested;
    
    // 用于倾斜校正的中心横向滚动角
    float   CenterRoll;
    
    // 用于倾斜校正的中心仰/俯角
    float   CenterPitch;

    
    // 当仅使用加速度时我们只对存储加速度的帧进行过滤
    FV3     FilteredAccelerometer;
};

#endif //LORD_PLATFORM == LORD_PLATFORM_MAC_IOS

#endif
