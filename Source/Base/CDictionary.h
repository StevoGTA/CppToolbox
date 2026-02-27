//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSet.h"
#include "CString.h"
#include "SValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDictionary

class CDictionary : public CEquatable {
	// Types
	public:
		typedef	UInt32	Count;

				class	IteratorInfo;

	// Item
	public:
		struct Item {
			// Procs
			typedef bool	(*IncludeProc)(const CString& key, const SValue& value, void* userData);

			// Methods
			public:

								// Lifecycle methods
								Item(const CString& key, const SValue& value) : mKey(key), mValue(value) {}
								Item(const CString& key, const SValue& value, SValue::OpaqueCopyProc opaqueCopyProc) :
									mKey(key), mValue(value, opaqueCopyProc)
									{}
								Item(const Item& other, SValue::OpaqueCopyProc opaqueCopyProc) :
									mKey(other.mKey), mValue(other.mValue, opaqueCopyProc)
									{}

								// Instance methods
			const	CString&	getKey() const
									{ return mKey; }
					SValue&		getValue() const
									{ return (SValue&) mValue; }

			// Properties
			private:
				CString	mKey;
				SValue	mValue;
		};

	// Procs
	public:
		struct Procs {
			// Procs
			public:
				typedef	Count		(*GetCountProc)(void* userData);
				typedef	CString		(*GetKeyAtIndexProc)(UInt32 index, void* userData);
				typedef	OR<SValue>	(*GetValueProc)(const CString& key, void* userData);
				typedef	void		(*SetProc)(const CString& key, const SValue& value, void* userData);
				typedef	void		(*RemoveKeysProc)(const TSet<CString>& keys, void* userData);
				typedef	void		(*RemoveAllProc)(void* userData);
				typedef	void		(*DisposeUserDataProc)(void* userData);

			// Methods
			public:
							// Lifecycle methods
							Procs(GetCountProc getCountProc, GetKeyAtIndexProc getKeyAtIndexProc,
									GetValueProc getValueProc, SetProc setProc, RemoveKeysProc removeKeysProc,
									RemoveAllProc removeAllProc, DisposeUserDataProc disposeUserDataProc,
											void* userData) :
								mGetCountProc(getCountProc), mGetKeyAtIndexProc(getKeyAtIndexProc),
										mGetValueProc(getValueProc), mSetProc(setProc), mRemoveKeysProc(removeKeysProc),
										mRemoveAllProc(removeAllProc), mDisposeUserDataProc(disposeUserDataProc),
										mUserData(userData)
								{}

							// Instance methods
				Count		getCount() const
								{ return mGetCountProc(mUserData); }
				CString		getKeyAtIndex(UInt32 index) const
								{ return mGetKeyAtIndexProc(index, mUserData); }
				OR<SValue>	getValue(const CString& key) const
								{ return mGetValueProc(key, mUserData); }
				void		set(const CString& key, const SValue& value)
								{ mSetProc(key, value, mUserData); }
				void		removeKeys(const TSet<CString>& keys)
								{ mRemoveKeysProc(keys, mUserData); }
				void		removeAll()
								{ mRemoveAllProc(mUserData); }
				void		disposeUserData() const
								{ return mDisposeUserDataProc(mUserData); }

			// Properties
			private:
				GetCountProc		mGetCountProc;
				GetKeyAtIndexProc	mGetKeyAtIndexProc;
				GetValueProc		mGetValueProc;
				SetProc				mSetProc;
				RemoveKeysProc		mRemoveKeysProc;
				RemoveAllProc		mRemoveAllProc;
				DisposeUserDataProc	mDisposeUserDataProc;
				void*				mUserData;
		};

	// Iterators
	public:
		class Iterator : public CIterator {
			// Methods
			public:
									// Lifecycle methods
									Iterator(const I<IteratorInfo>& iteratorInfo);
									Iterator(const Iterator& other);

									// CIterator methods
							bool	isValid() const;
							UInt32	getIndex() const;
							void	advance();

									// Instance methods
				const	CString&	getKey() const;
						SValue&		getValue() const;

			// Properties
			private:
				I<IteratorInfo>	mIteratorInfo;
		};

		class KeyIterator : public Iterator {
			// Methods
			public:
									// Lifecycle methods
									KeyIterator(const I<IteratorInfo>& iteratorInfo) : Iterator(iteratorInfo) {}
									KeyIterator(const KeyIterator& other) : Iterator(other) {}

