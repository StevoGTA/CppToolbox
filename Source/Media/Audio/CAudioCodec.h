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
	// Info
	public:
		struct Info {
								// Lifecycle methods
								Info(OSType id, const CString& name) : mID(id), mDecodeName(name), mEncodeName(name) {}
								Info(OSType id, const CString& name, const EncodeSettings& encodeSettings) :
									mID(id), mDecodeName(name), mEncodeName(name),
											mEncodeSettings(OI<EncodeSettings>(encodeSettings))
									{}
								Info(OSType id, const CString& decodeName, const CString& encodeName,
										const EncodeSettings& encodeSettings) :
									mID(id), mDecodeName(decodeName), mEncodeName(encodeName),
											mEncodeSettings(OI<EncodeSettings>(encodeSettings))
									{}
								Info(const Info& other) :
									mID(other.mID), mDecodeName(other.mDecodeName), mEncodeName(other.mEncodeName),
											mEncodeSettings(other.mEncodeSettings)
									{}

								// Instance methods
					OSType		getID() const
									{ return mID; }
			const	CString&	getDecodeName() const
									{ return mDecodeName; }
			const	CString&	getEncodeName() const
									{ return mEncodeName; }

			// Properties
			private:
				OSType				mID;
				CString				mDecodeName;
				CString				mEncodeName;
				OI<EncodeSettings>	mEncodeSettings;
		};

	// Methods
	public:
		// Lifecycle methods
		~CAudioCodec() {}

	protected:
		// Lifecycle methods
		CAudioCodec() : CCodec() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDecodeAudioCodec

class CDecodeAudioCodec : public CAudioCodec {
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
												audioFrames.completeWrite(*frameCount);

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

	// Methods
	public:
												// Instance methods
		virtual	TArray<SAudioProcessingSetup>	getAudioProcessingSetups(const SAudioStorageFormat& audioStorageFormat)
														= 0;
		virtual	OI<SError>						setup(const SAudioProcessingFormat& audioProcessingFormat) = 0;
		virtual	CAudioFrames::Requirements		getRequirements() const = 0;
		virtual	void							seek(UniversalTimeInterval timeInterval) = 0;
		virtual	OI<SError>						decodeInto(CAudioFrames& audioFrames) = 0;

	protected:
												// Lifecycle methods
												CDecodeAudioCodec() : CAudioCodec() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CEncodeAudioCodec

class CEncodeAudioCodec : public CAudioCodec {
	// Methods
	public:
												// Instance methods
		virtual	TArray<SAudioProcessingSetup>	getAudioProcessingSetups() const = 0;
		virtual	void							setup(const SAudioProcessingFormat& audioProcessingFormat) = 0;

	protected:
												// Lifecycle methods
												CEncodeAudioCodec() : CAudioCodec() {}
};
