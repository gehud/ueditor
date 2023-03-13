#pragma once
#ifndef UEDITOR_CORE_OUTLINE_WINDOW
#define UEDITOR_CORE_OUTLINE_WINDOW

#include "ueditor/core/editor_window.h"

#include <uengine/core/world.h>

using namespace uengine;

namespace ueditor {
	class OutlineWindow : public EditorWindow {
	public:
		OutlineWindow();
	protected:
		void on_imgui() override;
	private:
		Entity _selected;

		friend class PropertiesWindow;
	};
}

#endif