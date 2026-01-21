//----------------------------------------------------------------------------------------------------------------------
//	SSortDescriptor.h			©2026 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"
#include "Tuple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SSortDescriptor

struct SSortDescriptor : public TV2<CString, bool> {
	// Methods
	public:
												// Lifecycle methods
												SSortDescriptor(const CString& identifier, bool isAscending) :
													TV2(identifier, isAscending)
													{}
												SSortDescriptor(const SSortDescriptor& other) : TV2(other) {}

												// Instance methods
				const	CString&				getIdentifier() const
													{ return getA(); }
						bool					getIsAscending() const
													{ return getB(); }

												// Class methods
		static			TArray<CDictionary>		getInfos(const TArray<SSortDescriptor>& sortDescriptors);
		static			TArray<SSortDescriptor>	getSortDescriptors(const TArray<CDictionary>& infos);
};
