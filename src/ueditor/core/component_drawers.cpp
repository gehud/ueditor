#include "ueditor/core/component_drawers.h"

#include <uengine/core/ui/imgui.h>

namespace ueditor {
	TransformComponentDrawer::TransformComponentDrawer() : ComponentDrawer("Transform") {}

	void TransformComponentDrawer::on_imgui() {
		auto& transform = target();

		ImGui::DragFloat3("Position", &transform.position[0], 0.1f);
		Float3 euler_angles = transform.rotation.euler_angles();
		ImGui::DragFloat3("Rotation", &euler_angles[0], 0.1f);
		transform.rotation = Float4::quaternion(euler_angles);
		ImGui::DragFloat3("Scale", &transform.scale[0], 0.1f);
	}

	CameraComponentDrawer::CameraComponentDrawer() : ComponentDrawer("Camera") {}

	void CameraComponentDrawer::on_imgui() {
		auto& camera = target();

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
			float deg = Math::degrees(camera.fov);
			ImGui::DragFloat("FOV", &deg, 0.1f);
			camera.fov = Math::radians(deg);
		} else {
			ImGui::DragFloat("Size", &camera.size, 0.1f);
		}

		ImGui::DragFloat("Aspect", &camera.aspect, 0.1f);
		ImGui::ColorEdit4("Clear Color", &camera.clear_color[0]);
	}
}