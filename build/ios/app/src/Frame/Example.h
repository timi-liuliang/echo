#pragma once

#include <Foundation/Util/StringUtil.h>
#include <Engine/Object/Root.h>

namespace Examples
{
	/**
	 * 示例基类
	 */
	class Example
	{
	public:
		Example() {}
		virtual ~Example(){}

		// 帮助信息
		virtual const LORD::String getHelp() = 0;

		// 初始化
		virtual bool initialize() = 0;

		// 更新
		virtual void tick(LORD::ui32 elapsedTime) = 0;

		// 销毁
		virtual void destroy() = 0;

		// 按键消息
		virtual void keyboardProc(LORD::ui32 keyChar, bool isKeyDown){}

		// 鼠标消息
		virtual	void mouseWhellProc( LORD::i32 parma ) {}
		virtual void mouseProc( LORD::i32 xpos, LORD::i32 ypos ) {}
		virtual void mouseLBProc( LORD::i32 xpos, LORD::i32 ypos ) {}
		virtual void mouseMoveProc( LORD::i32 xpos, LORD::i32 ypos ) {}

	public:
	
	};

	/**
	 * 示例工厂基类
	 */
	struct ExampleFactory
	{
		LORD::String m_name;			// 示例名称
		LORD::String m_imageSet;		// 标识缩略图使用的纹理

		// 创建示例
		virtual Example* create() = 0;
	};

	extern LORD::vector<ExampleFactory*>::type	g_exampleFactorys;

	/**
	 * 示例工厂模板
	 */
	template<typename T> 
	struct ExampleFactoryT : public Examples::ExampleFactory
	{
		// 构造函数
		ExampleFactoryT( const LORD::String& name, const LORD::String& imageset)
		{
			m_name     = name;
			m_imageSet = imageset;

			Examples::g_exampleFactorys.push_back(this);
		}

		// 初始化
		virtual Example* create()
		{
			return LordNew(T);
		}
	};

	// 借助全局变量注册示例工厂基类,类型，名称，缩略图(imageset)
	#define EXAMPLE_REGISTER(T,N, IS) static ExampleFactoryT<T> G_##T(N, IS)
}

