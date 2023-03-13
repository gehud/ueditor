#include "ueditor/core/outline_window.h"

#include "ueditor/core/camera_controller.h"
#include "ueditor/core/properties_window.h"
#include "ueditor/core/selection.h"
#include "ueditor/core/ui/editor_imgui.h"

#include <uengine/core/scene.h>
#include <uengine/core/ui/imgui.h>

namespace ueditor {
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
				if (dynamic_cast<Entity*>(Selection::object()) && *static_cast<Entity*>(Selection::object()) == _selected)
					flags |= ImGuiTreeNodeFlags_Selected;

				if (ImGui::TreeNodeEx("Entity", ImGuiTreeNodeFlags_Leaf | flags)) {
					ImGui::TreePop();
				}

				if (ImGui::IsItemClicked()) {
					Selection::context(&world);
					_selected = entity;
					Selection::object(&_selected);
				}
			});

			ImGui::TreePop();
		}

		if (ImGui::BeginPopupContextItem()) {
			ImGui::TextDisabled("Actions");
			if (ImGui::MenuItem("Delete")) {
				if (dynamic_cast<Entity*>(Selection::object()))
					scene->world().destroy_entity(*static_cast<Entity*>(Selection::object()));
			}

			ImGui::TextDisabled("Create");
			if (ImGui::MenuItem("Entity")) {
				scene->world().create_entity();
			}

			ImGui::EndPopup();
		}
	}
}