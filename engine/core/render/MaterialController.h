#ifndef __ECHO_MATERIALCONTROLLER_H__
#define __ECHO_MATERIALCONTROLLER_H__

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include <engine/core/Memory/MemManager.h>
#include <engine/core/Util/Array.hpp>
#include "render/RenderDef.h"
#include "render/ShaderProgram.h"
#include "MaterialInst.h"

namespace Echo
{
	/**
	 * 材质参数控制器                                                       
	 */
	class MaterialController
	{
		friend class MaterialInst;
	public:
		// 材质控制类型
		enum MATERIAL_CONTROL_TYPE
		{
			MCT_RollLoop = 1,
			MCT_Linear = 2, // 参数线性变化;
			MCT_Bezier = 3, // 参数基于贝塞尔曲线变化;
			MCT_Script = 4, // 参数基于脚本变化;
			MCT_Time   = 5, // 时间

			MCT_None = 10,
		};

		// 控制器绑定纬度枚举;
		enum ValueBindType
		{
			BT_ValueX = 1 << 0,
			BT_ValueY = 1 << 1,
			BT_ValueZ = 1 << 2,
			BT_ValueW = 1 << 3,
			BT_ValueAll = BT_ValueX | BT_ValueY | BT_ValueZ | BT_ValueW,
		};

		// 循环方式枚举;
		enum ValueLoopType
		{
			LT_None = 0, // 0 ~ 1 
			LT_Loop = 1, // 0 ~ 1, 0 ~ 1, 0 ~ 1
			LT_Revert = 2, // 0 ~ 1 ~ 0 ~ 1 ~ 0
		};

		// 迭代方向枚举;
		enum ValueRevertType
		{
			RT_Forward = 0,
			RT_Backward = 1,
		};

	public:
		explicit MaterialController(MaterialInst* ins);
		virtual ~MaterialController();

		// 更新函数
		virtual void Update(ui32 delta);
		virtual void GetPropertyValue(const String& name, String& value);
		virtual bool SetPropertyValue(const String& name,const String& value);
		virtual void GetPropertyList(StringArray& list);
		virtual bool GetPropertyType(const String& name, ShaderParamType& type);

		virtual MaterialController*	Clone(MaterialInst* instance);

		virtual char* getControllerName() const; 

		// 设置控制的参数
		virtual void SetControlUniform(MaterialInst::uniform* uniform){ m_PUniform = uniform;}

		virtual void SetControlUniformName(const String& name){ m_uniformName = name; }

		// 设置控制器的材质实例;
		void SetMaterialInstance(MaterialInst* instance) { m_MateIns = instance; }

		// 获取控制参数
		MaterialInst::uniform*	GetControlUniform(){return m_PUniform;}

		//获取控制器类型
		MATERIAL_CONTROL_TYPE	GetControlType() { return m_controlType;}	

		// 文件打开保存
		void importData(rapidxml::xml_node<>* parentNode);
		void exportData(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* parentNode);

	protected:
		MATERIAL_CONTROL_TYPE		m_controlType;
		MaterialInst*			m_MateIns;
		MaterialInst::uniform*	m_PUniform;
		String						m_uniformName;

	protected:
		StringArray m_propertyList;

		typedef std::map<String, std::function<void(const String&)> > PropertySetHandlerMap;

		// 针对具体类型的SetPropertyImplMap;
		PropertySetHandlerMap m_setPropertyValueImpl;

		typedef std::map<String, std::function<String(void)> > PropertyGetHandlerMap;

		// 针对具体类型的GetPropertyImplMap; 
		PropertyGetHandlerMap m_getPropertyValueImpl;

		typedef Echo::array<std::function<void(ui32)>, ShaderParamType::SPT_MAX> ShaderParamHandler;

		// 针对Uniform类型的Handler函数表;
		ShaderParamHandler m_updateUniformImpls;
	};

	class RollLoopController : public MaterialController
	{
	public:
		explicit RollLoopController(MaterialInst* ins);
		~RollLoopController();

		virtual void Update(ui32 delta);
		virtual void GetPropertyValue(const String & name, String &value);
		virtual bool SetPropertyValue(const String & name,const String & value);
		virtual void GetPropertyList(StringArray& list);
		virtual bool GetPropertyType(const String& name, ShaderParamType& type);

		virtual RollLoopController* Clone(MaterialInst* instance);


		virtual void SetControlUniform(MaterialInst::uniform* uniform);
	protected:
		void initPropertyListMap();

	private:
		void*		m_beginValue;   //初始值
		void*		m_endValue;		//结束值
		void*		m_rollSpeed;	//滚动速度
	};

	class TimeController : public MaterialController
	{
		TimeController(const TimeController& rhs);
		TimeController& operator=(const TimeController& rhs);

	public:
		explicit TimeController(MaterialInst* ins);
		~TimeController();

		virtual void Update(ui32 delta) override;;
		virtual TimeController* Clone(MaterialInst* instance) override;
		virtual char* getControllerName() const override;

	private:
		float m_time;
	};

	// MCT_Linear
	class LinearController : public MaterialController
	{
		LinearController(const LinearController& rhs); 
		LinearController& operator=(const LinearController& rhs); 

