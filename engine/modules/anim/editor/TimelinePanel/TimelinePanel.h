#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/bottom_panel_tab.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class Timeline;
	class TimelinePanel : public BottomPanelTab
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;

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

		// sync data to editor
		void syncDataToEditor();

	private:
		// mouse right button on view
		void onRightClickGraphicsView();

		// switch cuve visibility
		void onSwitchCurveVisibility();

		// on double click curve key
		void onKeyDoubleClickedCurveKey();
		void onCurveKeyEditingFinished();

		// get key info
		bool getKeyInfo(KeyInfo& keyInfo, const String& animName, const String& objectPath, const String& propertyName, int curveIdx, int keyIdx);

	private:
		// get new name
		String getNewClipName();

		// sync clip data to editor
		void syncClipListDataToEditor();
		void syncClipNodeDataToEditor();
		void addNodePropertyToEditor();

		// curve display
		void clearCurveItemsTo(int number);
		void refreshCurveDisplayToEditor(const String& objectPath, const String& propertyName);
		void refreshCurveKeyDisplayToEditor(const String& objectPath, const String& propertyName);

		// set current edit anim
		void setCurrentEditAnim(const char* animName);

		// on nodetree widget size changed
		void onNodeTreeWidgetSizeChanged();

	private:
		// draw ruler
		void drawRuler();

	protected:
		Timeline*					m_timeline;
		QWidget*					m_addObjectMenu;
		QAction*					m_addNodeAction;
		String						m_currentEditAnim;
		int							m_nodeTreeWidgetWidth;
		QObject*					m_graphicsScene;

		float						m_rulerHeight;
		Color						m_rulerColor;
		QGraphicsLineItem*			m_rulerBottom;
		array<QGraphicsItem*, 4>	m_curveItems;
		array<QGraphicsItemArray, 4>m_curveKeyItems;
		array<bool, 4>				m_curveVisibles;
		QWidget*					m_curveKeyLineEdit;
		QGraphicsItem*				m_curveKeyItem;
	};
}

#endif