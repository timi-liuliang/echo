#include "engine/core/geom/AABB.h"
#include "engine/core/Util/StringUtil.h"
#include <sstream>

namespace Echo
{
	const AABB AABB::ZERO(Vector3::ZERO, Vector3::ZERO);

	// 从字符串构造box
	AABB AABB::fromString(const String& val)
	{
		StringArray vec = StringUtil::Split(val);

		if (vec.size() != 6)
		{
			return AABB();
		}
		else
		{
			return AABB(
				Vector3(StringUtil::ParseReal(vec[0]), StringUtil::ParseReal(vec[1]), StringUtil::ParseReal(vec[2])),
				Vector3(StringUtil::ParseReal(vec[3]), StringUtil::ParseReal(vec[4]), StringUtil::ParseReal(vec[5])));
		}
	}

	// 数据转换为字符串格式
	String& AABB::toString(String& s) const
	{
		StringStream stream;
		stream << vMin.x << " " << vMin.y << " " << vMin.z << " " \
			<< vMax.x << " " << vMax.y << " " << vMax.z << " ";
		s.assign(stream.str());
		return s;
	}
}
