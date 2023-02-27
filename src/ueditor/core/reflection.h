#pragma once

#include <uengine.h>

using namespace uengine;

namespace ueditor {
	class type {
	public:
		const path& get_path() const {
			return _path;
		}

		const string& get_name() const {
			return _name;
		}

		type* get_base() {
			return _base;
		}
	private:
		path _path;
		string _name;
		type* _base;

		type() : _base(nullptr) {}

		type(const path& path, const string& name, type* base) 
			: _path(path), _name(name), _base(base) {}

		friend class reflection;
	};

	class assembly {
	public:
		const list<type> get_types() {
			return _types;
		}
	private:
		list<type> _types;

		assembly() = default;

		friend class reflection;
	};

	class reflection {
	public:
		static assembly reflect(const path& directory);
	private:
		static void reflect_recurse(assembly& assembly, const path& path);

		static void reflect_header(assembly& assembly, const path& path);
	};
}