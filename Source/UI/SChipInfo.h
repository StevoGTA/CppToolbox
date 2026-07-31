//----------------------------------------------------------------------------------------------------------------------
//	SChipInfo.h		©2026 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SChipInfo

struct SChipInfo {
	// Style
	public:
		enum Style {
			kStyleOutlined,
			kStyleFilled,
		};

	// Symbol
	public:
		enum Symbol {
			kSymbolNone,
			kSymbolLocked,
		};

	// Methods
	public:
							// Lifecycle methods
							SChipInfo(const CString& text, Style style, Symbol symbol = kSymbolNone) :
								mText(text), mStyle(style), mSymbol(symbol)
								{}
							SChipInfo(const SChipInfo& other) :
								mText(other.mText), mStyle(other.mStyle), mSymbol(other.mSymbol)
								{}

							// Instance methods
		const	CString&	getText() const
								{ return mText; }
				Style		getStyle() const
								{ return mStyle; }
				Symbol		getSymbol() const
								{ return mSymbol; }

	// Properties
	private:
		CString	mText;
		Style	mStyle;
		Symbol	mSymbol;
};
