#include "ueditor/core/scene_serializer.h"

#include "ueditor/core/assets.h"
#include "ueditor/core/camera_controller.h"

#include <uengine/core/transform.h>
#include <uengine/core/uuid.h>
#include <uengine/core/rendering/camera.h>
#include <uengine/core/io/file.h>

#include <yaml-cpp/yaml.h>

namespace YAML {
	template<typename T>
	struct convert<Vector3<T>> {
		static Node encode(const Vector3<T>& value) {
			Node node;
			node.push_back(value.x);
			node.push_back(value.y);
			node.push_back(value.z);
			return node;
		}

		static bool decode(const Node& node, Vector3<T>& value) {
			if (!node.IsSequence() || node.size() != 3)
				return false;

			value.x = node[0].as<T>();
			value.y = node[1].as<T>();
			value.z = node[2].as<T>();

			return true;
		}
	};

	template<typename T>
	struct convert<Vector4<T>> {
		static Node encode(const Vector4<T>& value) {
			Node node;
			node.push_back(value.x);
			node.push_back(value.y);
			node.push_back(value.z);
			node.push_back(value.w);
			return node;
		}

		static bool decode(const Node& node, Vector4<T>& value) {
			if (!node.IsSequence() || node.size() != 4)
				return false;

			value.x = node[0].as<T>();
			value.y = node[1].as<T>();
			value.z = node[2].as<T>();
			value.w = node[3].as<T>();

			return true;
		}
	};
}

namespace ueditor {
	YAML::Emitter& operator<<(YAML::Emitter& out, const Float3& vector) {
		out << YAML::Flow;
		out << YAML::BeginSeq;
		out << vector.x;
		out << vector.y;
		out << vector.z;
		out << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const Float4& vector) {
		out << YAML::Flow;
		out << YAML::BeginSeq;
		out << vector.x;
		out << vector.y;
		out << vector.z;
		out << vector.z;
		out << YAML::EndSeq;
		return out;
	}

	static void serialize_entity(YAML::Emitter& out, World& world, Entity entity) {
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << UUID();

		if (world.has_component<Transform>(entity)) {
			auto& transform = world.get_component<Transform>(entity);
			out << YAML::Key << "Transform" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Position" << YAML::Value << transform.position;
			out << YAML::Key << "Rotation" << YAML::Value << transform.rotation;
			out << YAML::Key << "Scale" << YAML::Value << transform.scale;
			out << YAML::Key << "EulerAnglesHint" << YAML::Value << transform.euler_angles_hint;
			out << YAML::EndMap;
		}

		if (world.has_component<Camera>(entity)) {
			auto& camera = world.get_component<Camera>(entity);
			out << YAML::Key << "Camera" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Projection" << YAML::Value << (int)camera.projection;
			out << YAML::Key << "FOV" << YAML::Value << camera.fov;
			out << YAML::Key << "Size" << YAML::Value << camera.size;
			out << YAML::Key << "NearPlane" << YAML::Value << camera.near_plane;
			out << YAML::Key << "FarPlane" << YAML::Value << camera.far_plane;
			out << YAML::Key << "ClearColor" << YAML::Value << camera.clear_color;
			out << YAML::Key << "IsBase" << YAML::Value << camera.is_base;
			out << YAML::EndMap;
		}

		if (world.has_component<RenderMesh>(entity)) {
			auto& render_mesh = world.get_component<RenderMesh>(entity);
			out << YAML::Key << "RenderMesh" << YAML::Value << YAML::BeginMap;
			if (render_mesh.mesh) {
				out << YAML::Key << "Mesh" << YAML::Value << Assets::uuid(render_mesh.mesh);
			}
			out << YAML::EndMap;
		}

		if (world.has_component<CameraController>(entity)) {
			auto& camera_controller = world.get_component<CameraController>(entity);
			out << YAML::Key << "EditorCameraController" << YAML::Value << YAML::BeginMap;
			out << YAML::Key << "Pitch" << YAML::Value << camera_controller.pitch;
			out << YAML::Key << "Sensitivity" << YAML::Value << camera_controller.sensitivity;
			out << YAML::Key << "Speed" << YAML::Value << camera_controller.speed;
			out << YAML::Key << "Yaw" << YAML::Value << camera_controller.yaw;
			out << YAML::EndMap;
		}

		out << YAML::EndMap;
	}

