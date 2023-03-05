#include "ueditor/core/scene_serializer.h"

#include <uengine/core/transform.h>
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
		out << YAML::Key << "Entity" << YAML::Value << entity.index();

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
		FileStream fs(path, OpenMode::Out);
		fs << out.c_str();
	}

	void SceneSerializer::deserialize(const Path& path) {
		FileStream fs(path, OpenMode::In);
	
		YAML::Node data = YAML::Load(fs.string().data());
		if (!data["Scene"])
			return;

		String scene_name = data["Scene"].as<std::string>();
		Log::info("Deserializing scene: {0}", scene_name);

		auto entities = data["Entities"];
		if (!entities)
			return;

		auto& world = _scene->world();

		world.clear();

		for (auto entity_data : entities) {
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
			}
		}
	}
}