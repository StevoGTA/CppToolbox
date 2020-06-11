//----------------------------------------------------------------------------------------------------------------------
//	SGPUVertexBuffer.h			©2020 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SGPUBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUVertexBufferInfo

struct SGPUVertexBufferInfo {

	// Lifecycle methods
	SGPUVertexBufferInfo(UInt8 vertexCount, UInt8 vertexOffset, UInt8 textureCoordinateCount,
			UInt8 textureCoordinateOffset, UInt8 textureIndexOffset, UInt8 totalSize) :
		mVertexCount(vertexCount), mVertexOffset(vertexOffset), mTextureCoordinateCount(textureCoordinateCount),
				mTextureCoordinateOffset(textureCoordinateOffset), mTextureIndexOffset(textureIndexOffset),
				mTotalSize(totalSize)
		{}

	// Properties
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
																		4 * sizeof(T), 5 * sizeof(T));

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
																		5 * sizeof(T), 6 * sizeof(T));

													return *sSGPUVertexBufferInfo;
												}

	// Properties
	T3DPoint<T>	mVertex;
	T2DPoint<T>	mTexture;
	T			mTextureIndex;
};

typedef	TGPUVertexType3Vertex2Texture<Float32>	SGPUVertexType3Vertex2Texture32;
