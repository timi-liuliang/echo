#include "Light.h"

namespace Echo
{
	// 获取类型
	const char* Light::getType()
	{
		switch (m_type)
		{
		case LT_Ambient:		return "Ambient";
		case LT_Directional:	return "Directional";
		case LT_Point:			return "Point";
		case LT_Spot:			return "Spot";
		case LT_PBRLight:		return "PBRLight";
		default:				return "Unknown";
		}
	}
}