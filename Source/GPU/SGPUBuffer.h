//----------------------------------------------------------------------------------------------------------------------
//	SGPUBuffer.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUBuffer

struct SGPUBuffer {
	// Lifecycle methods
	SGPUBuffer(CData data, void* internalReference = nil) : mData(data), mInternalReference(internalReference) {}
	SGPUBuffer(const SGPUBuffer& other) : mData(other.mData), mInternalReference(other.mInternalReference) {}

	// Properties
	CData	mData;
	void*	mInternalReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUVertexBufferInfo

struct SGPUVertexBufferInfo {

	// Lifecycle methods
	SGPUVertexBufferInfo(UInt8 vertexCount, UInt8 vertexOffset, UInt8 textureCoordinateCount,
			UInt8 textureCoordinateOffset, UInt8 totalSize) :
		mVertexCount(vertexCount), mVertexOffset(vertexOffset), mTextureCoordinateCount(textureCoordinateCount),
				mTextureCoordinateOffset(textureCoordinateOffset), mTotalSize(totalSize)
		{}

	// Properties
	UInt8	mVertexCount;
	UInt8	mVertexOffset;
	UInt8	mTextureCoordinateCount;
	UInt8	mTextureCoordinateOffset;
	UInt8	mTotalSize;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TGPUVertexType2Vertex2Texture

template <typename T> struct TGPUVertexType2Vertex2Texture {

											// Lifecycle methods
											TGPUVertexType2Vertex2Texture(T2DPoint<T> vertex, T2DPoint<T> texture,
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
																new SGPUVertexBufferInfo(2, 0, 2, 2 * sizeof(T),
																		5 * sizeof(T));

													return *sSGPUVertexBufferInfo;
												}

	// Properties
	T2DPoint<T>	mVertex;
	T2DPoint<T>	mTexture;
	T			mTextureIndex;
};

typedef	TGPUVertexType2Vertex2Texture<Float32>	SGPUVertexType2Vertex2Texture32;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TGPUVertexType3Vertex2Texture

template <typename T> class TGPUVertexType3Vertex2Texture {

											// Lifecycle methods
											TGPUVertexType3Vertex2Texture(T3DPoint<T> vertex, T2DPoint<T> texture,
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
																new SGPUVertexBufferInfo(3, 0, 2, 3 * sizeof(T),
																		6 * sizeof(T));

													return *sSGPUVertexBufferInfo;
												}

	// Properties
	T3DPoint<T>	mVertex;
	T2DPoint<T>	mTexture;
	T			mTextureIndex;
};

typedef	TGPUVertexType3Vertex2Texture<Float32>	SGPUVertexType3Vertex2Texture32;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUVertexBuffer

struct SGPUVertexBuffer : SGPUBuffer {
	// Lifecycle methods
	SGPUVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo, CData data, void* internalReference) :
		SGPUBuffer(data, internalReference), mGPUVertexBufferInfo(gpuVertexBufferInfo)
		{}
	SGPUVertexBuffer(const SGPUVertexBuffer& other) :
		SGPUBuffer(other), mGPUVertexBufferInfo(other.mGPUVertexBufferInfo)
		{}

	// Properties
	const	SGPUVertexBufferInfo&	mGPUVertexBufferInfo;
};
