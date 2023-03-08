#include "ueditor/core/properties_window.h"

#include "ueditor/core/outline_window.h"
#include "ueditor/core/component_drawers.h"

namespace ueditor {
	Dictionary<TypeIndex, SharedPtr<IComponentDrawer>> PropertiesWindow::_drawers;

	PropertiesWindow::PropertiesWindow() : EditorWindow("Properties") {
		add<TransformComponentDrawer>();
		add<CameraComponentDrawer>();
		add<RenderMeshComponentDrawer>();
	}

	void PropertiesWindow::on_imgui() {
		if (!OutlineWindow::_selection)
			return;

		auto& world = *OutlineWindow::_selection.world;
		const auto& entity = OutlineWindow::_selection.entity;

		for (auto& pair : _drawers) {
			if (world.has_component(entity, pair.key)) {
				if (ImGui::TreeNodeEx((void*)pair.value->type_index().hash_code(), ImGuiTreeNodeFlags_Selected, pair.value->name().data())) {
					pair.value->target(world.get_component(entity, pair.key));
					pair.value->on_imgui();
					ImGui::TreePop();
				}
			}
		}
	}

	Dictionary<TypeIndex, SharedPtr<IComponentDrawer>>& PropertiesWindow::drawers() {
		return _drawers;
	}
}