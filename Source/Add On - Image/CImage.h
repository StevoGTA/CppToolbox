//----------------------------------------------------------------------------------------------------------------------
//	CImage.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CImage

class CImageInternals;
class CImage {
	// Enums
	public:
		enum Type {
			kTypeJPEG	= MAKE_OSTYPE('J', 'P', 'E', 'G'),
			kTypePNG	= MAKE_OSTYPE('P', 'N', 'G', ' '),
		};

	// Methods
	public:
							// Lifecycle methods
							CImage(const CData& data, OV<Type> type = OV<Type>());
							CImage(const CImage& other);
							~CImage();

							// Instance methods
				CBitmap		getBitmap() const;

							// Class methods
		static	CBitmap		getBitmap(const CData& data);

		static	OV<Type>	getTypeFromResourceName(const CString& resourceName);
		static	OV<Type>	getTypeFromMIMEType(const CString& MIMEType);
		static	OV<Type>	getTypeFromData(const CData& data);
		static	CString		getDefaultFilenameExtension(Type type);
		static	CString		getMIMEType(Type type);

	// Properties
	private:
		CImageInternals*	mInternals;
};
