//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDataSource.h"

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataSourceSetup

class CDataSourceSetup {
	public:
		CDataSourceSetup()
			{
				CErrorRegistry::registerError(kDataProviderReadBeyondEndError,
						CString("Read beyond end"));
				CErrorRegistry::registerError(kDataProviderSetPosBeforeStartError,
						CString("Set position before start"));
				CErrorRegistry::registerError(kDataProviderSetPosAfterEndError,
						CString("Set position after end"));
			}
};

static	CDataSourceSetup	sDataSourceSetup;

////----------------------------------------------------------------------------------------------------------------------
////----------------------------------------------------------------------------------------------------------------------
//// MARK: CDataSourceInternals
//
//class CDataSourceInternals {
//	public:
//		CDataSourceInternals() : mReferenceCount(1) {}
//		~CDataSourceInternals() {}
//
//		CDataSourceInternals*	addReference()
//									{ mReferenceCount++; return this; }
//		void					removeReference()
//									{
//										// Remove reference and see if we are the last one
//										if (--mReferenceCount == 0) {
//											// Last one
//											CDataSourceInternals*	THIS = this;
//											DisposeOf(THIS);
//										}
//									}
//
//		UInt32	mReferenceCount;
//};
//
////----------------------------------------------------------------------------------------------------------------------
////----------------------------------------------------------------------------------------------------------------------
//// MARK: CDataSource
//
//// MARK: Lifecycle methods
//
////----------------------------------------------------------------------------------------------------------------------
//CDataSource::CDataSource()
////----------------------------------------------------------------------------------------------------------------------
//{
//	mInternals = new CDataSourceInternals();
//}
//
////----------------------------------------------------------------------------------------------------------------------
//CDataSource::CDataSource(const CDataSource& other)
////----------------------------------------------------------------------------------------------------------------------
//{
//	mInternals = other.mInternals->addReference();
//}
//
////----------------------------------------------------------------------------------------------------------------------
//CDataSource::~CDataSource()
////----------------------------------------------------------------------------------------------------------------------
//{
//	mInternals->removeReference();
//}
