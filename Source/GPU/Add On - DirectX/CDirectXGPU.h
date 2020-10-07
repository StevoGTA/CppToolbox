//----------------------------------------------------------------------------------------------------------------------
//	CDirectXGPU.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"
#include "SDirectXDisplaySupportInfo.h"

using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUProcsInfo

typedef CoreWindow^					(*CDirectXGPUGetCoreWindowProc)(void* userData);
typedef SDirectXDisplaySupportInfo	(*CDirectXGPUGetDisplaySupportInfoProc)(void* userData);
typedef	UInt32						(*CDirectXGPUGetFPSProc)(void* userData);
typedef Float32						(*CDirectXGPUGetDPIProc)(void* userData);
typedef S2DSizeF32					(*CDirectXGPUGetSizeProc)(void* userData);
typedef DisplayOrientations			(*CDirectXGPUGetOrientationProc)(void* userData);
typedef	bool						(*CDirectXGPURequiresDeviceValidationProc)(void* userData);
typedef void						(*CDirectXGPUHandledDeviceValidationProc)(void* userData);

struct SGPUProcsInfo {
								// Lifecycle methods
								SGPUProcsInfo(
										CDirectXGPUGetCoreWindowProc getCoreWindowProc,
										CDirectXGPUGetDisplaySupportInfoProc getDisplaySupportInfoProc,
										CDirectXGPUGetFPSProc getFPSProc, CDirectXGPUGetDPIProc getDPIProc,
										CDirectXGPUGetSizeProc getSizeProc,
										CDirectXGPUGetOrientationProc getOrientationProc,
										CDirectXGPURequiresDeviceValidationProc requiresDeviceValidationProc,
										CDirectXGPUHandledDeviceValidationProc handledDeviceValidationProc,
										void* userData) :
									mGetCoreWindowProc(getCoreWindowProc),
											mGetDisplaySupportInfoProc(getDisplaySupportInfoProc),
											mGetFPSProc(getFPSProc), mGetDPIProc(getDPIProc), mGetSizeProc(getSizeProc),
											mGetOrientationProc(getOrientationProc),
											mRequiresDeviceValidationProc(requiresDeviceValidationProc),
											mHandledDeviceValidationProc(handledDeviceValidationProc),
											mUserData(userData)
									{}
								SGPUProcsInfo(const SGPUProcsInfo& other) :
									mGetCoreWindowProc(other.mGetCoreWindowProc),
											mGetDisplaySupportInfoProc(other.mGetDisplaySupportInfoProc),
											mGetFPSProc(other.mGetFPSProc),  mGetDPIProc(other.mGetDPIProc),
											mGetSizeProc(other.mGetSizeProc),
											mGetOrientationProc(other.mGetOrientationProc),
											mRequiresDeviceValidationProc(other.mRequiresDeviceValidationProc),
											mHandledDeviceValidationProc(other.mHandledDeviceValidationProc),
											mUserData(other.mUserData)
									{}

								// Instance methods
	CoreWindow^					getCoreWindow() const
									{ return mGetCoreWindowProc(mUserData); }
	SDirectXDisplaySupportInfo	getDisplaySupportInfo() const
									{ return mGetDisplaySupportInfoProc(mUserData); }
	UInt32						getFPS() const
									{ return mGetFPSProc(mUserData); }
	Float32						getDPI() const
									{ return mGetDPIProc(mUserData); }
	S2DSizeF32					getSize() const
									{ return mGetSizeProc(mUserData); }
	DisplayOrientations			getOrientation() const
									{ return mGetOrientationProc(mUserData); }
	bool						requiresDeviceValidation() const
									{ return mRequiresDeviceValidationProc(mUserData); }
	void						handledDeviceValidation() const
									{ mHandledDeviceValidationProc(mUserData); }

	// Properties
	private:
		CDirectXGPUGetCoreWindowProc			mGetCoreWindowProc;
		CDirectXGPUGetDisplaySupportInfoProc	mGetDisplaySupportInfoProc;
		CDirectXGPUGetFPSProc					mGetFPSProc;
		CDirectXGPUGetDPIProc					mGetDPIProc;
		CDirectXGPUGetSizeProc					mGetSizeProc;
		CDirectXGPUGetOrientationProc			mGetOrientationProc;
		CDirectXGPURequiresDeviceValidationProc	mRequiresDeviceValidationProc;
		CDirectXGPUHandledDeviceValidationProc	mHandledDeviceValidationProc;
		void*									mUserData;
};
