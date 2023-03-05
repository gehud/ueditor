#pragma once
#ifndef UEDITOR_CORE_OUTLINE_WINDOW
#define UEDITOR_CORE_OUTLINE_WINDOW

#include "ueditor/core/editor_window.h"

#include <uengine/core/world.h>

namespace ueditor {
	class OutlineWindow : public EditorWindow {
	public:
		OutlineWindow();
	protected:
		void on_imgui() override;
	private:
		struct Selection {
			World* world = nullptr;
			Entity entity;

			explicit operator bool() const noexcept { 
				return entity != Entity::null(); 
			}
		};

		static Selection _selection;

		friend class PropertiesWindow;
	};
}

#endif