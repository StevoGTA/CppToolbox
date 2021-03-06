//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTableColumn.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SSQLiteValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTableColumn

class CSQLiteTable;

class CSQLiteTableColumnInternals;
class CSQLiteTableColumn : public CEquatable {
	// Kind
	public:
		enum Kind {
			// Values
			// INTEGER values are whole numbers (either positive or negative).
			kInteger,

			// REAL values are real numbers with decimal values that use 8-byte floats.
			kReal,

			// TEXT is used to store character data. The maximum length of TEXT is unlimited. SQLite supports
			//	various character encodings.
			kText,

			// BLOB stands for a binary large object that can be used to store any kind of data. The maximum size
			//	of BLOBs is unlimited
			kBlob,

			// Dates (not built-in bytes, but we handle)
			//	See https://sqlite.org/lang_datefunc.html
			kDateISO8601FractionalSecondsAutoSet,		// YYYY-MM-DDTHH:MM:SS.SSS (will auto set on insert/replace)
			kDateISO8601FractionalSecondsAutoUpdate,	// YYYY-MM-DDTHH:MM:SS.SSS (will auto update on insert/replace)
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

	// Methods
	public:
											// Lifecycle methods
											CSQLiteTableColumn(const CString& name, Kind kind, Options options = kNone,
													OI<SSQLiteValue> defaultValue = nil);
											CSQLiteTableColumn(const CSQLiteTableColumn& other);
											~CSQLiteTableColumn();

											// CEquatable methods
						bool				operator==(const CEquatable& other) const
												{ return getName() == ((const CSQLiteTableColumn&) other).getName(); }

											// Instance methods
				const	CString&			getName() const;
						Kind				getKind() const;
						Options				getOptions() const;
						OI<SSQLiteValue>	getDefaultValue() const;

											// Class methods
		static	bool						isInteger(Kind kind)
												{ return kind == kInteger; }
		static	bool						isReal(Kind kind)
												{ return kind == kReal; }
		static	bool						isText(Kind kind)
												{ return (kind == kText) ||
														(kind == kDateISO8601FractionalSecondsAutoSet) ||
														(kind == kDateISO8601FractionalSecondsAutoUpdate); }
		static	bool						isBlob(Kind kind)
												{ return kind == kBlob; }

		static	CSQLiteTableColumn			dateISO8601FractionalSecondsAutoSet(const CString& name);
		static	CSQLiteTableColumn			dateISO8601FractionalSecondsAutoUpdate(const CString& name);

	// Properties
	public:
		static	const	CSQLiteTableColumn	mRowID;
		static	const	CSQLiteTableColumn	mAll;

	private:
		CSQLiteTableColumnInternals*	mInternals;
};
