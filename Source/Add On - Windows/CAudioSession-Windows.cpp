//----------------------------------------------------------------------------------------------------------------------
//	CAudioSession-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioSession.h"

#include "CLogServices.h"
#include "CPlatform.h"

#undef Delete

#include <collection.h>
#include <ppltasks.h>

#define Delete(x)	{ delete x; x = nil; }

using namespace Windows::Media::Devices;
using namespace Windows::Devices::Enumeration;

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	String^	PKEY_AudioEndpoint_Supports_EventDriven_Mode = "{1da5d803-d492-4edd-8c23-e0c0ffee7f0e} 7";

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioSessionInternals

ref class CAudioSessionInternals sealed {
	public:
				CAudioSessionInternals()
					{

					}

		void	logInfo()
					{
						// Setup
						String^	audioRenderSelector = MediaDevice::GetAudioRenderSelector();

						auto	properties = ref new Platform::Collections::Vector<String^>();
						properties->Append(PKEY_AudioEndpoint_Supports_EventDriven_Mode);

						// Enumerate
						Concurrency::task<DeviceInformationCollection^>(
										DeviceInformation::FindAllAsync(audioRenderSelector, properties))
								.then([this](DeviceInformationCollection^ deviceInformationCollection) {
							// Handle results
							if (deviceInformationCollection == nullptr) {
								// Log
								CLogServices::logMessage(CString(OSSTR("Query Audio Render Selectors failed.")));

								return;
							}

							// Log
							CLogServices::logMessage(
									CString(OSSTR("Found the following Audio Renderers (")) +
											CString(deviceInformationCollection->Size) + CString(OSSTR("): ")));
							try {
								// Iterate results
								for(unsigned int i = 0; i < deviceInformationCollection->Size; i++) {
									// Setup
									DeviceInformation^	deviceInfo = deviceInformationCollection->GetAt(i);
									CLogServices::logMessage(
											CString(OSSTR("    ")) + CPlatform::stringFrom(deviceInfo->Name));

									// Iterate properties
//									Object^	string =
//													safe_cast<Object^>(deviceInfo->Properties->Lookup(
//															PKEY_AudioEndpoint_Supports_EventDriven_Mode));
//									if (string != nullptr) {
//										// Log
//bool value = true;
//										CLogServices::logMessage(
//												CString(OSSTR("        Supports Event-driven Mode: ")) +
//														CString(value ? OSSTR("yes") : OSSTR("no")));
//									}
								}
							} catch (Platform::Exception^ exception) {
								// Exception
								CLogServices::logError(CPlatform::stringFrom(exception->Message));
							}
						});
					}
};

static	CAudioSessionInternals	sAudioSessionInternals;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioSession

// Properties

CAudioSession CAudioSession::mShared;

// Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioSession::CAudioSession()
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CAudioSession::~CAudioSession()
//----------------------------------------------------------------------------------------------------------------------
{
}

// Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::activate()
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::deactivate()
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioSession::isActivated() const
//----------------------------------------------------------------------------------------------------------------------
{
	return true;
}

//----------------------------------------------------------------------------------------------------------------------
bool CAudioSession::inputIsAvailable() const
//----------------------------------------------------------------------------------------------------------------------
{
	return false;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::set(Category category)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::set(OverrideRoute overrideRoute)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CAudioSession::getCurrentHardwareIOBufferDuration() const
//----------------------------------------------------------------------------------------------------------------------
{
	return 0.25;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::setPreferredHardwareIOBufferDuration(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CAudioSession::getCurrentHardwareOutputVolume() const
//----------------------------------------------------------------------------------------------------------------------
{
	return 1.0;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::logInfo()
//----------------------------------------------------------------------------------------------------------------------
{
	sAudioSessionInternals.logInfo();
}
