//----------------------------------------------------------------------------------------------------------------------
//	CIterator.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CIterator

class CIterator {
	// Methods
	public:
								// Lifecycle methods
		virtual					~CIterator() {}

								// Instance methods
		virtual		bool		isValid() const = 0;
		virtual		UInt32		getIndex() const = 0;
					bool		isFirst() const
									{ return getIndex() == 0; }
		virtual		void		advance() = 0;

		explicit				operator bool() const
									{ return isValid(); }
					CIterator&	operator++()
									{ advance(); return *this; }
					CIterator&	operator++(int)
									{ advance(); return *this; }

	protected:
								// Lifecycle methods
								CIterator() {}
};
