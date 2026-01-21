//----------------------------------------------------------------------------------------------------------------------
//	CTableViewItem.h			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "SValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTableViewItem

class CTableViewItem {
	// Content
	public:
		struct Content {
			// Methods
			public:
										// Lifecycle methods
										Content(const OV<SValue>& value, const CString& displayString) :
											mValue(value), mDisplayString(displayString), mToolTipString(displayString)
											{}
										Content(const SValue& value, const CString& displayString) :
											mValue(value), mDisplayString(displayString), mToolTipString(displayString)
											{}
										Content(const SValue& value) : mValue(value) {}
										Content(const CString& string) :
											mValue(string), mDisplayString(string), mToolTipString(string)
											{}
										Content(const CString& displayString, const CColor& displayColor) :
											mDisplayString(displayString), mDisplayColor(displayColor),
													mToolTipString(displayString)
											{}
										Content(const Content& other) :
											mValue(other.mValue), mDisplayString(other.mDisplayString),
													mDisplayColor(other.mDisplayColor),
													mToolTipString(other.mToolTipString)
											{}

										// Instance methods
				const	OV<SValue>&		getValue() const
											{ return mValue; }
				const	OV<CString>&	getDisplayString() const
											{ return mDisplayString; }
				const	OV<CColor>&		getDisplayColor() const
											{ return mDisplayColor; }
				const	OV<CString>&	getToolTipString() const
											{ return mToolTipString; }

			// Properties
			private:
				OV<SValue>	mValue;
				OV<CString>	mDisplayString;
				OV<CColor>	mDisplayColor;
				OV<CString>	mToolTipString;
		};

	// Methods
	public:

									// Lifecycle methods
									CTableViewItem() {}
		virtual						~CTableViewItem() {}

									// Instance methods
		virtual	const	CString&	getID() const = 0;
		virtual			OSType		getType() const = 0;
};