	void SceneSerializer::serialize(const Path& path) {
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene";
		out << YAML::Value << _scene->name().data();
		out << YAML::Key << "Entities";
		out << YAML::Value << YAML::BeginSeq;
		auto& world = _scene->world();
		world.each([&](Entity entity) {
			serialize_entity(out, world, entity);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;
		File file(path, FileMode::Out);
		file << out.c_str();
	}

	void SceneSerializer::deserialize(const Path& path) {
		File file(path, FileMode::In);
	
		YAML::Node data = YAML::Load(file.string().data());
		if (!data["Scene"])
			return;

		String scene_name = data["Scene"].as<std::string>();
		_scene->name(scene_name);

		auto entities = data["Entities"];
		if (!entities)
			return;

		auto& world = _scene->world();


		int editor_camera_entity_index = -1;
		int counter = 0;
		// Find editor camera and add it first.
		for (auto entity_data : entities) {
			if (entity_data["EditorCameraController"]) {
				editor_camera_entity_index = counter;

				auto entity = world.create_entity();
				auto camera_controller_data = entity_data["EditorCameraController"];
				auto& camera_controller = world.add_component<CameraController>(entity);
				camera_controller.pitch = camera_controller_data["Pitch"].as<float>();
				camera_controller.sensitivity = camera_controller_data["Sensitivity"].as<float>();
				camera_controller.speed = camera_controller_data["Speed"].as<float>();
				camera_controller.yaw = camera_controller_data["Yaw"].as<float>();

				if (entity_data["Transform"]) {
				auto transform_data = entity_data["Transform"];
				auto& transform = world.add_component<Transform>(entity);
				transform.position = transform_data["Position"].as<Float3>();
				transform.rotation = transform_data["Rotation"].as<Float4>();
				transform.scale = transform_data["Scale"].as<Float3>();
				transform.euler_angles_hint = transform_data["EulerAnglesHint"].as<Float3>();
				}

				if (entity_data["Camera"]) {
					auto camera_data = entity_data["Camera"];
					auto& camera = world.add_component<Camera>(entity);
					camera.projection = (Camera::Projection)camera_data["Projection"].as<int>();
					camera.fov = camera_data["FOV"].as<float>();
					camera.size = camera_data["Size"].as<float>();
					camera.near_plane = camera_data["NearPlane"].as<float>();
					camera.far_plane = camera_data["FarPlane"].as<float>();
					camera.clear_color = camera_data["ClearColor"].as<Float4>();
					camera.is_base = camera_data["IsBase"].as<bool>();
				}
			}

			++counter;
		}

		counter = 0;
		for (auto entity_data : entities) {
			if (counter == editor_camera_entity_index)
				continue;
				
			auto entity = world.create_entity();

			if (entity_data["Transform"]) {
				auto transform_data = entity_data["Transform"];
				auto& transform = world.add_component<Transform>(entity);
				transform.position = transform_data["Position"].as<Float3>();
				transform.rotation = transform_data["Rotation"].as<Float4>();
				transform.scale = transform_data["Scale"].as<Float3>();
				transform.euler_angles_hint = transform_data["EulerAnglesHint"].as<Float3>();
			}

			if (entity_data["Camera"]) {
				auto camera_data = entity_data["Camera"];
				auto& camera = world.add_component<Camera>(entity);
				camera.projection = (Camera::Projection)camera_data["Projection"].as<int>();
				camera.fov = camera_data["FOV"].as<float>();
				camera.size = camera_data["Size"].as<float>();
				camera.near_plane = camera_data["NearPlane"].as<float>();
				camera.far_plane = camera_data["FarPlane"].as<float>();
				camera.clear_color = camera_data["ClearColor"].as<Float4>();
				camera.is_base = camera_data["IsBase"].as<bool>();
			}

			if (entity_data["RenderMesh"]) {
				auto render_mesh_data = entity_data["RenderMesh"];
				auto& render_mesh = world.add_component<RenderMesh>(entity);
				if (render_mesh_data["Mesh"]) {
					render_mesh.mesh = Assets::load<Mesh>(render_mesh_data["Mesh"].as<ULong>());

					// Temp.
					auto shader = make_shared<Shader>("../assets/shaders/texture.glsl");
					auto material = make_shared<Material>(shader);
					auto texture = make_shared<Texture2D>("../assets/textures/checkerboard.png");
					texture->filter_mode(Texture::FilterMode::Nearest);
					material->set("u_Texture", texture);
					render_mesh.materials = {material};
				}
			}

			++counter;
		}
	}
}