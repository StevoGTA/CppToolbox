//----------------------------------------------------------------------------------------------------------------------
//	SGPUVertexBuffer.h			©2020 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CUUID.h"
#include "SGPUBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUVertexBufferInfo

struct SGPUVertexBufferInfo {

	// Lifecycle methods
	SGPUVertexBufferInfo(const CUUID& uuid, UInt8 vertexCount, UInt8 vertexOffset, UInt8 textureCoordinateCount,
			UInt8 textureCoordinateOffset, UInt8 textureIndexOffset, UInt8 totalSize) :
		mUUID(uuid), mVertexCount(vertexCount), mVertexOffset(vertexOffset),
				mTextureCoordinateCount(textureCoordinateCount), mTextureCoordinateOffset(textureCoordinateOffset),
				mTextureIndexOffset(textureIndexOffset), mTotalSize(totalSize)
		{}

	// Properties
	const	CUUID&	mUUID;
			UInt8	mVertexCount;
			UInt8	mVertexOffset;
			UInt8	mTextureCoordinateCount;
			UInt8	mTextureCoordinateOffset;
			UInt8	mTextureIndexOffset;
			UInt8	mTotalSize;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUVertexBuffer

struct SGPUVertexBuffer : SGPUBuffer {
	// Lifecycle methods
	SGPUVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo, void* internalReference) :
		SGPUBuffer(internalReference), mGPUVertexBufferInfo(gpuVertexBufferInfo)
		{}

	// Properties
	const	SGPUVertexBufferInfo&	mGPUVertexBufferInfo;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TGPUBufferTypeVertex2DTexture2D

template <typename T> struct TGPUBufferTypeVertex2DTexture2D {

											// Lifecycle methods
											TGPUBufferTypeVertex2DTexture2D(T2DPoint<T> vertex, T2DPoint<T> texture,
													UInt8 textureIndex) :
												mVertex(vertex), mTexture(texture), mTextureIndex(textureIndex)
												{}

											// Class methods
	static	const	SGPUVertexBufferInfo&	getGPUVertexBufferInfo()
												{
													// Setup
													static	SGPUVertexBufferInfo*	sSGPUVertexBufferInfo = nil;

													// Check if have initialized
													if (sSGPUVertexBufferInfo == nil)
														// Initialize
														sSGPUVertexBufferInfo =
																new SGPUVertexBufferInfo(mUUID, 2, 0, 2, 2 * sizeof(T),
																		4 * sizeof(T), 5 * sizeof(T));

													return *sSGPUVertexBufferInfo;
												}

	// Properties
	static	CUUID		mUUID;

			T2DPoint<T>	mVertex;
			T2DPoint<T>	mTexture;
			T			mTextureIndex;
};

template<typename T>	CUUID	TGPUBufferTypeVertex2DTexture2D<T>::mUUID;

typedef	TGPUBufferTypeVertex2DTexture2D<Float32>	SGPUBufferTypeVertex2DTexture2D;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TGPUBufferTypeVertex3DTexture2D

template <typename T> class TGPUBufferTypeVertex3DTexture2D {

											// Lifecycle methods
											TGPUBufferTypeVertex3DTexture2D(T3DPoint<T> vertex, T2DPoint<T> texture,
													UInt8 textureIndex) :
												mVertex(vertex), mTexture(texture), mTextureIndex(textureIndex)
												{}

											// Class methods
	static	const	SGPUVertexBufferInfo&	getGPUVertexBufferInfo()
												{
													// Setup
													static	SGPUVertexBufferInfo*	sSGPUVertexBufferInfo = nil;

													// Check if have initialized
													if (sSGPUVertexBufferInfo == nil)
														// Initialize
														sSGPUVertexBufferInfo =
																new SGPUVertexBufferInfo(mUUID, 3, 0, 2, 3 * sizeof(T),
																		5 * sizeof(T), 6 * sizeof(T));

													return *sSGPUVertexBufferInfo;
												}

	// Properties
	static	CUUID		mUUID;

			T3DPoint<T>	mVertex;
			T2DPoint<T>	mTexture;
			T			mTextureIndex;
};

template<typename T>	CUUID	TGPUBufferTypeVertex3DTexture2D<T>::mUUID;

typedef	TGPUBufferTypeVertex3DTexture2D<Float32>	SGPUBufferTypeVertex3DTexture2D;
