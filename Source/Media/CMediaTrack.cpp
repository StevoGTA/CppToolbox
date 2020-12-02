//----------------------------------------------------------------------------------------------------------------------
//	CMediaTrack.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaTrack.h"

#include "TReferenceTracking.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaTrackInternals

//class CMediaTrackInternals : public TReferenceCountable<CMediaTrackInternals> {
//	public:
//		CMediaTrackInternals(void* externalReference,
//				CMediaTrack::ExternalReferenceDeleteProc externalReferenceDeleteProc) :
//			TReferenceCountable(),
//					mExternalReference(externalReference), mExternalReferenceDeleteProc(externalReferenceDeleteProc)
//			{
//			}
//		~CMediaTrackInternals()
//			{
//				// Cleanup
//				if (mExternalReferenceDeleteProc != nil)
//					// Call proc
//					mExternalReferenceDeleteProc(mExternalReference);
//			}
//
//		void*										mExternalReference;
//		CMediaTrack::ExternalReferenceDeleteProc	mExternalReferenceDeleteProc;
//};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaTrack

// MARK: Lifecycle methods

////----------------------------------------------------------------------------------------------------------------------
//CMediaTrack::CMediaTrack(void* externalReference, ExternalReferenceDeleteProc externalReferenceDeleteProc)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Setup
//	mInternals = new CMediaTrackInternals(externalReference, externalReferenceDeleteProc);
//}
//
////----------------------------------------------------------------------------------------------------------------------
//CMediaTrack::CMediaTrack(const CMediaTrack& other)
////----------------------------------------------------------------------------------------------------------------------
//{
//	mInternals = other.mInternals->addReference();
//}
//
////----------------------------------------------------------------------------------------------------------------------
//CMediaTrack::~CMediaTrack()
////----------------------------------------------------------------------------------------------------------------------
//{
//	mInternals->removeReference();
//}

// MARK: Instance methods

