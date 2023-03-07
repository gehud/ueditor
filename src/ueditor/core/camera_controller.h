#pragma once
#ifndef UEDITOR_CORE_CAMERA_CONTROLLER
#define UEDITOR_CORE_CAMERA_CONTROLLER

#include <uengine/core/system.h>
#include <uengine/core/input.h>
#include <uengine/core/keys.h>
#include <uengine/core/time.h>
#include <uengine/core/transform.h>
#include <uengine/core/rendering/camera.h>

using namespace uengine;

namespace ueditor {
	struct CameraController {
		float speed = 5;
		float sensitivity = 10.0f;
		float pitch = 0.0f;
		Float4 yaw_rotation = Float4::quaternion();
		bool is_enabled = true;
		bool is_outline_hidden = true;
	};

	class CameraControllerSystem : public System {
	protected:
		void on_update() override {
			world().view<Transform, Camera, CameraController>().each([](Transform& transform, Camera& camera, CameraController& camera_controller) {
				if (!camera_controller.is_enabled) {
					return;
				}

				auto model = Float4x4::translate(transform.position)
				* Float4x4::rotate(transform.rotation)
				* Float4x4::scale(transform.scale);
				if (Input::is_key(UENGINE_KEY_W)) {
					transform.position += model.forward() * camera_controller.speed * Time::delta();
				} else if (Input::is_key(UENGINE_KEY_S)) {
					transform.position -= model.forward() * camera_controller.speed * Time::delta();
				}

				if (Input::is_key(UENGINE_KEY_D)) {
					transform.position += model.right() * camera_controller.speed * Time::delta();
				} else if (Input::is_key(UENGINE_KEY_A)) {
					transform.position -= model.right() * camera_controller.speed * Time::delta();
				}

				if (Input::is_key(UENGINE_KEY_E)) {
					transform.position += model.up() * camera_controller.speed * Time::delta();
				} else if (Input::is_key(UENGINE_KEY_Q)) {
					transform.position -= model.up() * camera_controller.speed * Time::delta();
				}

				if (Input::is_mouse_button(UENGINE_MOUSE_BUTTON_RIGHT)) {
					auto mouse_position_delta = Input::mouse_position_delta();
					camera_controller.pitch = Math::clamp(camera_controller.pitch + mouse_position_delta.y * camera_controller.sensitivity * Time::delta(), -90.0f, 90.0f);
					camera_controller.yaw_rotation *= Float4::quaternion(Float3(0.0f, mouse_position_delta.x * camera_controller.sensitivity * Time::delta(), 0.0f));
					transform.rotation = camera_controller.yaw_rotation * Float4::quaternion(Float3(camera_controller.pitch, 0.0f, 0.0f));
				}
			});
		}
	};
}

#endif