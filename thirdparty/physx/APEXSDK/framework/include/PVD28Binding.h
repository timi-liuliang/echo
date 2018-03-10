/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PVD28_BINDING_H
#define PVD28_BINDING_H

#include "PsShare.h"
#ifndef WITHOUT_PVD

#include "NxApexDefs.h"
#if NX_SDK_VERSION_MAJOR == 2

#include "foundation/Px.h"

#include "PvdBinding.h"
class NxRemoteDebugger;

namespace PVD {

	class Pvd28Binding : public PvdBinding
	{
	protected:
		virtual ~Pvd28Binding(){}
	public:
		//overloads the connect method.

		//Perhaps setup a foundation instance, and attach yourself to this debugger.
		static Pvd28Binding& attach( NxRemoteDebugger& inDebugger, bool inSetupFoundation = true );
	};

}
#endif // NX_SDK_VERSION_MAJOR == 2
#endif // WITHOUT_PVD
#endif // PVD28_BINDING_H
