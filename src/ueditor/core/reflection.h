#pragma once

#include <uengine.h>

using namespace uengine;

namespace ueditor {
	class Type {
	public:
		const Path& path() const {
			return _path;
		}

		const String& name() const {
			return _name;
		}

		Type* base() {
			return _base;
		}
	private:
		Path _path;
		String _name;
		Type* _base;

		Type() : _base(nullptr) {}

		Type(const Path& path, const String& name, Type* base) 
			: _path(path), _name(name), _base(base) {}

		friend class Reflection;
	};

	class Assembly {
	public:
		const List<Type>& types() const {
			return _types;
		}
	private:
		List<Type> _types;

		Assembly() = default;

		friend class Reflection;
	};

	class Reflection {
	public:
		static Assembly reflect(const Path& directory);
	private:
		static void reflect_recurse(Assembly& assembly, const Path& path);

		static void reflect_header(Assembly& assembly, const Path& path);
	};
}