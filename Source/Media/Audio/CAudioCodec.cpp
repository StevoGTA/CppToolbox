//----------------------------------------------------------------------------------------------------------------------
//	CAudioCodec.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioCodec.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioCodec

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CAudioCodec::getPacketIndex(const SMediaPosition& mediaPosition,
		const SAudioProcessingFormat& audioProcessingFormat,
		const TArray<CCodec::PacketAndLocation>& packetAndLocations)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get new sample position
	UInt64	frameIndex = mediaPosition.getFrameIndex(audioProcessingFormat.getSampleRate());

	// Find packet index
	UInt32	packetIndex = 0;
	for (TIteratorD<CCodec::PacketAndLocation> iterator = packetAndLocations.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Setup
		const	CAudioCodec::Packet& packet = iterator->mPacket;
		if (frameIndex > packet.mDuration) {
			// Advance another packet
			packetIndex++;
			frameIndex -= packet.mDuration;
		} else
			// Done
			break;
	}

	return packetIndex;
}
