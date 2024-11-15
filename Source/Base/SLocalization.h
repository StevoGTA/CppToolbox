//----------------------------------------------------------------------------------------------------------------------
//	SLocalization.h			©2024 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SLocalization

// See https://www.loc.gov/standards/iso639-2/php/code_list.php
// See https://iso639-3.sil.org

struct SLocalization {
	// Language
	public:
		struct Language {
			// Methods
			public:
													// Lifecycle methods
													Language(const Language& other) :
														mISO639_2_Code(other.mISO639_2_Code), mIsCommon(other.mIsCommon)
														{}

													// Instance methods
								OSType				getISO639_2_Code() const
														{ return mISO639_2_Code; }
								CString				getISO639_2_CodeAsString() const
														{ return CString(getISO639_2_Code(), true, false)
																.getSubString(0, 3); }
								bool				isCommon() const
														{ return mIsCommon; }

								CString				getDisplayName() const;

								bool				operator==(const Language& other) const
														{ return mISO639_2_Code == other.mISO639_2_Code; }
								bool				operator!=(const Language& other) const
														{ return mISO639_2_Code != other.mISO639_2_Code; }

													// Class methods
				static			TArray<Language>&	getAll();
				static			OV<Language>		getFor(OSType iso639_2_Code);
				static			OV<Language>		getFor(const CString& iso639_2_CodeString);
				static	const	Language&			getDefault();

				static			CString				getDisplayName(const TArray<Language>& languages);

				static			bool				equals(const TArray<Language>& languages1,
															const TArray<Language>& languages2);

			private:
													// Lifecycle methods
													Language(OSType iso639_2_Code, bool isCommon) :
														mISO639_2_Code(iso639_2_Code), mIsCommon(isCommon)
														{}

			// Properties
			private:
				OSType	mISO639_2_Code;
				bool	mIsCommon;
		};
};
