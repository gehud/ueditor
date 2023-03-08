#pragma once
#ifndef UEDITOR_CORE_ASSETS
#define UEDITOR_CORE_ASSETS

#include <uengine/core/definitions.h>
#include <uengine/core/memory.h>
#include <uengine/core/traits.h>
#include <uengine/core/object.h>
#include <uengine/core/io/path.h>
#include <uengine/core/rendering/mesh.h>
#include <uengine/core/collections/dictionary.h>

using namespace uengine;

namespace ueditor {
	/**
	 * @brief Asset database.
	 */
	class Assets {
	public:
		static const Path& path();

		static void path(const Path& path);

		static Path path(const ULong& uuid);

		static ULong uuid(const Path& path);

		static ULong uuid(const SharedPtr<Object>& object);

		static void import(const Path& path);
		
		template<typename T>
		static SharedPtr<T> load(const Path& path) {
			if (path.extension() != ".mesh" || !IsSame<T, Mesh>::value) {
				Log::error("Unsupported asset format.");
				return nullptr;
			}

			return load_mesh(path);
		}

		template<typename T>
		static SharedPtr<T> load(const ULong& uuid) {
			auto path = Assets::path(uuid);
			if (!path.is_empty()) {
				return load<T>(path);
			}
		}
	private:
		static Dictionary<ULong, Path> _paths;
		static Dictionary<SharedPtr<Object>, ULong> _uids;
		static Path _path;

		static SharedPtr<Mesh> load_mesh(const Path& path);

		static void initialize(const Path& path);

		friend class EditorApplication;
	};
}

#endif	