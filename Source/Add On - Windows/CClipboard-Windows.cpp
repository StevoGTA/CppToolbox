//----------------------------------------------------------------------------------------------------------------------
//	CClipboard-Windows.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CClipboard.h"

#include "SError-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	HWND								sOwnerWindow = NULL;
static	CClipboard::ContentsReplacedProc	sContentsReplacedProc = nil;
static	void*								sContentsReplacedProcUserData = nil;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local procs

//----------------------------------------------------------------------------------------------------------------------
static LRESULT CALLBACK sOwnerWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check message
	if ((message == WM_DESTROYCLIPBOARD) && (sContentsReplacedProc != nil)) {
		// Contents replaced
		CClipboard::ContentsReplacedProc	contentsReplacedProc = sContentsReplacedProc;
		sContentsReplacedProc = nil;
		contentsReplacedProc(sContentsReplacedProcUserData);

		return 0;
	} else
		// Other message
		return ::DefWindowProcW(window, message, wParam, lParam);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CClipboard

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CClipboard::set(const CString& type, const CData& data, ContentsReplacedProc contentsReplacedProc,
		void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have owner window
	if (sOwnerWindow == NULL) {
		// Create owner window
		WNDCLASSEXW	windowClass = {0};
		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.lpfnWndProc = sOwnerWindowProc;
		windowClass.hInstance = ::GetModuleHandleW(NULL);
		windowClass.lpszClassName = L"CClipboardOwner";
		::RegisterClassExW(&windowClass);

		sOwnerWindow =
				::CreateWindowExW(0, L"CClipboardOwner", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, windowClass.hInstance,
						NULL);
		if (sOwnerWindow == NULL)
			// Error
			return OV<SError>(SErrorFromWindowsGetLastError());
	}

	// Setup
	UINT	format = ::RegisterClipboardFormatW(type.getOSString());
	if (format == 0)
		// Error
		return OV<SError>(SErrorFromWindowsGetLastError());

	HGLOBAL	global = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T) data.getByteCount());
	if (global == NULL)
		// Error
		return OV<SError>(SErrorFromWindowsGetLastError());
	data.copyBytes(::GlobalLock(global));
	::GlobalUnlock(global);

	// Open
	if (!::OpenClipboard(sOwnerWindow)) {
		// Error
		SError	error = SErrorFromWindowsGetLastError();
		::GlobalFree(global);

		return OV<SError>(error);
	}

	// Set.  Emptying the clipboard notifies the previous owner, which may be us for an earlier set().
	::EmptyClipboard();

	// Store new proc
	sContentsReplacedProc = contentsReplacedProc;
	sContentsReplacedProcUserData = userData;

	// Set the data
	OV<SError>	error;
	if (::SetClipboardData(format, global) == NULL) {
		// Error
		error.setValue(SErrorFromWindowsGetLastError());
		::GlobalFree(global);
	}

	// Close
	::CloseClipboard();

	return error;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CClipboard::getData(const CString& type)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UINT	format = ::RegisterClipboardFormatW(type.getOSString());
	if ((format == 0) || !::OpenClipboard(NULL))
		// Unavailable
		return OV<CData>();

	// Get data
	OV<CData>	data;
	HANDLE		handle = ::GetClipboardData(format);
	if (handle != NULL) {
		// Copy
		const	void*	bytePtr = ::GlobalLock(handle);
		if (bytePtr != NULL) {
			// Have bytes
			data.setValue(CData(bytePtr, (CData::ByteCount) ::GlobalSize(handle)));
			::GlobalUnlock(handle);
		}
	}

	// Close
	::CloseClipboard();

	return data;
}
