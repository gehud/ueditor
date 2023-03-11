#include "ueditor/core/component_drawers.h"

#include "ueditor/core/assets.h"

#include <uengine/core/io/file.h>
#include <uengine/core/io/directory.h>
#include <uengine/core/ui/imgui.h>

#include <yaml-cpp/yaml.h>

namespace ueditor {
	TransformComponentDrawer::TransformComponentDrawer() : ComponentDrawer("Transform") {}

	void TransformComponentDrawer::on_imgui() {
		auto& transform = target();

		ImGui::DragFloat3("Position", &transform.position[0], 0.1f);
		if (ImGui::DragFloat3("Rotation", &transform.euler_angles_hint[0], 0.1f)) {
			transform.rotation = Float4::quaternion(transform.euler_angles_hint);
		}
		ImGui::DragFloat3("Scale", &transform.scale[0], 0.1f);
	}

	CameraComponentDrawer::CameraComponentDrawer() : ComponentDrawer("Camera") {}

	void CameraComponentDrawer::on_imgui() {
		auto& camera = target();

		ImGui::Checkbox("Is Base", &camera.is_base);

		const char* projection_enum_strings[] = {
			"Perspective",
			"Othographics"
		};

		const char* projection_enum_current = projection_enum_strings[(int)camera.projection];
		if (ImGui::BeginCombo("Projection", projection_enum_current)) {
			for (int i = 0; i < 2; i++) {
				bool selected = projection_enum_current == projection_enum_strings[i];
				if (ImGui::Selectable(projection_enum_strings[i], selected)) {
					projection_enum_current = projection_enum_strings[i];
					camera.projection = (Camera::Projection)i;

				}

				if (selected)
					ImGui::SetItemDefaultFocus();

			}

			ImGui::EndCombo();
		}

		if (camera.projection == Camera::Projection::Perspective) {
			ImGui::DragFloat("FOV", &camera.fov, 0.1f);
		} else {
			ImGui::DragFloat("Size", &camera.size, 0.1f);
		}

		ImGui::ColorEdit4("Clear Color", &camera.clear_color[0]);
	}

	RenderMeshComponentDrawer::RenderMeshComponentDrawer() : ComponentDrawer("Render Mesh") {}

	void RenderMeshComponentDrawer::on_imgui() {
		auto& render_mesh = target();

		ImVec2 label_size = ImGui::CalcTextSize("Mesh");
		ImGui::Text("Mesh");
		ImGui::SameLine();
		ImGuiID id = ImGui::GetID("ObjectField");
		ImVec2 cursor_position = ImGui::GetCursorScreenPos();
		ImRect rect = {cursor_position, 
		{
			cursor_position.x + ImGui::GetContentRegionAvail().x, 
			cursor_position.y + label_size.y
		}};
		ImGui::GetWindowDrawList()->AddRectFilled(
			rect.Min,
			rect.Max,
			ImGui::GetColorU32({0.09f, 0.09f, 0.09f, 1.0f}));

		if (ImGui::BeginDragDropTargetCustom(rect, id)) {
			auto payload = ImGui::AcceptDragDropPayload("EXPLORER_ITEM");
			if (payload) {
				Path path((const char*)payload->Data);
				auto filename = path.filename().string();
				if (filename.find("sub_asset_") != -1) {
					int last_of_us = filename.find_last_of('_');
            		ULong uuid = filename.substring(last_of_us + 1).as<ULong>();
					if (Assets::type(uuid) == "Mesh") {
						render_mesh.mesh = Assets::load<Mesh>(uuid);
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
	}
}