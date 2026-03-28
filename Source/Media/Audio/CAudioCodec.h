//----------------------------------------------------------------------------------------------------------------------
//	CAudioCodec.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioFrames.h"
#include "CCodec.h"
#include "CDataSource.h"
#include "SAudio.h"
#include "TimeAndDate.h"
#include "Tuple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioCodec

class CAudioCodec : public CCodec {
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
	// FrameSource
	public:
		class FrameSource {
			// FrameCountAndBuffer
			public:
				template <typename T> struct DecodeInfo : public TV2<UInt32, TBuffer<T> > {
					// Methods
					public:
											// Lifecycle methods
											DecodeInfo(UInt32 frameCount, const TBuffer<T>& buffer) :
												TV2<UInt32, TBuffer<T> >(frameCount, buffer)
												{}

											// Instance methods
								UInt32		getFrameCount() const
												{ return TV2<UInt32, TBuffer<T> >::getA(); }
						const	TBuffer<T>&	getBuffer() const
												{ return TV2<UInt32, TBuffer<T> >::getB(); }
				};

			// Methods
			public:
												// Lifecycle methods
												FrameSource(const I<CRandomAccessDataSource>& randomAccessDataSource,
														UInt64 startByteOffset, UInt64 byteCount,
														UInt8 frameByteCount) :
													mRandomAccessDataSource(randomAccessDataSource),
															mStartByteOffset(startByteOffset), mByteCount(byteCount),
															mFrameByteCount(frameByteCount),
															mCurrentPosition(mStartByteOffset)
													{}

												// Instance methods
				UInt8							getFrameByteCount() const
													{ return mFrameByteCount; }

				void							seek(UInt64 frameIndex)
													{ mCurrentPosition =
															mStartByteOffset + frameIndex * mFrameByteCount; }
				TVResult<UInt32>				readInto(CAudioFrames& audioFrames)
													{
														// Read
														CAudioFrames::Info	writeInfo = audioFrames.getWriteInfo();

														// Check situation
														UInt32	frameCount =
																		std::min(writeInfo.getFrameCount(),
																				(UInt32) (mStartByteOffset +
																						mByteCount - mCurrentPosition) /
																						mFrameByteCount);
														if (frameCount > 0) {
															// Read frames
															OV<SError>	error =
																				mRandomAccessDataSource->read(
																						mCurrentPosition,
																						(UInt8*) writeInfo
																								.getSegments()[0],
																						(UInt64) frameCount *
																								mFrameByteCount);
															ReturnValueIfError(error, TVResult<UInt32>(*error));

															// Success
															mCurrentPosition +=
																	(UInt64) frameCount * (UInt64) mFrameByteCount;

															// Complete write
															audioFrames.completeWrite(frameCount);

															return TVResult<UInt32>(frameCount);
														} else
															// End of data
															return TVResult<UInt32>(SError::mEndOfData);
													}
				TVResult<DecodeInfo<UInt8> >	readUInt8Frames(UInt32 frameCount)
													{
														// Check situation
														frameCount =
																std::min(frameCount,
																		(UInt32) (mStartByteOffset + mByteCount -
																				mCurrentPosition) / mFrameByteCount);
														if (frameCount > 0) {
															// Read frames
															TBuffer<UInt8>	buffer(frameCount * mFrameByteCount);

															OV<SError>	error =
																				mRandomAccessDataSource->read(
																						mCurrentPosition,
																						*buffer,
																						(UInt64) frameCount *
																								mFrameByteCount);
															ReturnValueIfError(error,
																	TVResult<DecodeInfo<UInt8> >(*error));

															// Success
															mCurrentPosition +=
																	(UInt64) frameCount * (UInt64) mFrameByteCount;

															return TVResult<DecodeInfo<UInt8> >(
																	DecodeInfo<UInt8>(frameCount, buffer));
														} else
															// End of data
															return TVResult<DecodeInfo<UInt8> >(SError::mEndOfData);
													}

			// Properties
			private:
				I<CRandomAccessDataSource>	mRandomAccessDataSource;
				UInt64						mStartByteOffset;
				UInt64						mByteCount;
				UInt8						mFrameByteCount;

				UInt64						mCurrentPosition;
		};

	// SourceInfo
	public:
		class SourceInfo {
			// Methods
			public:

														// Lifecycle methods
														SourceInfo(const SAudio::Format& audioFormat,
																UInt64 frameCount) :
															mAudioFormat(audioFormat), mFrameCount(frameCount)
															{}
														SourceInfo(const SourceInfo& other) :
															mAudioFormat(other.mAudioFormat),
																	mFrameCount(other.mFrameCount)
															{}
				virtual									~SourceInfo() {}

														// Instance methods
						const	SAudio::Format&			getAudioFormat() const
															{ return mAudioFormat; }
								UInt64					getFrameCount() const
															{ return mFrameCount; }
				virtual			I<CDecodeAudioCodec>	getDecodeAudioCodec() const = 0;

			// Properties
			protected:
				SAudio::Format	mAudioFormat;
				UInt64			mFrameCount;
		};

	// PacketSourceInfo
	public:
		class PacketSourceInfo : public SourceInfo {
			// Methods
			public:

				// Lifecycle methods
				PacketSourceInfo(const SAudio::Format& audioFormat, UInt64 frameCount,
						const I<CRandomAccessDataSource>& randomAccessDataSource,
						const TArray<SMedia::PacketAndLocation>& mediaPacketAndLocations) :
					SourceInfo(audioFormat, frameCount),
							mRandomAccessDataSource(randomAccessDataSource),
							mMediaPacketAndLocations(mediaPacketAndLocations)
					{}
				PacketSourceInfo(const PacketSourceInfo& other) :
					SourceInfo(other),
							mRandomAccessDataSource(other.mRandomAccessDataSource),
							mMediaPacketAndLocations(other.mMediaPacketAndLocations)
					{}

			// Properties
			protected:
				I<CRandomAccessDataSource>			mRandomAccessDataSource;
				TArray<SMedia::PacketAndLocation>	mMediaPacketAndLocations;
		};

	// Methods
	public:
												// Instance methods
		virtual	TArray<SAudio::ProcessingSetup>	getAudioProcessingSetups(const SAudio::Format& audioFormat)
														= 0;
		virtual	OV<SError>						setup(const SAudio::ProcessingFormat& audioProcessingFormat)
													{ return OV<SError>(); }
		virtual	CAudioFrames::Requirements		getRequirements() const
													{ return CAudioFrames::Requirements(1, 1); }
		virtual	void							seek(UniversalTimeInterval timeInterval) = 0;
		virtual	OV<SError>						decodeInto(CAudioFrames& audioFrames) = 0;

	protected:
												// Lifecycle methods
												CDecodeAudioCodec() : CAudioCodec() {}
};
