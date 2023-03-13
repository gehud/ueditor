#include "ueditor/core/properties_window.h"

#include "ueditor/core/assets.h"
#include "ueditor/core/selection.h"
#include "ueditor/core/outline_window.h"
#include "ueditor/core/explorer_window.h"
#include "ueditor/core/component_drawers.h"
#include "ueditor/core/ui/editor_imgui.h"

#include <uengine/core/io/path.h>

namespace ueditor {
	Dictionary<TypeIndex, SharedPtr<IComponentDrawer>> PropertiesWindow::_drawers;

	PropertiesWindow::PropertiesWindow() : EditorWindow("Properties") {
		add<TransformComponentDrawer>();
		add<CameraComponentDrawer>();
		add<RenderMeshComponentDrawer>();
	}

	void PropertiesWindow::on_imgui() {
		if (dynamic_cast<World*>(Selection::context()) != nullptr) {
			auto& world = *static_cast<World*>(Selection::context());
			const auto& entity = *static_cast<Entity*>(Selection::object());

			for (auto& pair : _drawers) {
				if (world.has_component(entity, pair.key())) {
					if (ImGui::TreeNodeEx((void*)pair.value()->type_index().hash_code(), ImGuiTreeNodeFlags_Selected, pair.value()->name().data())) {
						pair.value()->target(world.get_component(entity, pair.key()));
						pair.value()->on_imgui();
						ImGui::TreePop();
					}
				}
			}

			if (ImGui::Button("Component")) {
				ImGui::OpenPopup("ComponentPopup");
			}

			if (ImGui::BeginPopup("ComponentPopup")) {
				if (ImGui::MenuItem("Transform"))
					if (entity != Entity::null())
						world.add_component<Transform>(entity);
				if (ImGui::MenuItem("Camera"))
					if (entity != Entity::null())
						world.add_component<Camera>(entity);
				if (ImGui::MenuItem("RenderMesh"))
					if (entity != Entity::null())
						world.add_component<RenderMesh>(entity);
				ImGui::EndPopup();
			}
		} else if (dynamic_cast<Material*>(Selection::object()) != nullptr) {
			ImGui::Text("...");
		}
	}

	Dictionary<TypeIndex, SharedPtr<IComponentDrawer>>& PropertiesWindow::drawers() {
		return _drawers;
	}
}