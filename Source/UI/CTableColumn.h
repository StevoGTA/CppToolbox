//----------------------------------------------------------------------------------------------------------------------
//	CTableColumn.h			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTableColumn

class CTableColumn {
	// ValueCompareResult
	public:
		enum ValueCompareResult {
			kValueCompareResultAscending	= -1,
			kValueCompareResultSame			= 0,
			kValueCompareResultDescending	= 1,
		};

	// Procs
	public:
		struct Procs {
			// Procs
			public:
				typedef	void	(*NoteContentChangedProc)(const CTableColumn& tableColumn, void* userData);
				typedef	void	(*NoteTitleChangedProc)(const CTableColumn& tableColumn, void* userData);

			// Methods
			public:
						// Lifecycle methods
						Procs(NoteContentChangedProc noteContentChangedProc,
								NoteTitleChangedProc noteTitleChangedProc, void* userData) :
							mNoteContentChangedProc(noteContentChangedProc),
									mNoteTitleChangedProc(noteTitleChangedProc), mUserData(userData)
							{}
						Procs(const Procs& other) :
							mNoteContentChangedProc(other.mNoteContentChangedProc),
									mNoteTitleChangedProc(other.mNoteTitleChangedProc),
									mUserData(other.mUserData)
							{}

						// Instance methods
				void	noteContentChanged(const CTableColumn& tableColumn) const
							{ mNoteContentChangedProc(tableColumn, mUserData); }
				void	noteTitleChanged(const CTableColumn& tableColumn) const
							{ mNoteTitleChangedProc(tableColumn, mUserData); }

			// Properties
			private:
				NoteContentChangedProc	mNoteContentChangedProc;
				NoteTitleChangedProc	mNoteTitleChangedProc;
				void*					mUserData;
		};

	// Internals
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											CTableColumn(const CString& identifier, UInt16 minWidth,
													UInt16 maxWidth, UInt16 defaultWidth, bool isInitiallyDisplayed,
													bool isEditable);
		virtual								~CTableColumn();

											// Instance methods
				const	CString&			getIdentifier() const;

						UInt16				getMinWidth() const;
						UInt16				getMaxWidth() const;
						UInt16				getWidth() const;
						void				setWidth(UInt16 width);

						bool				isDisplayed() const;
						void				setIsDisplayed(bool isDisplayed);
						bool				isEditable() const;

						void				setProcs(const Procs& procs);

		virtual			CString				getTitle() const = 0;
		virtual			ValueCompareResult	compare(const SValue& value1, const SValue& value2) const;

											// Class methods
		static			bool				compareTitle(const I<CTableColumn>& tableColumn1,
													const I<CTableColumn>& tableColumn2, void* userData);

	protected:
											// Subclass methods
						void				noteContentChanged() const;
						void				noteTitleChanged() const;

	// Properties
	private:
		Internals*	mInternals;
};
