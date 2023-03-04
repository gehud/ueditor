#pragma once

#include <uengine.h>

using namespace uengine;

namespace ueditor {
	class library_loader {
	public:
		library_loader();

		~library_loader();

		void open(const String& path);

		void close();

		template<typename Invocable>
		Invocable* get(const String& name) const {
			return reinterpret_cast<Invocable*>(get_proc_address(name));
		}
	private:
		void* _handle;

		void* get_proc_address(const String& name) const;
	};
}