									// Instance methods
				const	CString&	operator*() const
										{ return getKey(); }
		};

		class ValueIterator : public Iterator {
			// Methods
			public:
						// Lifecycle methods
						ValueIterator(const I<IteratorInfo>& iteratorInfo) : Iterator(iteratorInfo) {}
						ValueIterator(const ValueIterator& other) : Iterator(other) {}

						// Instance methods
				SValue&	operator*() const
							{ return getValue(); }
		};

	// Internals
	protected:
		class Internals;

	// Procs:
	protected:
		typedef			UInt32		(*IteratorGetCurrentIndexProc)(const I<IteratorInfo>& iteratorInfo);
		typedef	const	CString&	(*IteratorGetCurrentKeyProc)(const I<IteratorInfo>& iteratorInfo);
		typedef			SValue*		(*IteratorGetCurrentValueProc)(const I<IteratorInfo>& iteratorInfo);
		typedef			Item*		(*IteratorAdvanceProc)(const I<IteratorInfo>& iteratorInfo);

	// Methods
	public:
													// Lifecycle methods
													CDictionary(SValue::OpaqueCopyProc opaqueCopyProc = nil,
															SValue::OpaqueEqualsProc opaqueEqualsProc = nil,
															SValue::OpaqueDisposeProc opaqueDisposeProc = nil);
													CDictionary(const Procs& procs);
													CDictionary(const CDictionary& other);
		virtual										~CDictionary();

													// CEquatable methods
						bool						operator==(const CEquatable& other) const
														{ return equals((const CDictionary&) other); }

													// Instance methods
		virtual			Count						getCount() const;
		virtual			TSet<CString>				getKeys() const;
						bool						isEmpty() const
														{ return getCount() == 0; }

		virtual			bool						contains(const CString& key) const;

