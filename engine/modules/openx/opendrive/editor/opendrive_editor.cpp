#include "opendrive_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	OpenDriveEditor::OpenDriveEditor(Object* object)
		: ObjectEditor(object)
	{

	}

	OpenDriveEditor::~OpenDriveEditor()
	{

	}

	ImagePtr OpenDriveEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/openx/opendrive/editor/icon/opendrive.png");
	}

	void OpenDriveEditor::onEditorSelectThisNode()
	{
	}

	void OpenDriveEditor::postEditorCreateObject()
	{

	}

	void OpenDriveEditor::editor_update_self()
	{

	}
#endif
}