	public: 
		explicit LinearController(MaterialInst* instance); 
		~LinearController(); 

	public: 
		virtual void Update(ui32 delta) override; 

		virtual bool SetPropertyValue(const String& name, const String& value) override; 
		virtual void GetPropertyValue(const String& name, String& value) override; 

		virtual void GetPropertyList(StringArray& list) override; 
		virtual bool GetPropertyType(const String& name, ShaderParamType& type) override;

		virtual LinearController* Clone(MaterialInst* instance) override;

		virtual char* getControllerName() const override; 

	private: 
		// for constructor && copy constructor
		void initPropertyListMap(); 

		void initUniformUpdaterFunctionArray(); 
		void initSetPropertyFunctionMap(); 
		void initGetPropertyFunctionMap(); 

		void SetPropertyBegin(const String& begin); 
		String GetPropertyBegin() const; 

		void SetPropertyEnd(const String& end); 
		String GetPropertyEnd() const; 
		
		void SetPropertyDelta(const String& delta); 
		String GetPropertyDelta() const;
		
		void SetPropertyBindType(const String& bindType); 
		String GetPropertyBindType() const; 
		
		void SetPropertyLoopType(const String& loopType); 
		String GetPropertyLoopType() const; 

	private: 
		// uniform modifier.
		void UniformModifierInt(ui32 delta); 
		void UniformModifierFloat(ui32 delta);
		void UniformModifierVector2(ui32 delta);
		void UniformModifierVector3(ui32 delta);
		void UniformModifierVector4(ui32 delta);
		void UniformModifierMatrix4(ui32 delta);
		void UniformModifierTexture(ui32 delta);

	private: 
		float m_begin;				 // 初始值;
		float m_end;				 // 最终值;

		float m_delta;				 // 变动幅度（0.0 ~ 1.0）

		ValueBindType m_bindType;		 // 控制器绑定纬度（默认为ALL）
		ValueLoopType m_loopType;		 // 控制器循环方式（默认为NONE）
		ValueRevertType m_revertType;	 // 控制器前进方向（默认为Forward）
	};

	// MCT_Bezier
	class BezierController : public MaterialController
	{
		BezierController(const BezierController& rhs); 
		BezierController& operator=(const BezierController& rhs); 

	public: 
		explicit BezierController(MaterialInst* instance); 
		~BezierController(); 

	public:
		virtual void Update(ui32 delta) override;

		virtual bool SetPropertyValue(const String& name, const String& value) override;
		virtual void GetPropertyValue(const String& name, String& value) override;

		virtual void GetPropertyList(StringArray& list) override;
		virtual bool GetPropertyType(const String& name, ShaderParamType& type) override;

		virtual BezierController* Clone(MaterialInst* instance) override;
	
		virtual char* getControllerName() const override; 

	private:
		// for constructor && copy constructor
		void initPropertyListMap();

		void initUniformUpdaterFunctionArray();
		void initSetPropertyFunctionMap();
		void initGetPropertyFunctionMap();

		void setPropertyBeginNode(const String& beginNode); 
		String getPropertyBeginNode() const; 

		void setPropertyControlNode1(const String& controlNode1); 
		String getPropertyControlNode1() const; 

		void setPropertyControlNode2(const String& controlNode2); 
		String getPropertyControlNode2() const; 

		void setPropertyEndNode(const String& endNode); 
		String getPropertyEndNode() const; 

		void setPropertyBindType(const String& bindType); 
		String getPropertyBindType() const; 

		void setPropertyLoopType(const String& loopType); 
		String getPropertyLoopType() const; 

	private:
		// uniform modifier.
		void UniformModifierInt(ui32 delta);
		void UniformModifierFloat(ui32 delta);
		void UniformModifierVector2(ui32 delta);
		void UniformModifierVector3(ui32 delta);
		void UniformModifierVector4(ui32 delta);
		void UniformModifierMatrix4(ui32 delta);
		void UniformModifierTexture(ui32 delta);

	private: 
		Vector3 calcBezier3(ui32 delta); 

	private: 
		// for Bezier3
		Vector3 m_beginNode; 
		Vector3 m_controlNode1;
		Vector3 m_controlNode2; 
		Vector3 m_endNode; 

		ValueBindType m_bindType; 
		ValueLoopType m_loopType; 
		ValueRevertType m_revertType; 

	private: 
		float m_bezierAmount; 
	};

	// MCT_Script
	class ScriptController : public MaterialController
	{
		ScriptController(const ScriptController& rhs);
		ScriptController& operator=(const ScriptController& rhs);

	public:
		explicit ScriptController(MaterialInst* instance);
		~ScriptController();

	public:
		virtual void Update(ui32 delta) override;

		virtual bool SetPropertyValue(const String& name, const String& value) override;
		virtual void GetPropertyValue(const String& name, String& value) override;

		virtual void GetPropertyList(StringArray& list) override;
		virtual bool GetPropertyType(const String& name, ShaderParamType& type) override;

		virtual ScriptController* Clone(MaterialInst* instance) override;

		virtual char* getControllerName() const; 

	private:
		// for constructor && copy constructor
		void initPropertyListMap();

	private:
		// 脚本路径;
		String m_scriptPath; 

	private: 
		// Callbacks; 
	};
}

#endif