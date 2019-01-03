#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// create a QGraphicsScene
	QObject* qGraphicsSceneNew();

	// add widget to QGraphicsScene
	QGraphicsProxyWidget* qGraphicsSceneAddWidget(QObject* scene, QWidget* widget);

	// add line
	QGraphicsItem* qGraphicsSceneAddLine(QObject* scene, float startX, float startY, float endX, float endY, const Color& color);

	// add path
	QGraphicsItem* qGraphicsSceneAddPath(QObject* scene, const vector<Vector2>::type paths, float widthf, const Color& color);

	// add simple text
	QGraphicsItem* qGraphicsSceneAddSimpleText(QObject* scene, const char* txt, const Color& color);

	// add eclipse
	QGraphicsItem* qGraphicsSceneAddEclipse(QObject* scene, float left, float top, float width, float height, const Color& color);

	// remove item
	void qGraphicsSceneRemoveItem(QObject* scene, QGraphicsItem* item);

	// delete item
	void qGraphicsSceneDeleteItem(QObject* scene, QGraphicsItem* item);
}

#endif