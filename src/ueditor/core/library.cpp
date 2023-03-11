#include "library.h"

#include <Windows.h>

namespace ueditor {
	Library::Library(const Path& path) {
		_handle = LoadLibrary(path.string().data());
		UENGINE_ASSERT(_handle, "Failed to load library.");
	}

	Library::~Library() {
		close();
	}

	void Library::close() {
		if (_handle) {
			BOOL result = FreeLibrary(static_cast<HMODULE>(_handle));
			UENGINE_ASSERT(result, "Failed to close library.");
		}
	}

	void* Library::get_proc_address(const String& name) const {
		return GetProcAddress(static_cast<HMODULE>(_handle), name.data());
	}
}