#pragma once
#ifndef UEDITOR_CORE_ASSETS
#define UEDITOR_CORE_ASSETS

#include <uengine/core/definitions.h>
#include <uengine/core/memory.h>
#include <uengine/core/traits.h>
#include <uengine/core/object.h>
#include <uengine/core/traits.h>
#include <uengine/core/io/path.h>
#include <uengine/core/rendering/mesh.h>
#include <uengine/core/rendering/shader.h>
#include <uengine/core/rendering/material.h>
#include <uengine/core/collections/dictionary.h>
#include <uengine/core/collections/list.h>

using namespace uengine;

namespace ueditor {
	/**
	 * @brief Asset database.
	 */
	class Assets {
	public:
		enum class ImportMode {
			Default,
			Force
		};

		static const Path& path();

		static Path path(const ULong& uuid);

		static ULong uuid(const SharedPtr<Object>& object);

		static ULong uuid(const Path& path);

		static List<ULong> uuids(const Path& path);

		static String name(const ULong& uuid);

		static String type(const ULong& uuid);

		static bool is_complex(const Path& path);	

		static void import(const Path& path, ImportMode import_mode = ImportMode::Default);

		template<DerivedFrom<Object> T>
		static SharedPtr<T> load(const ULong& uuid) {
			Log::error("Unsupported asset format");
			return nullptr;
		}

		template<>
		static SharedPtr<Shader> load(const ULong& uuid) {
			return load_shader(uuid);
		}

		template<>
		static SharedPtr<Material> load(const ULong& uuid) {
			return load_material(uuid);
		}

		template<>
		static SharedPtr<Mesh> load(const ULong& uuid) {
			return load_mesh(uuid);
		}

		//TODO: Load subassets.

		template<DerivedFrom<Object> T>
		static SharedPtr<T> load(const Path& path) {
			ULong uuid = Assets::uuid(path);
			if (uuid == 0) {
				Log::error("Failed to load asset.");
				return nullptr;
			}

			return load<T>(uuid);
		}
	private:
		static Dictionary<WeakPtr<Object>, ULong> _loaded;
		static Path _assets;
		static Path _cache;

		static void import_all(const Path& path, ImportMode import_mode = ImportMode::Default);

		static void initialize(const Path& assets, const Path& cache);

		static void import_shader(const Path& path, ImportMode import_mode = ImportMode::Default);

		static void import_material(const Path& path, ImportMode import_mode = ImportMode::Default);

		static void import_model(const Path& path, ImportMode import_mode = ImportMode::Default);

		static SharedPtr<Shader> load_shader(const ULong& uuid);

		static SharedPtr<Material> load_material(const ULong& uuid);

		static SharedPtr<Mesh> load_mesh(const ULong& uuid);

		friend class EditorApplication;
	};
}

#endif	