#pragma once
#ifndef UEDITOR_CORE_ASSETS
#define UEDITOR_CORE_ASSETS

#include <uengine/core/definitions.h>
#include <uengine/core/memory.h>
#include <uengine/core/io/path.h>
#include <uengine/core/io/file.h>
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

		static void import(const Path& path);
	private:
		static Path _path;

		friend class EditorApplication;
	};
}

#endif	