//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTableColumn.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SSQLiteValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTableColumn

class CSQLiteTable;

class CSQLiteTableColumn : public CEquatable {
	// Kind
	public:
		enum Kind {
			// Values
			// INTEGER values are whole numbers (either positive or negative).
			kKindInteger,

			// REAL values are real numbers with decimal values that use 8-byte floats.
			kKindReal,

			// TEXT is used to store character data. The maximum length of TEXT is unlimited. SQLite supports
			//	various character encodings.
			kKindText,

			// BLOB stands for a binary large object that can be used to store any kind of data. The maximum size
			//	of BLOBs is unlimited
			kKindBlob,

			// Dates (not built-in bytes, but we handle)
			//	See https://sqlite.org/lang_datefunc.html
			kKindDateISO8601FractionalSecondsAutoSet,		// YYYY-MM-DDTHH:MM:SS.SSS (will auto set on insert/replace)
			kKindDateISO8601FractionalSecondsAutoUpdate,	// YYYY-MM-DDTHH:MM:SS.SSS (will auto update on insert/replace)
		};

	// Options
	public:
		enum Options {
			kNone	 		= 0,
			kPrimaryKey		= 1 << 0,
			kAutoIncrement	= 1 << 1,
			kNotNull		= 1 << 2,
			kUnique			= 1 << 3,
			kCheck			= 1 << 4,
		};

	// Reference
	public:
		struct Reference {
			// Lifecycle methods
			Reference(const CSQLiteTableColumn& tableColumn, const CSQLiteTable& referencedTable,
					const CSQLiteTableColumn& referencedTableColumn) :
				mTableColumn(tableColumn), mReferencedTable(referencedTable),
						mReferencedTableColumn(referencedTableColumn)
				{}

			// Properties
			const	CSQLiteTableColumn&	mTableColumn;
			const	CSQLiteTable&		mReferencedTable;
			const	CSQLiteTableColumn&	mReferencedTableColumn;
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											CSQLiteTableColumn(const CString& name, Kind kind, Options options = kNone,
													OV<SSQLiteValue> defaultValue = OV<SSQLiteValue>());
											CSQLiteTableColumn(const CSQLiteTableColumn& other);
											~CSQLiteTableColumn();

											// CEquatable methods
						bool				operator==(const CEquatable& other) const
												{ return getName() == ((const CSQLiteTableColumn&) other).getName(); }

											// Instance methods
				const	CString&			getName() const;
						Kind				getKind() const;
						Options				getOptions() const;
						OV<SSQLiteValue>	getDefaultValue() const;

											// Class methods
		static	bool						isInteger(Kind kind)
												{ return kind == kKindInteger; }
		static	bool						isReal(Kind kind)
												{ return kind == kKindReal; }
		static	bool						isText(Kind kind)
												{ return (kind == kKindText) ||
														(kind == kKindDateISO8601FractionalSecondsAutoSet) ||
														(kind == kKindDateISO8601FractionalSecondsAutoUpdate); }
		static	bool						isBlob(Kind kind)
												{ return kind == kKindBlob; }

		static	CSQLiteTableColumn			dateISO8601FractionalSecondsAutoSet(const CString& name);
		static	CSQLiteTableColumn			dateISO8601FractionalSecondsAutoUpdate(const CString& name);

	// Properties
	public:
		static	const	CSQLiteTableColumn	mRowID;
		static	const	CSQLiteTableColumn	mAll;

	private:
						Internals*			mInternals;
};
