#include "Color.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	// predefined specific vectors
	const Color Color::INVALID(Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL);
	const Color Color::RED(1, 0, 0, 1);
	const Color Color::GREEN(0, 1, 0, 1);
	const Color Color::BLUE(0, 0, 1, 1);
	const Color Color::BLACK(0, 0, 0, 1);
	const Color Color::WHITE(1, 1, 1, 1);
	const Color Color::YELLOW(1, 1, 0, 1);
	const Color Color::CYAN(0, 1, 1, 1);
	const Color Color::PURPLE(1, 0, 1, 1);
	const Color Color::GRAY(0.62745F, 0.62745F, 0.62745F, 1);

	namespace StringUtil
	{
		Color ParseColor(const String& val)
		{
			StringArray vec = StringUtil::Split(val);

			if (vec.size() != 4)
			{
				return Color::BLACK;
			}
			else
			{
				return Color(StringUtil::ParseReal(vec[0]), StringUtil::ParseReal(vec[1]), StringUtil::ParseReal(vec[2]), StringUtil::ParseReal(vec[3]));
			}
		}

		String ToString(const Color& val)
		{
			StringStream stream;
			stream << val.r << " " << val.g << " " << val.b << " " \
				<< val.a << " ";
			
			return stream.str();
		}

		Color fromString(const String& val)
		{
			StringArray vec = StringUtil::Split(val);

			if (vec.size() != 4)
			{
				return Color::BLACK;
			}
			else
			{
				return Color(StringUtil::ParseReal(vec[0]), StringUtil::ParseReal(vec[1]), StringUtil::ParseReal(vec[2]), StringUtil::ParseReal(vec[3]));
			}
		}

	}
}
