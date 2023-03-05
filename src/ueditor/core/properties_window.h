#pragma once
#ifndef UEDITOR_CORE_PROPERTIES_WINDOW
#define UEDITOR_CORE_PROPERTIES_WINDOW

#include "ueditor/core/component_drawer.h"
#include "ueditor/core/editor_window.h"

#include <uengine/core/memory.h>
#include <uengine/core/traits.h>
#include <uengine/core/world.h>
#include <uengine/core/collections/dictionary.h>

namespace ueditor {
	class PropertiesWindow : public EditorWindow {
	public:
		PropertiesWindow();
	protected:
		void on_imgui() override;
	private:
		static Dictionary<TypeIndex, SharedPtr<IComponentDrawer>> _drawers;

		static Dictionary<TypeIndex, SharedPtr<IComponentDrawer>>& drawers();

		template<DerivedFrom<IComponentDrawer> T>
		static void add() {
			auto ptr = uengine::make_shared<T>();
			drawers().add(ptr->type_index(), ptr);
		}
	};
}

#endif