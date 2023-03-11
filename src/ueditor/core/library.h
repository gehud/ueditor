#pragma once

#include <uengine.h>

using namespace uengine;

namespace ueditor {
	class Library {
	public:
		Library(const Path& path);

		~Library();

		void close();

		template<typename TInvocable>
		TInvocable* get(const String& name) const {
			return reinterpret_cast<TInvocable*>(get_proc_address(name));
		}
	private:
		void* _handle;

		void* get_proc_address(const String& name) const;
	};
}