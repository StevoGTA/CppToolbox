//----------------------------------------------------------------------------------------------------------------------
//	CImage.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CImage

class CImage {
	// Enums
	public:
		enum Type {
			kTypeJPEG	= MAKE_OSTYPE('J', 'P', 'E', 'G'),
			kTypePNG	= MAKE_OSTYPE('P', 'N', 'G', ' '),
			kTypeNV12	= MAKE_OSTYPE('N', 'V', '1', '2'),
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
									// Lifecycle methods
									CImage(const CData& data, const OV<Type>& type = OV<Type>(),
											const OV<S2DSizeS32>& size = OV<S2DSizeS32>());
									CImage(const CImage& other);
									~CImage();

									// Instance methods
				TVResult<CBitmap>	getBitmap() const;
				OV<SError>			decodeInto(CBitmap& bitmap, const S2DRectS32& rect) const;

									// Class methods
		static	OV<Type>			getTypeFromResourceName(const CString& resourceName);
		static	OV<Type>			getTypeFromMIMEType(const CString& MIMEType);
		static	OV<Type>			getTypeFromData(const CData& data);
		static	OV<CString>			getDefaultFilenameExtension(Type type);
		static	OV<CString>			getMIMEType(Type type);

		static	TVResult<CBitmap>	getBitmap(const CData& data)
										{ return CImage(data).getBitmap(); }
		static	TVResult<CBitmap>	getBitmap(const CData& data, Type type)
										{ return CImage(data, OV<Type>(type)).getBitmap(); }
		static	TVResult<CBitmap>	getBitmap(const CData& data, const OV<Type>& type)
										{ return CImage(data, type).getBitmap(); }
		static	TVResult<CData>		getData(const CBitmap& bitmap, Type type);

	// Properties
	private:
		Internals*	mInternals;
};
