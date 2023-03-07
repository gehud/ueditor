#include "ueditor/core/outline_window.h"

#include "ueditor/core/camera_controller.h"

#include <uengine/core/scene.h>
#include <uengine/core/ui/imgui.h>

namespace ueditor {
	OutlineWindow::Selection OutlineWindow::_selection;

	OutlineWindow::OutlineWindow() : EditorWindow("Outline") {}

	void OutlineWindow::on_imgui() {
		auto& scene = Scene::active();
		if (!scene) {
			return;
		}

		if (ImGui::TreeNodeEx(scene->name().data(), ImGuiTreeNodeFlags_Selected)) {
			auto& world = scene->world();
			world.each([&](Entity entity) {
				if (world.has_component<CameraController>(entity) 
					&& world.get_component<CameraController>(entity).is_outline_hidden) {
					return;
				}
				ImGuiTreeNodeFlags flags = 0;
				if (_selection)
					if (_selection.world == &world && _selection.entity == entity)
						flags |= ImGuiTreeNodeFlags_Selected;

				if (ImGui::TreeNodeEx("Entity", ImGuiTreeNodeFlags_Leaf | flags)) {
					ImGui::TreePop();
				}

				if (ImGui::IsItemClicked()) {
					_selection = {&world, entity};
				}
			});

			ImGui::TreePop();
		}
	}
}