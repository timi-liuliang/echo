/*! \file OneClickCreateCallBack.h
    \brief	MouseCreateCallBack for objects that doesn't have
					3D attributes and therefore could be created with
					just one click. The callback is used by
					SimpleOperator and SimpleTest classes. The class
					has Singleton Pattern.
*/
/**********************************************************************
 *<
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 10-25-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
#include "..\maxapi.h"

class OneClickCreateCallBack : public CreateMouseCallBack {
	public:

    		/** @defgroup OneClickCreateCallBack OneClickCreateCallBack.h
    		*  @{
    		*/

		/*! \fn PFExport static OneClickCreateCallBack* Instance();
		*  \brief 
		*/
		PFExport static OneClickCreateCallBack* Instance();

		/*! \fn PFExport static void DeleteThis();
		*  \brief 
		*/
		PFExport static void DeleteThis();

		/*! \fn PFExport int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
		*  \brief 
		*/
		PFExport int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);

/*@}*/

	protected:
		OneClickCreateCallBack();
		static const OneClickCreateCallBack* instance() { return m_instance; }
		static OneClickCreateCallBack*& _instance() { return m_instance; }

	private:
		static OneClickCreateCallBack* m_instance;
};

