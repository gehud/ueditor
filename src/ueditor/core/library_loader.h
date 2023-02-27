#pragma once

#include <uengine.h>

using namespace uengine;

namespace ueditor {
	class library_loader {
	public:
		library_loader();

		~library_loader();

		void open(const string& path);

		void close();

		template<typename Invocable>
		Invocable* get(const string& name) const {
			return reinterpret_cast<Invocable*>(get_proc_address(name));
		}
	private:
		void* _handle;

		void* get_proc_address(const string& name) const;
	};
}