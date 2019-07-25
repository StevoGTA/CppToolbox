//----------------------------------------------------------------------------------------------------------------------
//	CDataProvider.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDataProvider.h"

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataProviderSetup

class CDataProviderSetup {
	public:
		CDataProviderSetup()
			{
				CErrorRegistry::registerError(kDataProviderReadBeyondEndError,
						CString("Read beyond end"));
				CErrorRegistry::registerError(kDataProviderSetPosBeforeStartError,
						CString("Set position before start"));
				CErrorRegistry::registerError(kDataProviderSetPosAfterEndError,
						CString("Set position after end"));
			}
};

static	CDataProviderSetup	sDataProviderSetup;

////----------------------------------------------------------------------------------------------------------------------
////----------------------------------------------------------------------------------------------------------------------
//// MARK: CDataProviderInternals
//
//class CDataProviderInternals {
//	public:
//		CDataProviderInternals() : mReferenceCount(1) {}
//		~CDataProviderInternals() {}
//
//		CDataProviderInternals*	addReference()
//									{ mReferenceCount++; return this; }
//		void					removeReference()
//									{
//										// Remove reference and see if we are the last one
//										if (--mReferenceCount == 0) {
//											// Last one
//											CDataProviderInternals*	THIS = this;
//											DisposeOf(THIS);
//										}
//									}
//
//		UInt32	mReferenceCount;
//};
//
////----------------------------------------------------------------------------------------------------------------------
////----------------------------------------------------------------------------------------------------------------------
//// MARK: CDataProvider
//
//// MARK: Lifecycle methods
//
////----------------------------------------------------------------------------------------------------------------------
//CDataProvider::CDataProvider()
////----------------------------------------------------------------------------------------------------------------------
//{
//	mInternals = new CDataProviderInternals();
//}
//
////----------------------------------------------------------------------------------------------------------------------
//CDataProvider::CDataProvider(const CDataProvider& other)
////----------------------------------------------------------------------------------------------------------------------
//{
//	mInternals = other.mInternals->addReference();
//}
//
////----------------------------------------------------------------------------------------------------------------------
//CDataProvider::~CDataProvider()
////----------------------------------------------------------------------------------------------------------------------
//{
//	mInternals->removeReference();
//}
