#pragma once
#ifndef UEDITOR_CORE_COMPONENT_DRAWERS
#define UEDITOR_CORE_COMPONENT_DRAWERS

#include "ueditor/core/component_drawer.h"

#include <uengine/core/transform.h>
#include <uengine/core/rendering/camera.h>

using namespace uengine;

namespace ueditor {
	class TransformComponentDrawer : public ComponentDrawer<Transform> {
	public:
		TransformComponentDrawer();
	protected:
		void on_imgui() override;
	};

	class CameraComponentDrawer : public ComponentDrawer<Camera> {
	public:
		CameraComponentDrawer();
	protected:
		void on_imgui() override;
	};
}

#endif