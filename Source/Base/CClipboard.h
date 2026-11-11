//----------------------------------------------------------------------------------------------------------------------
//	CClipboard.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CClipboard

class CClipboard {
	// Procs
	public:
		typedef	void	(*ContentsReplacedProc)(void* userData);

	// Methods
	public:
							// Class methods
		static	OV<SError>	set(const CString& type, const CData& data, ContentsReplacedProc contentsReplacedProc,
									void* userData);
		static	OV<CData>	getData(const CString& type);
};
