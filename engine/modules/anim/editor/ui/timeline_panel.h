#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/base/object.h"

#ifdef ECHO_EDITOR_MODE

#include "timeline_panel.h"
#include "ui_timeline_panel.h"

namespace Echo
{
	class Timeline;
	class TimelinePanel : public QDockWidget, public Ui_TimelinePanel
	{
	public:
		// key info
		struct KeyInfo
		{
			enum class Type
			{
				Int,
				Float,
				String,
			}		m_type;

			float	m_value;
		};

	public:
		TimelinePanel(Object* obj);

		// update
		void update();

	public:
		// clip operate slots
		void onNewClip();
		void onDuplicateClip();
		void onDeleteClip();
		void onRenameClip();

		// anim operate
		void onAddObject();
		void onAddNode();
		void onAddSetting();
		void onAddResource();

		// property operate
		void onSelectItem();
		void onAddProperty();
		void onSelectProperty();

		// play pause stop
		void onPlayAnim();
		void onStopAnim();
		void onRestartAnim();

		// current edit anim changed
		void onCurrentEditAnimChanged();
		void onCurrentEditAnimLengthChanged();

		// sync data to editor
		void syncDataToEditor();

	private:
		// mouse right button on view
		void onRightClickGraphicsView();
		void onGraphicsSceneWheelEvent();

		// switch cuve visibility
		void onSwitchCurveVisibility();

		// on double click curve key
		void onKeyDoubleClickedCurveKey();
		void onCurveKeyEditingFinished();
		void onKeyPositionChanged();

		// on add key
		void onAddBoolKeyToCurve();
		void onAddStringKeyToCurve();
		void onAddKeyToCurveRed();
		void onAddKeyToCurveGreen();
		void onAddKeyToCurveBlue();
		void onAddKeyToCurveWhite();
		void addKeyToCurve(int curveIdx);

		// get key info
		bool getKeyInfo(KeyInfo& keyInfo, const String& animName, const String& objectPath, const String& propertyName, int curveIdx, int keyIdx);

		// get time and value by pos
		bool calcKeyTimeAndValueByPos(const Vector2& pos, i32& time, float& value);
		bool calcKeyPosByTimeAndValue(i32 time, float value, Vector2& pos);

	private:
		// get new name
		String getNewClipName();

		// sync clip data to editor
		void syncClipTimeLength();
		void syncClipListDataToEditor();
		void syncClipNodeDataToEditor();
		void syncPropertyDataToEditor(QTreeWidgetItem* parentItem, class AnimProperty* property, const StringArray& propertyChain);
		void addNodePropertyToEditor();

		// curve display
		void clearCurveItemsTo(int number);
		void refreshCurveDisplayToEditor(const String& objectPath, const StringArray& propertyChain);
		void refreshCurveKeyDisplayToEditor(const String& objectPath, const StringArray& propertyChain);

		// set current edit anim
		void setCurrentEditAnim(const char* animName);

		// on nodetree widget size changed
		void onNodeTreeWidgetSizeChanged();

	private:
		// set tool button icon
		void setToolbuttonIcon(QToolButton* button, const String& iconPath);

	protected:
		Timeline*					m_timeline;
		QMenu*						m_addObjectMenu;
		String						m_currentEditAnim;
		int							m_nodeTreeWidgetWidth;
		String						m_currentEditObjectPath;
		StringArray					m_currentEditPropertyChain;
		float						m_keyRadius = 7.f;
		QMenu*						m_keyEditMenu = nullptr;
		Vector2						m_keyEditCursorScenePos;
	};
}

#endif