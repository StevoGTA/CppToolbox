//----------------------------------------------------------------------------------------------------------------------
//	CAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "CCodec.h"
#include "CDataSource.h"
#include "SAudioFormats.h"
#include "SAudioSourceStatus.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioCodec

class CAudioCodec : public CCodec {
	// FrameSourceDecodeInfo
	public:
		class FrameSourceDecodeInfo : public DecodeInfo {
			// Methods
			public:
										// Lifecycle methods
										FrameSourceDecodeInfo(const I<CSeekableDataSource>& seekableDataSource,
												UInt64 startByteOffset, UInt64 byteCount, UInt8 frameByteCount) :
											DecodeInfo(), mSeekableDataSource(seekableDataSource),
													mStartByteOffset(startByteOffset), mByteCount(byteCount),
													mFrameByteCount(frameByteCount),
													mCurrentPosition(mStartByteOffset)
											{}

										// Instance methods
					UInt8				getFrameByteCount() const
											{ return mFrameByteCount; }

					void				seek(UInt64 frameIndex)
											{ mCurrentPosition = mStartByteOffset + frameIndex * mFrameByteCount; }
					TVResult<UInt32>	readInto(void* buffer, UInt32 frameCount)
											{
												// Check situation
												UInt64	framesRemaining =
																(mStartByteOffset + mByteCount - mCurrentPosition) /
																		mFrameByteCount;
												if (framesRemaining < frameCount)
													// Limit frame count to what is left
													frameCount = (UInt32) framesRemaining;
												if (frameCount > 0) {
													// Read
													OI<SError>	error =
																		mSeekableDataSource->readData(mCurrentPosition,
																				buffer, frameCount * mFrameByteCount);
													ReturnValueIfError(error, TVResult<UInt32>(*error));

													// Success
													mCurrentPosition += frameCount * mFrameByteCount;

													return TVResult<UInt32>(frameCount);
												} else
													// End of data
													return TVResult<UInt32>(SError::mEndOfData);
											}
					TVResult<UInt32>	readInto(CAudioFrames& audioFrames)
											{
												// Read
												CAudioFrames::Info	writeInfo = audioFrames.getWriteInfo();
												UInt8*				buffer = (UInt8*) writeInfo.getSegments()[0];
												TVResult<UInt32>	frameCount =
																			readInto(buffer, writeInfo.getFrameCount());
												ReturnResultIfResultError(frameCount);

												// Complete write
												audioFrames.completeWrite(frameCount.getValue());

												return frameCount;
											}
					TVResult<UInt32>	readInto(CData& data)
											{ return readInto(data.getMutableBytePtr(),
													(UInt32) data.getByteCount() / mFrameByteCount); }

			// Properties
			private:
				I<CSeekableDataSource>	mSeekableDataSource;
				UInt64					mStartByteOffset;
				UInt64					mByteCount;
				UInt8					mFrameByteCount;

				UInt64					mCurrentPosition;
		};

	// Info
	public:
		struct Info {
			// Procs
			typedef	TArray<SAudioProcessingSetup>	(*GetAudioProcessingSetupsProc)(OSType id,
															const SAudioStorageFormat& audioStorageFormat);
			typedef	I<CAudioCodec>					(*InstantiateProc)(OSType id);

													// Lifecycle methods
													Info(OSType id, const CString& name,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(name), mEncodeName(name),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													Info(OSType id, const CString& name,
															const EncodeSettings& encodeSettings,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(name), mEncodeName(name),
																mEncodeSettings(OI<EncodeSettings>(encodeSettings)),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													Info(OSType id, const CString& decodeName,
															const CString& encodeName,
															const EncodeSettings& encodeSettings,
															GetAudioProcessingSetupsProc getAudioProcessingSetupsProc,
															InstantiateProc instantiateProc) :
														mID(id), mDecodeName(decodeName), mEncodeName(encodeName),
																mEncodeSettings(OI<EncodeSettings>(encodeSettings)),
																mGetAudioProcessingSetupsProc(
																		getAudioProcessingSetupsProc),
																mInstantiateProc(instantiateProc)
														{}
													Info(const Info& other) :
														mID(other.mID), mDecodeName(other.mDecodeName),
																mEncodeName(other.mEncodeName),
																mEncodeSettings(other.mEncodeSettings),
																mGetAudioProcessingSetupsProc(
																		other.mGetAudioProcessingSetupsProc),
																mInstantiateProc(other.mInstantiateProc)
														{}

													// Instance methods
			OSType									getID() const
														{ return mID; }
			const	CString&						getDecodeName() const
														{ return mDecodeName; }
			const	CString&						getEncodeName() const
														{ return mEncodeName; }
					I<CAudioCodec>					instantiate() const
														{ return mInstantiateProc(mID); }
					TArray<SAudioProcessingSetup>	getAudioProcessingSetups(
															const SAudioStorageFormat& audioStorageFormat) const
														{ return mGetAudioProcessingSetupsProc(mID,
																audioStorageFormat); }

			// Properties
			private:
				OSType							mID;
				CString							mDecodeName;
				CString							mEncodeName;
				OI<EncodeSettings>				mEncodeSettings;
				InstantiateProc					mInstantiateProc;
				GetAudioProcessingSetupsProc	mGetAudioProcessingSetupsProc;
		};

	// Methods
	public:
												// Lifecycle methods
												~CAudioCodec() {}

												// Instance methods
		virtual	OI<SError>						setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
														const I<CCodec::DecodeInfo>& decodeInfo) = 0;
		virtual	CAudioFrames::Requirements		getRequirements() const = 0;
		virtual	void							seek(UniversalTimeInterval timeInterval) = 0;
		virtual	OI<SError>						decodeInto(CAudioFrames& audioFrames) = 0;

		virtual	TArray<SAudioProcessingSetup>	getEncodeAudioProcessingSetups() const = 0;
		virtual	void							setupForEncode(const SAudioProcessingFormat& audioProcessingFormat) = 0;

	protected:
												// Lifecycle methods
												CAudioCodec() : CCodec() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDecodeOnlyAudioCodec

class CDecodeOnlyAudioCodec : public CAudioCodec {
	// Methods
	public:
										// CAudioCodec methods
		TArray<SAudioProcessingSetup>	getEncodeAudioProcessingSetups() const
											{ AssertFailUnimplemented(); return TNArray<SAudioProcessingSetup>(); }
		void							setupForEncode(const SAudioProcessingFormat& audioProcessingFormat)
											{ AssertFailUnimplemented(); }

	protected:
										// Lifecycle methods
										CDecodeOnlyAudioCodec() : CAudioCodec() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CEncodeOnlyAudioCodec

class CEncodeOnlyAudioCodec : public CAudioCodec {
	// Methods
	public:
					// CAudioCodec methods
		OI<SError>	setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
							const I<CCodec::DecodeInfo>& decodeInfo)
						{ AssertFailUnimplemented(); return OI<SError>(SError::mUnimplemented); }
		void		seek(UniversalTimeInterval timeInterval)
						{ AssertFailUnimplemented(); }
		OI<SError>	decodeInto(CAudioFrames& audioFrames)
						{ AssertFailUnimplemented(); return OI<SError>(SError::mUnimplemented); }

	protected:
					// Lifecycle methods
					CEncodeOnlyAudioCodec() : CAudioCodec() {}
};
