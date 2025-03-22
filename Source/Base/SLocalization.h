//----------------------------------------------------------------------------------------------------------------------
//	SLocalization.h			©2024 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SLocalization

struct SLocalization {
	// Currency
	// See https://www.iso.org/iso-4217-currency-codes.html
	// See https://www.six-group.com/en/products-services/financial-information/data-standards.html
	public:
		struct Currency {
			// Format
			public:
				enum Format {
					kFormatSymbol,			// $123.45
					kFormatCode,			// 123.45 USD
					kFormatNameAsNoValue,	// 0.00 Dollars
					kFormatNameAsSingle,	// 1.00 Dollar
					kFormatNameAsMultiple,	// 2.34 Dollars
				};

			// Methods
			public:
													// Lifecycle methods
													Currency(const Currency& other) :
														mISO4217Code(other.mISO4217Code), mIsCommon(other.mIsCommon)
														{}

													// Instance methods
						const	CString&			getISO4217Code() const
														{ return mISO4217Code; }

								bool				isCommon() const
														{ return mIsCommon; }

								CString				getDisplayName() const;
								CString				getCodeAndDisplayName() const;
								CString				getFormatted(const CString& value, Format format) const;

								bool				operator==(const Currency& other) const
														{ return mISO4217Code == other.mISO4217Code; }
								bool				operator!=(const Currency& other) const
														{ return mISO4217Code != other.mISO4217Code; }

													// Class methods
				static			TArray<Currency>&	getAll();
				static			OV<Currency>		getFor(const CString& iso4217Code);
				static	const	Currency&			getDefault();

			private:
													// Lifecycle methods
													Currency(const CString& iso4217Code, bool isCommon) :
														mISO4217Code(iso4217Code), mIsCommon(isCommon)
														{}

			// Properties
			private:
				CString	mISO4217Code;
				bool	mIsCommon;
		};

	// Language
	// See https://www.loc.gov/standards/iso639-2/php/code_list.php
	// See https://iso639-3.sil.org
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
