#include "ueditor/core/outline_window.h"

#include <uengine/core/scene.h>
#include <uengine/core/ui/imgui.h>

namespace ueditor {
	OutlineWindow::Selection OutlineWindow::_selection;

	OutlineWindow::OutlineWindow() : EditorWindow("Outline") {}

	void OutlineWindow::on_imgui() {
		for (auto& scene : Scene::loaded()) {
			if (ImGui::TreeNodeEx(scene->name().data(), ImGuiTreeNodeFlags_Selected)) {
				auto& world = scene->world();
				world.each([&](Entity entity) {
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
}