				const	SValue&						getValue(const CString& key) const;
						OV<SValue>					getOValue(const CString& key) const;
						bool						getBool(const CString& key, bool defaultValue = false) const;
						OV<bool>					getOVBool(const CString& key) const;
				const	TArray<CDictionary>&		getArrayOfDictionaries(const CString& key,
															const TArray<CDictionary>& defaultValue =
																	TNArray<CDictionary>()) const;
						OV<TArray<CDictionary> >	getOVArrayOfDictionaries(const CString& key) const;
				const	TArray<CString>&			getArrayOfStrings(const CString& key,
															const TArray<CString>& defaultValue = TNArray<CString>())
															const;
						OV<TArray<CString> >		getOVArrayOfStrings(const CString& key) const;
				const	CData&						getData(const CString& key,
															const CData& defaultValue = CData::mEmpty) const;
						OV<CData>					getOVData(const CString& key) const;
				const	CDictionary&				getDictionary(const CString& key,
															const CDictionary& defaultValue = mEmpty) const;
						OV<CDictionary>				getOVDictionary(const CString& key) const;
				const	CString&					getString(const CString& key,
															const CString& defaultValue = CString::mEmpty) const;
						OV<CString>					getOVString(const CString& key) const;
						Float32						getFloat32(const CString& key, Float32 defaultValue = 0.0) const;
						OV<Float32>					getOVFloat32(const CString& key) const;
						Float64						getFloat64(const CString& key, Float64 defaultValue = 0.0) const;
						OV<Float64>					getOVFloat64(const CString& key) const;
						SInt8						getSInt8(const CString& key, SInt8 defaultValue = 0) const;
						OV<SInt8>					getOVSInt8(const CString& key) const;
						SInt16						getSInt16(const CString& key, SInt16 defaultValue = 0) const;
						OV<SInt16>					getOVSInt16(const CString& key) const;
						SInt32						getSInt32(const CString& key, SInt32 defaultValue = 0) const;
						OV<SInt32>					getOVSInt32(const CString& key) const;
						SInt64						getSInt64(const CString& key, SInt64 defaultValue = 0) const;
						OV<SInt64>					getOVSInt64(const CString& key) const;
						UInt8						getUInt8(const CString& key, UInt8 defaultValue = 0) const;
						OV<UInt8>					getOVUInt8(const CString& key) const;
						UInt16						getUInt16(const CString& key, UInt16 defaultValue = 0) const;
						OV<UInt16>					getOVUInt16(const CString& key) const;
						UInt32						getUInt32(const CString& key, UInt32 defaultValue = 0) const;
						OV<UInt32>					getOVUInt32(const CString& key) const;
						UInt64						getUInt64(const CString& key, UInt64 defaultValue = 0) const;
						OV<UInt64>					getOVUInt64(const CString& key) const;
						OV<SValue::Opaque>			getOpaque(const CString& key) const;
						OSType						getOSType(const CString& key, OSType defaultValue = 0) const
														{ return getUInt32(key, defaultValue); }
						OV<OSType>					getOVOSType(const CString& key) const
														{ return getOVUInt32(key); }
						void						getValue(const CString& key, bool& outValue,
															bool defaultValue = false) const
														{ outValue = getBool(key, defaultValue); }
						void						getValue(const CString& key, OV<bool>& outValue) const
														{ outValue = getOVBool(key); }
						void						getValue(const CString& key, Float32& outValue,
															Float32 defaultValue = 0.0) const
														{ outValue = getFloat32(key, defaultValue); }
						void						getValue(const CString& key, OV<Float32>& outValue) const
														{ outValue = getOVFloat32(key); }
						void						getValue(const CString& key, Float64& outValue,
															Float64 defaultValue = 0.0) const
														{ outValue = getFloat64(key, defaultValue); }
						void						getValue(const CString& key, OV<Float64>& outValue) const
														{ outValue = getOVFloat64(key); }
						void						getValue(const CString& key, SInt8& outValue,
															SInt8 defaultValue = 0) const
														{ outValue = getSInt8(key, defaultValue); }
						void						getValue(const CString& key, OV<SInt8>& outValue) const
														{ outValue = getOVSInt8(key); }
						void						getValue(const CString& key, SInt16& outValue,
															SInt16 defaultValue = 0) const
														{ outValue = getSInt16(key, defaultValue); }
						void						getValue(const CString& key, OV<SInt16>& outValue) const
														{ outValue = getOVSInt16(key); }
						void						getValue(const CString& key, SInt32& outValue,
															SInt32 defaultValue = 0) const
														{ outValue = getSInt32(key, defaultValue); }
						void						getValue(const CString& key, OV<SInt32>& outValue) const
														{ outValue = getOVSInt32(key); }
						void						getValue(const CString& key, SInt64& outValue,
															SInt64 defaultValue = 0) const
														{ outValue = getSInt64(key, defaultValue); }
						void						getValue(const CString& key, OV<SInt64>& outValue) const
														{ outValue = getOVSInt64(key); }
						void						getValue(const CString& key, UInt8& outValue,
															UInt8 defaultValue = 0) const
														{ outValue = getUInt8(key, defaultValue); }
						void						getValue(const CString& key, OV<UInt8>& outValue) const
														{ outValue = getOVUInt8(key); }
						void						getValue(const CString& key, UInt16& outValue,
															UInt16 defaultValue = 0) const
														{ outValue = getUInt16(key, defaultValue); }
						void						getValue(const CString& key, OV<UInt16>& outValue) const
														{ outValue = getOVUInt16(key); }
						void						getValue(const CString& key, UInt32& outValue,
															UInt32 defaultValue = 0) const
														{ outValue = getUInt32(key, defaultValue); }
						void						getValue(const CString& key, OV<UInt32>& outValue) const
														{ outValue = getOVUInt32(key); }
						void						getValue(const CString& key, UInt64& outValue,
															UInt64 defaultValue = 0) const
														{ outValue = getUInt64(key, defaultValue); }
						void						getValue(const CString& key, OV<UInt64>& outValue) const
														{ outValue = getOVUInt64(key); }

