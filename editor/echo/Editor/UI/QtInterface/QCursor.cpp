#include <QCursor>
#include <engine/core/math/Vector2.h>

namespace Echo
{
	Vector2 qCursorPos()
	{
		QPoint cursorPoint = QCursor::pos();
		return Vector2( cursorPoint.x(), cursorPoint.y());
	}
}

