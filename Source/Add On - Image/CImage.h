//----------------------------------------------------------------------------------------------------------------------
//	CImage.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CImage

class CImageInternals;
class CImage {
	// Enums
	public:
		enum Type {
			kTypeJPEG	= MAKE_OSTYPE('J', 'P', 'E', 'G'),
			kTypePNG	= MAKE_OSTYPE('P', 'N', 'G', ' '),
			kTypeNV12	= MAKE_OSTYPE('N', 'V', '1', '2'),
		};

	// Methods
	public:
									// Lifecycle methods
									CImage(const CData& data, const OV<Type>& type = OV<Type>(),
											const OV<S2DSizeS32>& size = OV<S2DSizeS32>());
									CImage(const CImage& other);
									~CImage();

									// Instance methods
				TIResult<CBitmap>	getBitmap() const;
				OI<SError>			decodeInto(CBitmap& bitmap, const S2DRectS32& rect) const;

									// Class methods
		static	OV<Type>			getTypeFromResourceName(const CString& resourceName);
		static	OV<Type>			getTypeFromMIMEType(const CString& MIMEType);
		static	OV<Type>			getTypeFromData(const CData& data);
		static	OI<CString>			getDefaultFilenameExtension(Type type);
		static	OI<CString>			getMIMEType(Type type);

	// Properties
	private:
		CImageInternals*	mInternals;
};
