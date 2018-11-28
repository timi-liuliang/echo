#pragma once

#include "engine/core/editor/bottom_panel_tab.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class Timeline;
	class TimelinePanel : public BottomPanelTab
	{
	public:
		TimelinePanel(Object* obj);

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
		void onAddProperty();

		// play pause stop
		void onPlayAnim();

	private:
		// get new name
		String getNewClipName();

		// sync clip data to editor
		void syncClipListDataToEditor();
		void syncClipNodeDataToEditor();

		// set current edit anim
		void setCurrentEditAnim(const char* animName);

	protected:
		Timeline*		m_timeline;
		QWidget*		m_addObjectMenu;
		QAction*		m_addNodeAction;
		String			m_currentEditAnim;
	};
}

#endif