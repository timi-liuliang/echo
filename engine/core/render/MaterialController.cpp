#include <functional>
#include "MaterialController.h"
#include <engine/core/Util/LogManager.h>
#include <engine/core/Math/Curve.h>

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace Echo
{
	// void impl for debug.
	static void UniformModifierUnknown(ui32 delta, const char* controllerName)
	{
		EchoLogError("controller '%s' is try to modify an unknown type uniform.", controllerName); 
	}

	static float modifyValueByLoopType(float inParam, float delta, float from, float to, 
		MaterialController::ValueLoopType m_loopType, MaterialController::ValueRevertType m_revertType)
	{
		if (m_loopType == MaterialController::LT_None)
		{
			float out = inParam + delta;
			return inParam >= to ? to : out >= to ? to : out;
		}

		if (m_loopType == MaterialController::LT_Loop)
		{
			float out = inParam + delta;
			return inParam >= to ? from : out >= to ? to : out;
		}

		if (m_loopType == MaterialController::LT_Revert)
		{
			float out = m_revertType == MaterialController::RT_Forward ? inParam + delta : inParam - delta;

			if (m_revertType == MaterialController::RT_Forward)
			{
				if (inParam >= to)
				{
					m_revertType = MaterialController::RT_Backward;
				}

				return inParam >= to ? to : out >= to ? to : out;
			}

			if (m_revertType == MaterialController::RT_Backward)
			{
				if (inParam <= from)
				{
					m_revertType = MaterialController::RT_Forward;
				}

				return inParam <= from ? from : out <= from ? from : out;
			}
		}

		EchoLogDebug("m_loopType invalid.");
		return 0.0f;
	}

	MaterialController::MaterialController(MaterialInstance* ins)
		: m_MateIns(ins)
		, m_PUniform(NULL)
	{

	}

	MaterialController::~MaterialController()
	{

	}

	void MaterialController::Update( ui32 delta )
	{

	}

	void MaterialController::GetPropertyValue( const String & name, String &value)
	{

	}

	bool MaterialController::SetPropertyValue( const String & name,const String & value )
	{
		return false;
	}

	void MaterialController::GetPropertyList( StringArray& list )
	{

	}

	bool MaterialController::GetPropertyType( const String& name, ShaderParamType& type )
	{
		return false;
	}

	void MaterialController::importData(rapidxml::xml_node<>* parentNode)
	{
		rapidxml::xml_node<> *pPropertyNode = parentNode->first_node();
		while (pPropertyNode)
		{
			rapidxml::xml_attribute<> *pPropertyName = pPropertyNode->first_attribute();
			rapidxml::xml_attribute<> *pPropertyValue = pPropertyName->next_attribute();
			SetPropertyValue(pPropertyName->value(), pPropertyValue->value());

			pPropertyNode = pPropertyNode->next_sibling();
		}
	}

	void MaterialController::exportData(rapidxml::xml_document<>& doc, rapidxml::xml_node<>* parentNode)
	{
		rapidxml::xml_node<>* Controller = doc.allocate_node(rapidxml::node_element, "Controller", NULL);
		
		char* param = doc.allocate_string(m_uniformName.c_str());
		Controller->append_attribute(doc.allocate_attribute("Uniform", param));
		
		param = doc.allocate_string(getControllerName());
		Controller->append_attribute(doc.allocate_attribute("ControllerType", param));

		String propertyValue;
		for (const String& propertyName : m_propertyList)
		{
			rapidxml::xml_node<>* Property = doc.allocate_node(rapidxml::node_element, "Property", NULL);
			GetPropertyValue(propertyName, propertyValue);

			param = doc.allocate_string(propertyName.c_str());
			Property->append_attribute(doc.allocate_attribute("Name", param));

			param = doc.allocate_string(propertyValue.c_str());
			Property->append_attribute(doc.allocate_attribute("Value", param));

			Controller->append_node(Property);
		}
		parentNode->append_node(Controller);
	}

	MaterialController* MaterialController::Clone(MaterialInstance* instance)
	{
		EchoLogError("Trying to clone an unknown type MaterialController."); 
		return NULL; 
	}

	char* MaterialController::getControllerName() const
	{
		return "MaterialController"; 
	}

	RollLoopController::RollLoopController(MaterialInstance* ins)
		: MaterialController(ins)
		, m_beginValue(NULL)
		, m_endValue(NULL)
		, m_rollSpeed(NULL)
	{
		m_controlType = MaterialController::MCT_RollLoop;
	}

	RollLoopController::~RollLoopController()
	{
		if(m_beginValue)
			m_MateIns->delVoid2Value(m_PUniform->type,m_beginValue);
		if(m_endValue)
			m_MateIns->delVoid2Value(m_PUniform->type,m_endValue);
		if(m_rollSpeed)
			m_MateIns->delVoid2Value(m_PUniform->type,m_rollSpeed);
	}

	void RollLoopController::Update( ui32 delta )
	{
		float delta_s =  (float)delta * 0.001f;
		switch(m_PUniform->type)
		{
		case SPT_INT:
			{
				
			}break;
		case SPT_FLOAT:
			{
		
			}break;
		case SPT_VEC2:
			{
				(*(Vector2*)m_PUniform->value).x += (*(Vector2*)m_rollSpeed).x * delta_s;
				if((*(Vector2*)m_PUniform->value).x > (*(Vector2*)m_endValue).x)
				{
					(*(Vector2*)m_PUniform->value).x = (*(Vector2*)m_PUniform->value).x - (*(Vector2*)m_endValue).x + (*(Vector2*)m_beginValue).x;
				}
				else if((*(Vector2*)m_PUniform->value).x < (*(Vector2*)m_beginValue).x)
				{
					(*(Vector2*)m_PUniform->value).x = ((*(Vector2*)m_endValue).x - (*(Vector2*)m_beginValue).x - (*(Vector2*)m_PUniform->value).x);
				}

				(*(Vector2*)m_PUniform->value).y += (*(Vector2*)m_rollSpeed).y * delta_s;
				if((*(Vector2*)m_PUniform->value).y > (*(Vector2*)m_endValue).y)
				{
					(*(Vector2*)m_PUniform->value).y = (*(Vector2*)m_PUniform->value).y - (*(Vector2*)m_endValue).y + (*(Vector2*)m_beginValue).y;
				}
				else if((*(Vector2*)m_PUniform->value).y < (*(Vector2*)m_beginValue).y)
				{
					(*(Vector2*)m_PUniform->value).y = ((*(Vector2*)m_endValue).y - (*(Vector2*)m_beginValue).y - (*(Vector2*)m_PUniform->value).y);
				}

			}break;
		case SPT_VEC3:
			{
				
			};break;
		case SPT_VEC4:
			{
				
			};break;
		case SPT_MAT4:
			{
				//目前矩阵应该都是全局变量
			};break;
		case SPT_TEXTURE:
			{
				
			};break;
		case SPT_UNKNOWN:
		default:
			break;
		}
	}

	void RollLoopController::GetPropertyValue( const String & name, String &value)
	{
		MaterialInstance::uniform param;
		if(name == "beginValue")
		{
			param.type = m_PUniform->type;
			param.value = m_beginValue;
			m_MateIns->void2s(&param,value);
			// return m_beginValue;
		}
		else if(name == "endValue")
		{
			param.type = m_PUniform->type;
			param.value = m_endValue;
			m_MateIns->void2s(&param,value);
			// return m_endValue;
		}
		else if(name == "rollSpeed")
		{
			param.type = m_PUniform->type;
			param.value = m_rollSpeed;
			m_MateIns->void2s(&param,value);
			// return m_rollSpeed;
		}
		else
		{
			// return NULL;
		}
		
		// return NULL;
	}

	bool RollLoopController::SetPropertyValue( const String & name,const String & value )
	{
		if(name == "beginValue")
		{
			m_MateIns->S2Void(m_PUniform->type,value,m_beginValue);
			return true;
		}
		else if(name == "endValue")
		{
			m_MateIns->S2Void(m_PUniform->type,value,m_endValue);
			return true;
		}
		else if(name == "rollSpeed")
		{
			m_MateIns->S2Void(m_PUniform->type,value,m_rollSpeed);
			return true;
		}
		else
		{
			return false;
		}

		return false;
	}


	void RollLoopController::SetControlUniform(MaterialInstance::uniform* uniform )
	{
		MaterialController::SetControlUniform(uniform);

		m_beginValue = m_MateIns->createValue2Void(uniform->type);
		m_endValue	 = m_MateIns->createValue2Void(uniform->type);
		m_rollSpeed  = m_MateIns->createValue2Void(uniform->type);

		if(m_PUniform->type == SPT_VEC2)
		{
			(*(Vector2*)m_beginValue) = Vector2(-1.0,-1.0);
			(*(Vector2*)m_endValue) = Vector2(1.0,1.0);
			(*(Vector2*)m_rollSpeed) = Vector2(0.0,0.0);
			(*(Vector2*)m_PUniform->value) = (*(Vector2*)m_rollSpeed);
		}
		
	}

	void RollLoopController::GetPropertyList( StringArray& list )
	{
		list.clear();
		list.push_back("beginValue");
		list.push_back("endValue");
		list.push_back("rollSpeed");
	}

	bool RollLoopController::GetPropertyType( const String& name, ShaderParamType& type )
	{
		if(name == "beginValue" || name == "endValue" || name == "rollSpeed")
		{
			type = m_PUniform->type;
			return true;
		}
		else
		{
			return false;
		}

		return false;
	}

	RollLoopController* RollLoopController::Clone(MaterialInstance* instance)
	{
		//String _value;
		//_template->GetPropertyValue("beginValue", _value);
		//SetPropertyValue("beginValue", _value);
		//_template->GetPropertyValue("endValue", _value);
		//SetPropertyValue("endValue", _value);
		//_template->GetPropertyValue("rollSpeed", _value);
		//SetPropertyValue("rollSpeed", _value);
		return NULL; 
	}

	//////////////////////////////////////////////////////////////////////////

	LinearController::LinearController(MaterialInstance* instance) 
		: MaterialController(instance)
		, m_begin(0.0f)
		, m_end(0.0f)
		, m_delta(0.0f)
		, m_bindType(BT_ValueAll)
		, m_loopType(LT_None)
	{
		m_controlType = MaterialController::MCT_Linear;

		initPropertyListMap(); 

		initUniformUpdaterFunctionArray(); 
		initSetPropertyFunctionMap(); 
		initGetPropertyFunctionMap(); 
	}

	LinearController::LinearController(const LinearController& rhs)
		: MaterialController(rhs.m_MateIns)
		, m_begin(rhs.m_begin)
		, m_end(rhs.m_end)
		, m_delta(rhs.m_delta)
		, m_bindType(rhs.m_bindType)
		, m_loopType(rhs.m_loopType)
	{
		m_controlType  = rhs.m_controlType; 
		m_propertyList = rhs.m_propertyList;

		initUniformUpdaterFunctionArray();
		initSetPropertyFunctionMap();
		initGetPropertyFunctionMap();
	}

	LinearController::~LinearController()
	{

	}

	void LinearController::Update(ui32 delta)
	{
		if (!m_PUniform)
		{
			EchoLogError("Controller Lost Uniform!"); 
			return; 
		}

		const auto& uniformTpye = m_PUniform->type;

		m_updateUniformImpls[uniformTpye](delta); 
	}

	bool LinearController::SetPropertyValue(const String& name, const String& value)
	{
		auto iter = m_setPropertyValueImpl.find(name); 
		if (iter == m_setPropertyValueImpl.end())
		{
			return false; 
		}

		m_setPropertyValueImpl[name](value); 
		return true; 
	}

	void LinearController::GetPropertyValue(const String& name, String& value)
	{
		auto iter = m_getPropertyValueImpl.find(name); 
		if (iter == m_getPropertyValueImpl.end())
		{
			return; 
		}

		value = m_getPropertyValueImpl[name](); 
	}

	void LinearController::GetPropertyList(StringArray& list)
	{
		list = m_propertyList; 
	}

	bool LinearController::GetPropertyType(const String& name, ShaderParamType& type)
	{
		auto iter = std::find(m_propertyList.begin(), m_propertyList.end(), name); 
		if (iter == m_propertyList.end())
		{
			return false; 
		}

		type = m_PUniform->type; 
		return true; 
	}

	LinearController* LinearController::Clone(MaterialInstance* instance)
	{
		LinearController* other = EchoNew(LinearController(*this)); 
		other->SetMaterialInstance(instance); 

		return other; 
	}

	char* LinearController::getControllerName() const
	{
		return "LinearController"; 
	}

	void LinearController::initPropertyListMap()
	{
		m_propertyList.push_back("begin");
		m_propertyList.push_back("end");
		m_propertyList.push_back("delta");
		m_propertyList.push_back("bindType");
		m_propertyList.push_back("loopType");
	}

	void LinearController::initUniformUpdaterFunctionArray()
	{
		m_updateUniformImpls[SPT_UNKNOWN] = std::bind(&UniformModifierUnknown, std::placeholders::_1, "LinearController"); 
		m_updateUniformImpls[SPT_INT]     = std::bind(&LinearController::UniformModifierInt,     this, std::placeholders::_1); 
		m_updateUniformImpls[SPT_FLOAT]   = std::bind(&LinearController::UniformModifierFloat,   this, std::placeholders::_1);
		m_updateUniformImpls[SPT_VEC2]    = std::bind(&LinearController::UniformModifierVector2, this, std::placeholders::_1);
		m_updateUniformImpls[SPT_VEC3]    = std::bind(&LinearController::UniformModifierVector3, this, std::placeholders::_1);
		m_updateUniformImpls[SPT_VEC4]    = std::bind(&LinearController::UniformModifierVector4, this, std::placeholders::_1);
		m_updateUniformImpls[SPT_MAT4]    = std::bind(&LinearController::UniformModifierMatrix4, this, std::placeholders::_1);
		m_updateUniformImpls[SPT_TEXTURE] = std::bind(&LinearController::UniformModifierTexture, this, std::placeholders::_1);
	}

	void LinearController::initSetPropertyFunctionMap()
	{
		m_setPropertyValueImpl.insert(std::make_pair(
			String("begin"), std::bind(&LinearController::SetPropertyBegin, this, std::placeholders::_1))); 

		m_setPropertyValueImpl.insert(std::make_pair(
			String("end"), std::bind(&LinearController::SetPropertyEnd, this, std::placeholders::_1)));

		m_setPropertyValueImpl.insert(std::make_pair(
			String("delta"), std::bind(&LinearController::SetPropertyDelta, this, std::placeholders::_1)));

		m_setPropertyValueImpl.insert(std::make_pair(
			String("bindType"), std::bind(&LinearController::SetPropertyBindType, this, std::placeholders::_1)));

		m_setPropertyValueImpl.insert(std::make_pair(
			String("loopType"), std::bind(&LinearController::SetPropertyLoopType, this, std::placeholders::_1)));
	}

	void LinearController::initGetPropertyFunctionMap()
	{
		m_getPropertyValueImpl.insert(std::make_pair(
			String("begin"), std::bind(&LinearController::GetPropertyBegin, this))); 

		m_getPropertyValueImpl.insert(std::make_pair(
			String("end"), std::bind(&LinearController::GetPropertyEnd, this)));

		m_getPropertyValueImpl.insert(std::make_pair(
			String("delta"), std::bind(&LinearController::GetPropertyDelta, this)));

		m_getPropertyValueImpl.insert(std::make_pair(
			String("bindType"), std::bind(&LinearController::GetPropertyBindType, this)));

		m_getPropertyValueImpl.insert(std::make_pair(
			String("loopType"), std::bind(&LinearController::GetPropertyLoopType, this)));
	}

	void LinearController::SetPropertyBegin(const String& begin)
	{
		m_begin = StringUtil::ParseFloat(begin); 
	}

	String LinearController::GetPropertyBegin() const 
	{
		return StringUtil::ToString(m_begin); 
	}

	void LinearController::SetPropertyEnd(const String& end)
	{
		m_end = StringUtil::ParseFloat(end); 
	}

	String LinearController::GetPropertyEnd() const
	{
		return StringUtil::ToString(m_end); 
	}

	void LinearController::SetPropertyDelta(const String& delta)
	{
		m_delta = StringUtil::ParseFloat(delta); 
	}

	String LinearController::GetPropertyDelta() const
	{
		return StringUtil::ToString(m_delta); 
	}

	void LinearController::SetPropertyBindType(const String& bindType)
	{
		m_bindType = static_cast<ValueBindType>(StringUtil::ParseInt(bindType)); 
	}

	String LinearController::GetPropertyBindType() const
	{
		return StringUtil::ToString(static_cast<int>(m_bindType)); 
	}

	void LinearController::SetPropertyLoopType(const String& loopType)
	{
		m_loopType = static_cast<ValueLoopType>(StringUtil::ParseInt(loopType)); 
	}

	String LinearController::GetPropertyLoopType() const
	{
		return StringUtil::ToString(static_cast<int>(m_loopType)); 
	}

	void LinearController::UniformModifierInt(ui32 delta)
	{
		ECHO_UNUSED(delta); 

		EchoLogDebug("LinearController::UniformModifierInt, unsupported.");
	}

	void LinearController::UniformModifierFloat(ui32 delta)
	{
		ECHO_UNUSED(delta);

		float& value = *(float*)(m_PUniform->value);

		value = modifyValueByLoopType(value, m_delta, m_begin, m_end, m_loopType, m_revertType);
	}

	void LinearController::UniformModifierVector2(ui32 delta)
	{
		ECHO_UNUSED(delta);

		Vector2& value = *(Vector2*)(m_PUniform->value);

		if (m_bindType | BT_ValueX)
		{
			value.x = modifyValueByLoopType(value.x, m_delta, m_begin, m_end, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueY)
		{
			value.y = modifyValueByLoopType(value.y, m_delta, m_begin, m_end, m_loopType, m_revertType);
		}
	}

	void LinearController::UniformModifierVector3(ui32 delta)
	{
		ECHO_UNUSED(delta);

		Vector3& value = *(Vector3*)(m_PUniform->value);

		if (m_bindType | BT_ValueX)
		{
			value.x = modifyValueByLoopType(value.x, m_delta, m_begin, m_end, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueY)
		{
			value.y = modifyValueByLoopType(value.y, m_delta, m_begin, m_end, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueZ)
		{
			value.z = modifyValueByLoopType(value.z, m_delta, m_begin, m_end, m_loopType, m_revertType);
		}
	}

	void LinearController::UniformModifierVector4(ui32 delta)
	{
		ECHO_UNUSED(delta); 

		Vector4& value = *(Vector4*)(m_PUniform->value); 

		if (m_bindType | BT_ValueX)
		{
			value.x = modifyValueByLoopType(value.x, m_delta, m_begin, m_end, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueY)
		{
			value.y = modifyValueByLoopType(value.y, m_delta, m_begin, m_end, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueZ)
		{
			value.z = modifyValueByLoopType(value.z, m_delta, m_begin, m_end, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueW)
		{
			value.w = modifyValueByLoopType(value.w, m_delta, m_begin, m_end, m_loopType, m_revertType);
		}
	}

	void LinearController::UniformModifierMatrix4(ui32 delta)
	{
		ECHO_UNUSED(delta); 

		EchoLogDebug("LinearController::UniformModifierMatrix4, unsupported."); 
	}

	void LinearController::UniformModifierTexture(ui32 delta)
	{
		ECHO_UNUSED(delta);

		EchoLogDebug("LinearController::UniformModifierTexture, unsupported.");
	}

	//////////////////////////////////////////////////////////////////////////
	BezierController::BezierController(MaterialInstance* instance)
		: MaterialController(instance)
		, m_beginNode(Vector3::ZERO)
		, m_controlNode1(Vector3::ZERO)
		, m_controlNode2(Vector3::ZERO)
		, m_endNode(Vector3::ZERO)
		, m_bindType(BT_ValueAll)
		, m_loopType(LT_None)
		, m_bezierAmount(0.0f)
	{
		m_controlType = MCT_Bezier;

		initPropertyListMap(); 

		initUniformUpdaterFunctionArray(); 
		initSetPropertyFunctionMap(); 
		initGetPropertyFunctionMap(); 
	}

	BezierController::BezierController(const BezierController& rhs)
		: MaterialController(rhs.m_MateIns)
		, m_beginNode(rhs.m_beginNode)
		, m_controlNode1(rhs.m_controlNode1)
		, m_controlNode2(rhs.m_controlNode2)
		, m_endNode(rhs.m_endNode)
		, m_bindType(rhs.m_bindType)
		, m_loopType(rhs.m_loopType)
		, m_bezierAmount(rhs.m_bezierAmount)
		
	{
		m_controlType = rhs.m_controlType;

	}

	BezierController::~BezierController()
	{

	}

	void BezierController::Update(ui32 delta)
	{
		if (!m_PUniform)
		{
			EchoLogError("Controller Lost Uniform!");
			return;
		}

		const auto& uniformTpye = m_PUniform->type;

		m_updateUniformImpls[uniformTpye](delta);
	}

	bool BezierController::SetPropertyValue(const String& name, const String& value)
	{
		if (m_setPropertyValueImpl.find(name) == m_setPropertyValueImpl.end())
		{
			EchoLogDebug("[BezierController]::Property Setter %s Not Found...", name.c_str()); 
			return false; 
		}

		m_setPropertyValueImpl[name](value); 
		return true;
	}
	void BezierController::GetPropertyValue(const String& name, String& value)
	{
		if (m_getPropertyValueImpl.find(name) == m_getPropertyValueImpl.end())
		{
			EchoLogDebug("[BezierController]::Property Getter %s Not Found...", name.c_str()); 
			return; 
		}

		value = m_getPropertyValueImpl[name](); 
		return; 
	}

	void BezierController::GetPropertyList(StringArray& list)
	{
		list = m_propertyList; 
	}

	bool BezierController::GetPropertyType(const String& name, ShaderParamType& type)
	{
		if (std::find(std::begin(m_propertyList), std::end(m_propertyList), name) == m_propertyList.end()) 
		{
			EchoLogDebug("[BezierController]::Property List %s Not Found...", name.c_str()); 
			return false; 
		}

		type = m_PUniform->type;
		return true; 
	}

	BezierController* BezierController::Clone(MaterialInstance* instance)
	{
		BezierController* controller = EchoNew(BezierController(*this)); 
		controller->SetMaterialInstance(instance); 

		return controller;
	}

	char* BezierController::getControllerName() const
	{
		return "BezierController"; 
	}

	// for constructor && copy constructor
	void BezierController::initPropertyListMap()
	{
		m_propertyList.push_back("beginNode"); 
		m_propertyList.push_back("controlNode1"); 
		m_propertyList.push_back("controlNode2"); 
		m_propertyList.push_back("endNode"); 
		m_propertyList.push_back("bindType"); 
		m_propertyList.push_back("loopType"); 
	}

	void BezierController::initUniformUpdaterFunctionArray()
	{
		m_updateUniformImpls[SPT_UNKNOWN] = std::bind(&UniformModifierUnknown, std::placeholders::_1, "BezierController");
		m_updateUniformImpls[SPT_INT]     = std::bind(&BezierController::UniformModifierInt,     this, std::placeholders::_1);
		m_updateUniformImpls[SPT_FLOAT]   = std::bind(&BezierController::UniformModifierFloat,   this, std::placeholders::_1);
		m_updateUniformImpls[SPT_VEC2]    = std::bind(&BezierController::UniformModifierVector2, this, std::placeholders::_1);
		m_updateUniformImpls[SPT_VEC3]    = std::bind(&BezierController::UniformModifierVector3, this, std::placeholders::_1);
		m_updateUniformImpls[SPT_VEC4]    = std::bind(&BezierController::UniformModifierVector4, this, std::placeholders::_1);
		m_updateUniformImpls[SPT_MAT4]    = std::bind(&BezierController::UniformModifierMatrix4, this, std::placeholders::_1);
		m_updateUniformImpls[SPT_TEXTURE] = std::bind(&BezierController::UniformModifierTexture, this, std::placeholders::_1);
	}

	void BezierController::initSetPropertyFunctionMap()
	{
		m_setPropertyValueImpl[String("beginNode")] = std::bind(&BezierController::setPropertyBeginNode, this, std::placeholders::_1); 
		m_setPropertyValueImpl[String("controlNode1")] = std::bind(&BezierController::setPropertyControlNode1, this, std::placeholders::_1); 
		m_setPropertyValueImpl[String("controlNode2")] = std::bind(&BezierController::setPropertyControlNode2, this, std::placeholders::_1); 
		m_setPropertyValueImpl[String("endNode")] = std::bind(&BezierController::setPropertyEndNode, this, std::placeholders::_1);
		m_setPropertyValueImpl[String("bindType")] = std::bind(&BezierController::setPropertyBindType, this, std::placeholders::_1);
		m_setPropertyValueImpl[String("loopType")] = std::bind(&BezierController::setPropertyLoopType, this, std::placeholders::_1);
	}

	void BezierController::initGetPropertyFunctionMap()
	{
		m_getPropertyValueImpl[String("beginNode")] = std::bind(&BezierController::getPropertyBeginNode, this);
		m_getPropertyValueImpl[String("controlNode1")] = std::bind(&BezierController::getPropertyControlNode1, this);
		m_getPropertyValueImpl[String("controlNode2")] = std::bind(&BezierController::getPropertyControlNode2, this);
		m_getPropertyValueImpl[String("endNode")] = std::bind(&BezierController::getPropertyEndNode, this);
		m_getPropertyValueImpl[String("bindType")] = std::bind(&BezierController::getPropertyBindType, this);
		m_getPropertyValueImpl[String("loopType")] = std::bind(&BezierController::getPropertyLoopType, this);
	}



	Vector3 BezierController::calcBezier3(ui32 delta)
	{
		ECHO_UNUSED(delta); 

		m_bezierAmount = modifyValueByLoopType(m_bezierAmount, 0.01f, 0.0f, 1.0f, m_loopType, m_revertType);

		Vector3 result = Vector3::ZERO;

		Bezier3(result, m_beginNode, m_controlNode1, m_controlNode2, m_endNode, m_bezierAmount);

		return result; 
	}

	void BezierController::UniformModifierInt(ui32 delta)
	{
		ECHO_UNUSED(delta); 

		EchoLogDebug("BezierController::UniformModifierInt, unsupported.");
	}

	void BezierController::UniformModifierFloat(ui32 delta)
	{
		Vector3 result = calcBezier3(delta);

		float& value = *((float*)m_PUniform->value); 

		if (m_bindType | BT_ValueX)
		{
			value = modifyValueByLoopType(value, result.x, m_beginNode.x, m_endNode.x, m_loopType, m_revertType);
			return;
		}

		if (m_bindType | BT_ValueY)
		{
			value = modifyValueByLoopType(value, result.y, m_beginNode.y, m_endNode.y, m_loopType, m_revertType);
			return;
		}

		if (m_bindType | BT_ValueZ)
		{
			value = modifyValueByLoopType(value, result.z, m_beginNode.z, m_endNode.z, m_loopType, m_revertType);
			return;
		}

		return; 
	}

	void BezierController::UniformModifierVector2(ui32 delta)
	{
		Vector3 result = calcBezier3(delta);

		Vector2& value = *((Vector2*)m_PUniform->value); 

		if (m_bindType | BT_ValueX)
		{
			value.x = modifyValueByLoopType(value.x, result.x, m_beginNode.x, m_endNode.x, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueY)
		{
			value.y = modifyValueByLoopType(value.y, result.y, m_beginNode.y, m_endNode.y, m_loopType, m_revertType);
		}
	}

	void BezierController::UniformModifierVector3(ui32 delta)
	{
		Vector3 result = calcBezier3(delta);

		Vector3& value = *((Vector3*)m_PUniform->value);

		if (m_bindType | BT_ValueX)
		{
			value.x = modifyValueByLoopType(value.x, result.x, m_beginNode.x, m_endNode.x, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueY)
		{
			value.y = modifyValueByLoopType(value.y, result.y, m_beginNode.y, m_endNode.y, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueZ)
		{
			value.z = modifyValueByLoopType(value.z, result.z, m_beginNode.y, m_endNode.y, m_loopType, m_revertType);
		}
	}

	void BezierController::UniformModifierVector4(ui32 delta)
	{
		Vector3 result = calcBezier3(delta);

		Vector4& value = *((Vector4*)m_PUniform->value);

		if (m_bindType | BT_ValueX)
		{
			value.x = modifyValueByLoopType(value.x, result.x, m_beginNode.x, m_endNode.x, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueY)
		{
			value.y = modifyValueByLoopType(value.y, result.y, m_beginNode.y, m_endNode.y, m_loopType, m_revertType);
		}

		if (m_bindType | BT_ValueZ)
		{
			value.z = modifyValueByLoopType(value.z, result.z, m_beginNode.y, m_endNode.y, m_loopType, m_revertType);
		}
	}

	void BezierController::UniformModifierMatrix4(ui32 delta)
	{
		EchoLogDebug("[BezierController::UniformModifierMatrix4]:: UnSupported"); 
	}

	void BezierController::UniformModifierTexture(ui32 delta)
	{
		EchoLogDebug("[BezierController::UniformModifierTexture]:: UnSupported");
	}

	void BezierController::setPropertyBeginNode(const String& beginNode)
	{
		m_beginNode = StringUtil::ParseVec3(beginNode); 
	}

	void BezierController::setPropertyControlNode1(const String& controlNode1)
	{
		m_controlNode1 = StringUtil::ParseVec3(controlNode1); 
	}

	void BezierController::setPropertyControlNode2(const String& controlNode2)
	{
		m_controlNode2 = StringUtil::ParseVec3(controlNode2); 
	}

	void BezierController::setPropertyEndNode(const String& endNode)
	{
		m_endNode = StringUtil::ParseVec3(endNode); 
	}

	void BezierController::setPropertyBindType(const String& bindType)
	{
		m_bindType = static_cast<ValueBindType>(StringUtil::ParseInt(bindType)); 
	}

	void BezierController::setPropertyLoopType(const String& loopType)
	{
		m_loopType = static_cast<ValueLoopType>(StringUtil::ParseInt(loopType)); 
	}

	String BezierController::getPropertyBeginNode() const
	{
		return StringUtil::ToString(m_beginNode); 
	}

	String BezierController::getPropertyControlNode1() const
	{
		return StringUtil::ToString(m_controlNode1); 
	}

	String BezierController::getPropertyControlNode2() const
	{
		return StringUtil::ToString(m_controlNode2); 
	}

	String BezierController::getPropertyEndNode() const
	{
		return StringUtil::ToString(m_endNode); 
	}

	String BezierController::getPropertyBindType() const
	{
		return StringUtil::ToString(m_bindType); 
	}

	String BezierController::getPropertyLoopType() const
	{
		return StringUtil::ToString(m_loopType); 
	}


	//////////////////////////////////////////////////////////////////////////
	ScriptController::ScriptController(MaterialInstance* instance)
		:MaterialController(instance)
	{
		m_controlType = MCT_Script;
	}

	ScriptController::ScriptController(const ScriptController& rhs)
		: MaterialController(rhs.m_MateIns)
	{
		m_controlType = rhs.m_controlType;
		m_propertyList = rhs.m_propertyList; 

	}

	ScriptController::~ScriptController()
	{

	}

	void ScriptController::Update(ui32 delta) 
	{

	}

	bool ScriptController::SetPropertyValue(const String& name, const String& value) 
	{ 
		return true;  
	}
	void ScriptController::GetPropertyValue(const String& name, String& value) 
	{

	}

	void ScriptController::GetPropertyList(StringArray& list) 
	{

	}

	bool ScriptController::GetPropertyType(const String& name, ShaderParamType& type) 
	{ 
		return true;  
	}

	ScriptController* ScriptController::Clone(MaterialInstance* instance) 
	{ 
		ScriptController* other = EchoNew(ScriptController(*this)); 
		other->SetMaterialInstance(instance); 

		return other; 
	}

	char* ScriptController::getControllerName() const
	{
		return "ScriptController"; 
	}

	TimeController::TimeController(MaterialInstance* ins)
		: MaterialController(ins)
		, m_time(0.0f)
	{
		m_controlType = MCT_Time;
	}

	TimeController::TimeController(const TimeController& rhs)
		: MaterialController(rhs.m_MateIns)
		, m_time(rhs.m_time)
	{
		
	}

	TimeController::~TimeController()
	{

	}

	void TimeController::Update(ui32 delta)
	{
		if (!m_PUniform && m_MateIns)
		{
			m_PUniform = m_MateIns->GetUniform(m_uniformName);
		}


		if (m_PUniform)
		{
			m_time += delta * 0.001f;
			*(float*)m_PUniform->value = m_time;
		}
	}

	TimeController* TimeController::Clone(MaterialInstance* instance)
	{
		TimeController* other = EchoNew(TimeController(*this));
		other->SetMaterialInstance(instance);

		return other;
	}

	char* TimeController::getControllerName() const
	{
		return "TimeController";
	}

}





