//----------------------------------------------------------------------------------------------------------------------
//	SChipInfo.h		©2026 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SChipInfo

struct SChipInfo {
	// Style
	public:
		enum Style {
			kStyleOutlined,
			kStyleFilled,
			kStyleAccented,
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
							SChipInfo(const CString& text, Style style, const OV<CColor>& color,
									Symbol symbol = kSymbolNone) :
								mText(text), mStyle(style), mSymbol(symbol), mColor(color)
								{}
							SChipInfo(const CString& text, Style style, Symbol symbol = kSymbolNone) :
								mText(text), mStyle(style), mSymbol(symbol)
								{}
							SChipInfo(const SChipInfo& other) :
								mText(other.mText), mStyle(other.mStyle), mSymbol(other.mSymbol),
										mColor(other.mColor)
								{}

							// Instance methods
		const	CString&	getText() const
								{ return mText; }
				Style		getStyle() const
								{ return mStyle; }
				Symbol		getSymbol() const
								{ return mSymbol; }
		const	OV<CColor>&	getColor() const
								{ return mColor; }

	// Properties
	private:
		CString		mText;
		Style		mStyle;
		Symbol		mSymbol;
		OV<CColor>	mColor;
};
