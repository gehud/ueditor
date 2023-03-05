#pragma once
#ifndef UEDITOR_CORE_SCENE_SERIALIZER
#define UEDITOR_CORE_SCENE_SERIALIZER

#include <uengine/core/scene.h>
#include <uengine/core/io/path.h>

using namespace uengine;

namespace ueditor {
	class SceneSerializer {
	public:
		SceneSerializer(const SharedPtr<Scene>& scene) : _scene(scene) {}

		void serialize(const Path& path);

		void deserialize(const Path& path);
	private:
		SharedPtr<Scene> _scene;
	};
}

#endif