						void						set(const CString& key, bool value);
						void						set(const CString& key, const TArray<CDictionary>& value);
						void						set(const CString& key, const OV<TArray<CDictionary> >& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, const TArray<CString>& value);
						void						set(const CString& key, const OV<TArray<CString> >& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, const CData& value);
						void						set(const CString& key, const OV<CData>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, const CDictionary& value);
						void						set(const CString& key, const OV<CDictionary>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, const CString& value);
						void						set(const CString& key, const OV<CString>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, Float32 value);
						void						set(const CString& key, const OV<Float32>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, Float64 value);
						void						set(const CString& key, const OV<Float64>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, SInt8 value);
						void						set(const CString& key, const OV<SInt8>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, SInt16 value);
						void						set(const CString& key, const OV<SInt16>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, SInt32 value);
						void						set(const CString& key, const OV<SInt32>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, SInt64 value);
						void						set(const CString& key, const OV<SInt64>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, UInt8 value);
						void						set(const CString& key, const OV<UInt8>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, UInt16 value);
						void						set(const CString& key, const OV<UInt16>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, UInt32 value);
						void						set(const CString& key, const OV<UInt32>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, UInt64 value);
						void						set(const CString& key, const OV<UInt64>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, SValue::Opaque value);
						void						set(const CString& key, const SValue& value);
						void						set(const CString& key, const OV<SValue>& value)
														{
															// Check for value
															if (value.hasValue())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}
						void						set(const CString& key, const OR<SValue>& value)
														{
															// Check for value
															if (value.hasReference())
																// Have value
																set(key, *value);
															else
																// Don't have value
																remove(key);
														}

						void						remove(const CString& key);
						void						remove(const TArray<CString>& keys);
						void						remove(const TSet<CString>& keys);
						CDictionary					removing(const TArray<CString>& keys);
						CDictionary					removing(const TSet<CString>& keys);
						void						removeAll();

						bool						equals(const CDictionary& other,
															void* itemCompareProcUserData = nil) const;

						Iterator					getIterator() const;
						KeyIterator					getKeyIterator() const;
						ValueIterator				getValueIterator() const;

				const	OR<SValue>					operator[](const CString& key) const;
						CDictionary&				operator=(const CDictionary& other);
						CDictionary					operator+(const CDictionary& other) const;
						CDictionary&				operator+=(const CDictionary& other);

	protected:
													// Instance methods
						I<IteratorInfo>				getIteratorInfo() const;

	// Properties
	public:
		static	const	CDictionary	mEmpty;

	private:
						Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TDictionary

template <typename T> class TDictionary : public CDictionary {
	// Iterator
	public:
		class Iterator : public CDictionary::Iterator {
			// Methods
			public:
					// Lifecycle methods
					Iterator(const I<IteratorInfo>& iteratorInfo) : CDictionary::Iterator(iteratorInfo) {}
					Iterator(const Iterator& other) : CDictionary::Iterator(other) {}

					// Instance methods
				T&	getValue() const
						{ return *((T*) CDictionary::Iterator::getValue().getOpaque()); }
		};

	// ValueIterator
	public:
		class ValueIterator : public CDictionary::Iterator {
			// Methods
			public:
					// Lifecycle methods
					ValueIterator(const I<IteratorInfo>& iteratorInfo) : CDictionary::Iterator(iteratorInfo) {}
					ValueIterator(const ValueIterator& other) : CDictionary::Iterator(other) {}

					// Instance methods
				T&	operator*() const
						{ return *((T*) getValue().getOpaque()); }
				T*	operator->() const
						{ return (T*) getValue().getOpaque(); }
		};

	// Methods
	public:
								// Instance methods
		const	OR<T>			get(const CString& key) const
									{
										// Get opaque
										OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);

										return opaque.hasValue() ? OR<T>(*((T*) *opaque)) : OR<T>();
									}
		const	T&				get(const CString& key, const T& defaultValue) const
									{
										// Get opaque
										OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);

										return opaque.hasValue() ? *((T*) *opaque) : defaultValue;
									}
				Iterator		getIterator() const
									{ return Iterator(getIteratorInfo()); }
				ValueIterator	getValueIterator() const
									{ return ValueIterator(getIteratorInfo()); }

		const	OR<T>			operator[](const CString& key) const
									{ return get(key); }

	protected:
								// Lifecycle methods
								TDictionary(SValue::OpaqueCopyProc opaqueCopyProc,
										SValue::OpaqueEqualsProc opaqueEqualsProc,
										SValue::OpaqueDisposeProc opaqueDisposeProc) :
									CDictionary(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc)
									{}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMDictionary (TDictionary which can be modified)

