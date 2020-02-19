//----------------------------------------------------------------------------------------------------------------------
//	CImage.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"
#include "CByteParceller.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Image Type

enum ImageType {
	kImageTypeJPEG	= MAKE_OSTYPE('J', 'P', 'E', 'G'),
	kImageTypePNG	= MAKE_OSTYPE('P', 'N', 'G', ' '),
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CImage

class CImageInternals;
class CImage {
	// Methods
	public:
								// Lifecycle methods
								CImage(const CByteParceller& byteParceller, OV<ImageType> imageType = OV<ImageType>());
								CImage(const CImage& other);
								~CImage();

								// Instance methods
				CBitmap			getBitmap() const;

								// Class methods
		static	CBitmap			getBitmap(const CByteParceller& byteParceller);

		static	OV<ImageType>	getImageTypeFromResourceName(const CString& resourceName);
		static	OV<ImageType>	getImageTypeFromMIMEType(const CString& MIMEType);
		static	OV<ImageType>	getImageTypeFromData(const CData& data);
		static	CString			getDefaultFilenameExtensionForImageType(ImageType imageType);
		static	CString			getMIMETypeForImageType(ImageType imageType);

	// Properties
	private:
		CImageInternals*	mInternals;
};
