//----------------------------------------------------------------------------------------------------------------------
//	CDirectXGPU.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"
#include "SDirectXDisplaySupportInfo.h"

using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPU::Procs

struct CGPU::Procs {
	// Procs
	typedef CoreWindow^					(*GetCoreWindowProc)(void* userData);
	typedef SDirectXDisplaySupportInfo	(*GetDisplaySupportInfoProc)(void* userData);
	typedef	UInt32						(*GetFPSProc)(void* userData);
	typedef Float32						(*GetDPIProc)(void* userData);
	typedef S2DSizeF32					(*GetSizeProc)(void* userData);
	typedef DisplayOrientations			(*GetOrientationProc)(void* userData);
	typedef	bool						(*RequiresDeviceValidationProc)(void* userData);
	typedef void						(*HandledDeviceValidationProc)(void* userData);

								// Lifecycle methods
								Procs(
										GetCoreWindowProc getCoreWindowProc,
										GetDisplaySupportInfoProc getDisplaySupportInfoProc, GetFPSProc getFPSProc,
										GetDPIProc getDPIProc, GetSizeProc getSizeProc,
										GetOrientationProc getOrientationProc,
										RequiresDeviceValidationProc requiresDeviceValidationProc,
										HandledDeviceValidationProc handledDeviceValidationProc, void* userData) :
									mGetCoreWindowProc(getCoreWindowProc),
											mGetDisplaySupportInfoProc(getDisplaySupportInfoProc),
											mGetFPSProc(getFPSProc), mGetDPIProc(getDPIProc), mGetSizeProc(getSizeProc),
											mGetOrientationProc(getOrientationProc),
											mRequiresDeviceValidationProc(requiresDeviceValidationProc),
											mHandledDeviceValidationProc(handledDeviceValidationProc),
											mUserData(userData)
									{}
								Procs(const Procs& other) :
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
		GetCoreWindowProc				mGetCoreWindowProc;
		GetDisplaySupportInfoProc		mGetDisplaySupportInfoProc;
		GetFPSProc						mGetFPSProc;
		GetDPIProc						mGetDPIProc;
		GetSizeProc						mGetSizeProc;
		GetOrientationProc				mGetOrientationProc;
		RequiresDeviceValidationProc	mRequiresDeviceValidationProc;
		HandledDeviceValidationProc		mHandledDeviceValidationProc;
		void*							mUserData;
};
