//----------------------------------------------------------------------------------------------------------------------
//	CAudioSession-macOS.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioSession.h"

#include "CLogServices.h"
#include "SVersionInfo.h"

#import <AudioToolbox/AudioToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioSession

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioSession::logInfo()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OSStatus	status;
	UInt32		count;

	// Collect info on default Audio Objects
	AudioObjectPropertyAddress	propertyAddress;
	UInt32						dataSize;

	propertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
	propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
	propertyAddress.mElement = kAudioObjectPropertyElementMaster;

	AudioDeviceID	defaultOutputAudioDeviceID;
	dataSize = sizeof(AudioDeviceID);
	status =
			::AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nil, &dataSize,
					&defaultOutputAudioDeviceID);


	propertyAddress.mSelector = kAudioHardwarePropertyDefaultSystemOutputDevice;
	propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
	propertyAddress.mElement = kAudioObjectPropertyElementMaster;

	AudioDeviceID	defaultSystemOutputAudioDeviceID;
	dataSize = sizeof(AudioDeviceID);
	status =
			::AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nil, &dataSize,
					&defaultSystemOutputAudioDeviceID);

	// Log Audio Objects
	propertyAddress.mSelector = kAudioHardwarePropertyDevices;
	propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
	propertyAddress.mElement = kAudioObjectPropertyElementMaster;

	status = ::AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, nil, &dataSize);
	count = dataSize / sizeof(AudioDeviceID);

	AudioDeviceID	deviceIDs[count];
	status = ::AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nil, &dataSize, &deviceIDs);

	CLogServices::logMessage(
			CString(OSSTR("Found the following Audio Objects (")) + CString(count) + CString(OSSTR("): ")));
	for (UInt32 i = 0; i < count; i++) {
		// Collect info on this Audio Object
		CFStringRef	nameStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertyName;
		status = ::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, &nameStringRef);

		CFStringRef	manufacturerStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertyManufacturer;
		status =
				::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, &manufacturerStringRef);

		CFStringRef	modelNameStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertyModelName;
		status = ::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, &modelNameStringRef);

		CFStringRef	firmwareVersionStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertyFirmwareVersion;
		status =
				::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize,
						&firmwareVersionStringRef);

		CFStringRef	serialNumberStringRef;
		dataSize = sizeof(CFStringRef);
		propertyAddress.mSelector = kAudioObjectPropertySerialNumber;
		status =
				::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, &serialNumberStringRef);

		CString	infoString(nameStringRef);
		if (manufacturerStringRef != nil)
			infoString += CString(OSSTR(", ")) + CString(manufacturerStringRef);
		if (modelNameStringRef != nil)
			infoString += CString(OSSTR(", Model: ")) + CString(modelNameStringRef);
		if (firmwareVersionStringRef != nil)
			infoString += CString(OSSTR(", Firmware Version: ")) + CString(firmwareVersionStringRef);
		if (serialNumberStringRef != nil)
			infoString += CString(OSSTR(", Serial Number: ")) + CString(serialNumberStringRef);
		if (defaultOutputAudioDeviceID == deviceIDs[i])
			infoString += CString(OSSTR(", Default Output Device"));
		if (defaultSystemOutputAudioDeviceID == deviceIDs[i])
			infoString += CString(OSSTR(", Default System Output Device"));
		CLogServices::logMessage(
				CString(OSSTR("    ")) + infoString + CString(OSSTR(", with the following streams:")));

		// Log info on the input streams for this Audio Object
		propertyAddress.mSelector = kAudioDevicePropertyStreamConfiguration;
		propertyAddress.mScope = kAudioDevicePropertyScopeInput;
		propertyAddress.mElement = kAudioObjectPropertyElementMaster;
		::AudioObjectGetPropertyDataSize(deviceIDs[i], &propertyAddress, 0, nil, &dataSize);

		AudioBufferList*	audioBufferList = (AudioBufferList*) ::malloc(dataSize);
		::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, audioBufferList);

		for (UInt32 j = 0; j < audioBufferList->mNumberBuffers; j++)
			//
			CLogServices::logMessage(
					CString(OSSTR("        Stream ")) + CString(j) + CString(OSSTR(": ")) +
							CString(audioBufferList->mBuffers[j].mNumberChannels) +
							CString(OSSTR(" input channel(s)")));
		::free(audioBufferList);

		// Log info on the output streams for this Audio Object
		propertyAddress.mSelector = kAudioDevicePropertyStreamConfiguration;
		propertyAddress.mScope = kAudioDevicePropertyScopeOutput;
		propertyAddress.mElement = kAudioObjectPropertyElementMaster;
		::AudioObjectGetPropertyDataSize(deviceIDs[i], &propertyAddress, 0, nil, &dataSize);

		audioBufferList = (AudioBufferList*) ::malloc(dataSize);
		::AudioObjectGetPropertyData(deviceIDs[i], &propertyAddress, 0, nil, &dataSize, audioBufferList);

		for (UInt32 j = 0; j < audioBufferList->mNumberBuffers; j++)
			//
			CLogServices::logMessage(
					CString(OSSTR("        Stream ")) + CString(j) + CString(OSSTR(": ")) +
							CString(audioBufferList->mBuffers[j].mNumberChannels) +
							CString(OSSTR(" output channel(s)")));
		::free(audioBufferList);
	}

	// Log info on output Audio Components
	AudioComponentDescription	desc = {0};
	desc.componentType = kAudioUnitType_Output;

	count = ::AudioComponentCount(&desc);
	CLogServices::logMessage(
			CString(OSSTR("Found the following Output Units (")) + CString(count) + CString(OSSTR("): ")));

	AudioComponent	outputUnitComponent = nil;
	do {
		outputUnitComponent = ::AudioComponentFindNext(outputUnitComponent, &desc);
		if (outputUnitComponent != nil) {
			// Get info on this Audio Component
			CFStringRef	stringRef;
			status = ::AudioComponentCopyName(outputUnitComponent, &stringRef);

			AudioComponentDescription	fullDesc;
			status = ::AudioComponentGetDescription(outputUnitComponent, &fullDesc);

			UInt32	version;
			status = ::AudioComponentGetVersion(outputUnitComponent, &version);
			SVersionInfo	versionInfo(version);

			CFDictionaryRef	dictionaryRef;
			status = ::AudioComponentCopyConfigurationInfo(outputUnitComponent, &dictionaryRef);

			CLogServices::logMessage(
					CString(OSSTR("    ")) + CString(stringRef) +
							CString(OSSTR(" v")) + versionInfo.getString() +
							CString(OSSTR(" (")) + CString(fullDesc.componentType, true) +
									CString(OSSTR(", ")) + CString(fullDesc.componentSubType, true) +
									CString(OSSTR(", ")) + CString(fullDesc.componentManufacturer, true) +
									CString(OSSTR(")")));
		}
	} while (outputUnitComponent != nil);
}