template <typename T> class TMDictionary : public TDictionary<T> {
	// Methods
	protected:
				// Lifecycle methods
				TMDictionary(SValue::OpaqueCopyProc opaqueCopyProc, SValue::OpaqueEqualsProc opaqueEqualsProc,
						SValue::OpaqueDisposeProc opaqueDisposeProc) :
					TDictionary<T>(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc)
					{}
				TMDictionary(SValue::OpaqueCopyProc opaqueCopyProc, SValue::OpaqueEqualsProc opaqueEqualsProc,
						SValue::OpaqueDisposeProc opaqueDisposeProc, const TDictionary<T>& other,
						CDictionary::Item::IncludeProc itemIncludeProc, void* userData) :
					TDictionary<T>(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc)
					{
						for (CDictionary::Iterator iterator = other.getIterator; iterator; iterator++) {
							// Call proc
							if (itemIncludeProc(iterator.getKey(), iterator.getValue(), userData))
								// Add item
								CDictionary::set(iterator.getKey(), iterator.getValue());
						}
					}
				TMDictionary(const TDictionary<T>& other) : TDictionary<T>(other) {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNDictionary (TMDictionary where copy happens through new T())

template <typename T> class TNDictionary : public TMDictionary<T> {
	// Types
	public:
		typedef	bool	(*KeyIsMatchProc)(const CString& key, void* userData);

	// Methods
	public:
						// Lifecycle methods
						TNDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
							TMDictionary<T>((SValue::OpaqueCopyProc) copy, opaqueEqualsProc,
									(SValue::OpaqueDisposeProc) dispose)
							{}
						TNDictionary(const TDictionary<T>& other, CDictionary::Item::IncludeProc itemIncludeProc,
								void* userData) :
							TMDictionary<T>((SValue::OpaqueCopyProc) copy, nil, (SValue::OpaqueDisposeProc) dispose,
									other, itemIncludeProc, userData)
							{}
						TNDictionary(const TDictionary<T>& other) : TMDictionary<T>(other) {}

						// Instance methods
		void			set(const CString& key, const T& item)
							{ CDictionary::set(key, new T(item)); }
		void			set(const CString& key, const OV<T>& item)
							{
								// Check for instance
								if (item.hasValue())
									// Set
									CDictionary::set(key, new T(*item));
								else
									// Remove
									CDictionary::remove(key);
							}

		TNDictionary<T>	filtered(KeyIsMatchProc keyIsMatchProc, void* userData = nil)
							{
								// Setup
								TNDictionary<T>	dictionary;

								// Iterate items
								for (typename TDictionary<T>::Iterator iterator = TDictionary<T>::getIterator();
										iterator; iterator++) {
									// Check if match
									if (keyIsMatchProc(iterator.getKey(), userData))
										// A match
										dictionary.set(iterator.getKey(), iterator.getValue());
								}

								return dictionary;
							}

		TNSet<T>		getValues() const
							{
								// Setup
								TNSet<T>	values;

								// Iterate values
								for (typename TDictionary<T>::ValueIterator iterator =
												TDictionary<T>::getValueIterator();
										iterator; iterator++)
									// Add value
									values += *iterator;

								return values;
							}

	private:
						// Class methods
		static	T*		copy(SValue::Opaque opaque)
							{ return new T(*((T*) opaque)); }
		static	void	dispose(SValue::Opaque opaque)
							{ T* t = (T*) opaque; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNArrayDictionary (TNDictionary where values are arrays)

template <typename T> class TNArrayDictionary : public TNDictionary<TNArray<T> > {
	// Methods
	public:
				// Instance methods
		void	add(const CString& key, const T& item)
					{
						// Update
						const	OR<TNArray<T> >	array = TNDictionary<TNArray<T> >::get(key);
						if (array.hasReference())
							// Already have an array
							*array += item;
						else
							// First one
							TNDictionary<TNArray<T> >::set(key, TNArray<T>(item));
					}
		void	add(const CString& key, const TArray<T>& items)
					{
						// Update
						const	OR<TNArray<T> >	array = TNDictionary<TNArray<T> >::get(key);
						if (array.hasReference())
							// Already have an array
							*array += items;
						else
							// First one
							TNDictionary<TNArray<T> >::set(key, TNArray<T>(items));
					}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNSetDictionary (TNDictionary where values are sets)

template <typename T> class TNSetDictionary : public TNDictionary<TNSet<T> > {
	// Methods
	public:
				// Instance methods
		void	insert(const CString& key, const T& item)
					{
						// Update
						const	OR<TNSet<T> >	set = TNDictionary<TNSet<T> >::get(key);
						if (set.hasReference())
							// Already have a set
							*set += item;
						else
							// First one
							TNDictionary<TNSet<T> >::set(key, TNSet<T>(item));
					}
		void	insert(const CString& key, const TArray<T>& items)
					{
						// Update
						const	OR<TNSet<T> >	set = TNDictionary<TNSet<T> >::get(key);
						if (set.hasReference())
							// Already have a set
							*set += items;
						else
							// First one
							TNDictionary<TNSet<T> >::set(key, TNSet<T>(items));
					}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TReferenceDictionary

template <typename T> class TReferenceDictionary : public CDictionary {
	// Methods
	public:
						// Lifecycle methods
						TReferenceDictionary() : CDictionary() {}
						TReferenceDictionary(const TReferenceDictionary& dictionary) : CDictionary(dictionary) {}

						// Instance methods
		const	OR<T>	get(const CString& key) const
							{
								// Get opaque
								OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);

								return opaque.hasValue() ? OR<T>(*((T*) *opaque)) : OR<T>();
							}
				void	set(const CString& key, const T& item)
							{ CDictionary::set(key, &item); }

		const	OR<T>	operator[](const CString& key) const
							{ return get(key); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TKeyConvertibleDictionary

template <typename K, typename T> class TKeyConvertibleDictionary : public TNDictionary<T> {
	// Iterator
	public:
		class Iterator : public CDictionary::Iterator {
			// Methods
			public:
					// Lifecycle methods
					Iterator(const I<CDictionary::IteratorInfo>& iteratorInfo) : CDictionary::Iterator(iteratorInfo) {}
					Iterator(const Iterator& other) : CDictionary::Iterator(other) {}

					// Instance methods
				K	getKey() const
						{
							// Get key
							const	CString& 	key = CDictionary::Iterator::getKey();

							// Get value
							K	value;
							key.getValue(value);

							return value;
						}
				T&	getValue() const
						{ return *((T*) CDictionary::Iterator::getValue().getOpaque()); }
		};

	// ValueIterator
	public:
		class ValueIterator : public CDictionary::Iterator {
			// Methods
			public:
					// Lifecycle methods
					ValueIterator(const I<CDictionary::IteratorInfo>& iteratorInfo) :
						CDictionary::Iterator(iteratorInfo)
						{}
					ValueIterator(const ValueIterator& other) : CDictionary::Iterator(other) {}

					// Instance methods
				T&	operator*() const
						{ return *((T*) CDictionary::Iterator::getValue().getOpaque()); }
				T*	operator->() const
						{ return (T*) CDictionary::Iterator::getValue().getOpaque(); }
		};

	// Methods
	public:
								// Instance methods
		const	OR<T>			get(K key) const
									{ return TNDictionary<T>::get(CString(key)); }

				Iterator		getIterator() const
									{ return Iterator(CDictionary::getIteratorInfo()); }
				ValueIterator	getValueIterator() const
									{ return ValueIterator(CDictionary::getIteratorInfo()); }

		const	OR<T>			operator[](K key) const
									{ return get(key); }

	protected:
								// Lifecycle methods
								TKeyConvertibleDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
									TNDictionary<T>(opaqueEqualsProc)
									{}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNKeyConvertibleDictionary

template <typename K, typename T> class TNKeyConvertibleDictionary : public TKeyConvertibleDictionary<K, T> {
	// Methods
	public:
				// Lifecycle methods
				TNKeyConvertibleDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
					TKeyConvertibleDictionary<K, T>(opaqueEqualsProc)
					{}
				TNKeyConvertibleDictionary(const TKeyConvertibleDictionary<K, T>& other) :
					TKeyConvertibleDictionary<K, T>(other)
					{}

				// Instance methods
		void	set(K key, const T& item)
					{ TNDictionary<T>::set(CString(key), item); }

		void	remove(K key)
					{ TNDictionary<T>::remove(CString(key)); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNArrayKeyConvertibleDictionary

template <typename K, typename T> class TNArrayKeyConvertibleDictionary :
		public TNKeyConvertibleDictionary<K, TNArray<T> > {
	// Methods
	public:
				// Instance methods
		void	add(K key, const T& item)
					{
						// Setup
						CString	keyString(key);

						// Update
						const	OR<TNArray<T> >	array = TNKeyConvertibleDictionary<K, TNArray<T> >::get(key);
						if (array.hasReference())
							// Already have array
							*array += item;
						else
							// First one
							TNKeyConvertibleDictionary<K, TNArray<T> >::set(key, TNArray<T>(item));
					}
};
