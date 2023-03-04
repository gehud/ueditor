#pragma once

#include <uengine.h>

using namespace uengine;

namespace ueditor {
	class type {
	public:
		const Path& get_path() const {
			return _path;
		}

		const String& get_name() const {
			return _name;
		}

		type* get_base() {
			return _base;
		}
	private:
		Path _path;
		String _name;
		type* _base;

		type() : _base(nullptr) {}

		type(const Path& path, const String& name, type* base) 
			: _path(path), _name(name), _base(base) {}

		friend class reflection;
	};

	class assembly {
	public:
		const List<type> get_types() {
			return _types;
		}
	private:
		List<type> _types;

		assembly() = default;

		friend class reflection;
	};

	class reflection {
	public:
		static assembly reflect(const Path& directory);
	private:
		static void reflect_recurse(assembly& assembly, const Path& path);

		static void reflect_header(assembly& assembly, const Path& path);
	};
}