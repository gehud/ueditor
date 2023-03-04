#pragma once

#include <uengine.h>

using namespace uengine;

namespace ueditor {
	class Library {
	public:
		Library(const String& path);

		~Library();

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