#include "library_loader.h"
#include "library_loader.h"

#include <Windows.h>

namespace ueditor {
	library_loader::library_loader() : _handle(nullptr) {}

	library_loader::~library_loader() {
		close();
	}

	void library_loader::open(const string& path) {
		_handle = LoadLibrary(path.get_data());
		UENGINE_ASSERT(_handle, "Failed to load library.");
	}

	void library_loader::close() {
		if (_handle) {
			BOOL result = FreeLibrary(static_cast<HMODULE>(_handle));
			UENGINE_ASSERT(result, "Failed to close library.");
		}
	}

	void* library_loader::get_proc_address(const string& name) const {
		return GetProcAddress(static_cast<HMODULE>(_handle), name.get_data());
	}
}