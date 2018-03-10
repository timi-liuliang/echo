/**********************************************************************
 *<
	FILE: IRollupSettings

	DESCRIPTION: Rollup Window Settings Interface

	CREATED BY: Nikolai Sander

	HISTORY: created 8/8/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "ifnpub.h"
#include "GetCOREInterface.h"

// forward declarations
class ICatRegistry;

#define EFFECTS_ROLLUPCFG_CLASSID Class_ID(0x559c705d, 0x32573fe3)
#define ATMOSPHERICS_ROLLUPCFG_CLASSID Class_ID(0x40ef5775, 0x5b5606f)
#define DISPLAY_PANEL_ROLLUPCFG_CLASSID Class_ID(0x12d45445, 0x3a5779a2)
#define MOTION_PANEL_ROLLUPCFG_CLASSID Class_ID(0xbea1816, 0x50de0291)
#define HIERARCHY_PANEL_ROLLUPCFG_CLASSID Class_ID(0x5d6c08d4, 0x7e5e2c2b)
#define	UTILITY_PANEL_ROLLUPCFG_CLASSID   Class_ID(0x2e256000, 0x6a5b2b34)

/*! \sa  Class ICatRegistry , Class FPStaticInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface class for rollup settings. You can obtain a
pointer to this interface using; <b>IRollupSettings* GetIRollupSettings()</b>.
This macro will return\n\n
<b>(IRollupSettings*)GetCOREInterface(ROLLUP_SETTINGS_INTERFACE).</b>  */
class IRollupSettings : public FPStaticInterface 
{
	public:
	/*! \remarks This method returns a pointer to the category registry. */
	virtual ICatRegistry *GetCatReg()=0;
};

/*! \sa  Class IRollupSettings, Class IRollupWindow, Class IRollupCallback\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class makes it possible to get and set the category field (order number)
for rollups in the RollupOrder.cfg file. Normally a developer would use this
class in order to store the order of rollout panels as a result of a custom
handled Drag and Drop operation (see
<b>IRollupWindow::HandleRollupCallback()</b> and
<b>IRollupCallback::HandleDrop()</b>). First it is necessary to loop through
all rollup panels and update the category field in the <b>ICatRegistry</b>\n\n
\code
void TaskRollupManager::UpdateCats()
{
	for(int i = 0 ; i < GetNumPanels() ; i++) {
		IRollupPanel *panel = GetPanel(GetPanelDlg(i));
		Class_ID cid;
		SClass_ID sid;
		if(GetEditObjClassID(sid,cid)) {
			MCHAR title[256];
			GetWindowText(panel->GetTitleWnd(), title, 255);
			GetIRollupSettings()->GetCatReg()->UpdateCat(sid,cid, title, panel->GetCategory());
		}
	}
}
\endcode 
After that the ICatRegistry has to be saved with\n\n
<b>GetIRollupSettings()-\>GetCatReg()-\>Save();</b>\n\n
In order to reset the rollup order, one has to call
<b>ICatRegistry::DeleteList()</b> for a given superclassid and classid. The
structure of the RollupOrder.cfg file (and thus <b>CatRegistry</b>) can be seen
as list of items that represent the rollup order for each particular group. A
group is identified by a super classid and a classid.\n\n
In 3ds Max we can have different possibilities to store the order of rollups.
In the Create and Modify Panel we want to store the order of rollups for the
currently edited object, which can be an object, or a modifier. Thus the
classid and superclassid of the group is the one of the currently edited
object. For the Display panel however, we want the rollups to be the same no
matter which object is currently selected. To do this, we create a dummy
classid which represents the display panel and store the rollup orders under
this dummy classid. This classid is identified by the method
<b>IRollupCallback::GetEditObjClassID()</b>. Note, that no classddesc has to
exist for a dummy classid. The RollupOrder.cfg file will indicate "No
ClassName" in case a dummy class id is used.  */
class ICatRegistry: public MaxHeapOperators
{
public:
	/*! \remarks Destructor. */
	virtual ~ICatRegistry() {;}
	// This method gets the category (order field) for the given SuperClass ID, ClassID and Rollup Title
	/*! \remarks This method gets the category (order field) for the given
	SuperClass ID, ClassID and Rollup Title.
	\par Parameters:
	<b>SClass_ID sid</b>\n\n
	The superclass ID.\n\n
	<b>Class_ID cid</b>\n\n
	The class ID.\n\n
	<b>MCHAR *title</b>\n\n
	The rollup title.\n\n
	<b>int category</b>\n\n
	`This is used for error checking; it becomes the default value retured by
	the function if the call fails. So if you pass -1000 and get a return value
	of -1000, that means the entry was not found. */
	virtual int  GetCat(SClass_ID sid, Class_ID cid, const MCHAR *title,int category)=0;
	// This method updates (sets) the category (order field) for the given SuperClass ID, ClassID and Rollup Title
	/*! \remarks This method updates (sets) the category (order field) for the
	given SuperClass ID, ClassID and Rollup Title
	\par Parameters:
	<b>SClass_ID sid</b>\n\n
	The superclass ID.\n\n
	<b>Class_ID cid</b>\n\n
	The class ID.\n\n
	<b>MCHAR *title</b>\n\n
	The rollup title.\n\n
	<b>int category</b>\n\n
	The category. */
	virtual void UpdateCat(SClass_ID sid, Class_ID cid, const MCHAR *title,int category)=0;
	// This method Saves the category settings in File "UI\RollupOrder.cfg"
	/*! \remarks This method Saves the category settings in File
	<b>UI/RollupOrder.cfg</b> */
	virtual void Save()=0;
	// This method Loads the category settings from File "UI\RollupOrder.cfg"
	/*! \remarks This method Loads the category settings from File
	<b>UI/RollupOrder.cfg</b> */
	virtual void Load()=0;
	// This method Erases all category settings (in memory only)
	/*! \remarks This method Erases all category settings (in memory only) */
	virtual void EmptyRegistry()=0;
	// This method deletes a category list for a given superclass ID and class ID
	/*! \remarks This method deletes a category list for a given superclass ID
	and class ID.
	\par Parameters:
	<b>SClass_ID sid</b>\n\n
	The superclass ID.\n\n
	<b>Class_ID cid</b>\n\n
	The class ID. */
	virtual void DeleteList(SClass_ID sid, Class_ID cid)=0;
};

#define ROLLUP_SETTINGS_INTERFACE Interface_ID(0x281a65e8, 0x12db025d)
inline IRollupSettings* GetIRollupSettings() { return (IRollupSettings*)GetCOREInterface(ROLLUP_SETTINGS_INTERFACE); }

