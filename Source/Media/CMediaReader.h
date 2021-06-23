//----------------------------------------------------------------------------------------------------------------------
//	CMediaReader.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SMediaPosition.h"
#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaReader

class CMediaReader {
	// Methods
	public:
							// Lifecycle methods
							CMediaReader() {}
		virtual				~CMediaReader() {}

							// Instance methods
		virtual	Float32		getPercenConsumed() const = 0;
		virtual	OI<SError>	set(const SMediaPosition& mediaPosition,
									const SAudioProcessingFormat& audioProcessingFormat) = 0;
};
