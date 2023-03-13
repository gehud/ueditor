#include "ueditor/core/assets.h"

#include <uengine/core/uuid.h>
#include <uengine/core/io/file.h>
#include <uengine/core/io/directory.h>
#include <uengine/core/rendering/model.h>

#include <yaml-cpp/yaml.h>

namespace ueditor {
	Dictionary<WeakPtr<Object>, ULong> Assets::_loaded;
	Path Assets::_assets;	
	Path Assets::_cache;

	const Path& Assets::path() {
		return _assets;
	}

	Path Assets::path(const ULong& uuid) {
		File file(_cache / "assets");
		YAML::Node node = YAML::Load(file.string().data());
		if (node[uuid])
			return String(node[uuid]["Path"].as<std::string>());
		return "";
	}

	ULong Assets::uuid(const SharedPtr<Object>& object) {
		auto i = _loaded.find_if([&](const auto& pair) { return pair.key().get() == object.get(); });
		if (i != _loaded.end())
			return i->value();
		return 0;
	}

	ULong Assets::uuid(const Path& path) {
		if (!path.exists()) {
			Log::error("Path does not exists.");
			return 0;
		}
		File file(path, FileMode::In);
		YAML::Node node = YAML::Load(file.string().data());
		return node["Self"].as<ULong>();
	}

	List<ULong> Assets::uuids(const Path& path) {
		if (!path.exists()) {
			Log::error("Path does not exists.");
			return {};
		}

		File file(path, FileMode::In);
		YAML::Node node = YAML::Load(file.string().data());
		ULong self = node["Self"].as<ULong>();
		int sub_asset_count = 0;
		if (node["SubAssets"])
			sub_asset_count = node["SubAssets"].size();
		List<ULong> result;
		result.add(self);
		node = node["SubAssets"];
		for (int i = 0; i < sub_asset_count; i++)
			result.add(node[i].as<ULong>());

		return uengine::move(result);
	}

	String Assets::name(const ULong& uuid) {
		File artifacts(_cache / "artifacts", FileMode::In);
		YAML::Node node = YAML::Load(artifacts.string().data());
		if (node[uuid] && node[uuid]["Name"])
			return node[uuid]["Name"].as<std::string>();
		return "";
	}

	String Assets::type(const ULong& uuid) {
		File artifacts(_cache / "artifacts", FileMode::In);
		YAML::Node node = YAML::Load(artifacts.string().data());
		if (node[uuid] && node[uuid]["Type"])
			return node[uuid]["Type"].as<std::string>();
		return "";
	}

	bool Assets::is_complex(const Path& path) {
		if (!path.exists()) {
			Log::error("Path does not exists.");
			return false;
		}
		
		File file(path, FileMode::In);
		YAML::Node node = YAML::Load(file.string().data());
		return node["SubAssets"].IsDefined();

		return false;
	}

	void Assets::import_all(const Path& path, ImportMode import_mode) {
		Directory::for_each(path, [&](auto entry) {
			if (entry.path().extension() == ".glsl" 
			|| entry.path().extension() == ".material" 
			|| entry.path().extension() == ".fbx") {
				import(entry.path(), import_mode);
			} else if (entry.is_directory()) {
				import_all(entry.path());
			}
		});
	}

	void Assets::initialize(const Path& assets, const Path& cache) {
		_assets = assets;
		_cache = cache;
		import_all(_assets);
	}

	void Assets::import(const Path& path, ImportMode import_mode) {
		auto absolute_path = Assets::path() / path;
		if (absolute_path.extension() == ".glsl") {
			import_shader(absolute_path, import_mode);
			return;
		} else if (absolute_path.extension() == ".material") {
			import_material(absolute_path, import_mode);
			return;
		} else if (absolute_path.extension() == ".fbx") {
			import_model(absolute_path, import_mode);
			return;
		}

		Log::error("Connot import asset.");
	}


	void Assets::import_shader(const Path& path, ImportMode import_mode) {
		File meta_file(path + ".meta", FileMode::In);
		YAML::Node meta_file_node = YAML::Load(meta_file.string().data());
		if (meta_file_node["Self"] && import_mode == ImportMode::Default)
			return;
		
		meta_file.close();

		UUID uuid;
	
		File source_file(path, FileMode::In);
		YAML::Emitter artifacts_out;
		artifacts_out << YAML::BeginMap;
		artifacts_out << YAML::Key << uuid << YAML::Value;
		artifacts_out << YAML::BeginMap; 
		artifacts_out << YAML::Key << "Type" << YAML::Value << "Shader";
		artifacts_out << YAML::Key << "Source" << YAML::Value << source_file.string().data();
		artifacts_out << YAML::EndMap; 
		artifacts_out << YAML::EndMap;
		auto artifacts_path = _cache / "artifacts";
		File artifacts_file(artifacts_path, FileMode::Out | FileMode::Append);
		artifacts_file << artifacts_out.c_str() << '\n';

		YAML::Emitter meta_out;
		meta_out << YAML::BeginMap;
		meta_out << YAML::Key << "Self" << YAML::Value << uuid;
		meta_out << YAML::EndMap;
		meta_file.open(path + ".meta", FileMode::Out);
		meta_file << meta_out.c_str();

		YAML::Emitter assets_out;
		assets_out << YAML::BeginMap;
		assets_out << YAML::Key << uuid << YAML::Value;
		assets_out << YAML::BeginMap;
		assets_out << YAML::Key << "Path" << YAML::Value << path.relative(_assets).string().data();
		assets_out << YAML::EndMap;
		assets_out << YAML::EndMap;
		File assets_file(_cache / "assets", FileMode::Out | FileMode::Append);
		assets_file << assets_out.c_str() << '\n';
	}

	void Assets::import_material(const Path& path, ImportMode import_mode) {
		File meta_file(path + ".meta", FileMode::In);
		YAML::Node meta_file_node = YAML::Load(meta_file.string().data());
		if (meta_file_node["Self"] && import_mode == ImportMode::Default)
			return;
		
		meta_file.close();

		UUID uuid;

		YAML::Emitter artifats_out;
		artifats_out << YAML::BeginMap;
		artifats_out << YAML::Key << uuid << YAML::Value;
		artifats_out << YAML::BeginMap;
		artifats_out << YAML::Key << "Type" << YAML::Value << "Material";
		artifats_out << YAML::EndMap;
		artifats_out << YAML::EndMap;
		File artifacts(_cache / "artifacts", FileMode::Out | FileMode::Append);
		artifacts << artifats_out.c_str() << '\n';

		YAML::Emitter meta_out;
		meta_out << YAML::BeginMap;
		meta_out << YAML::Key << "Self" << YAML::Value << uuid;
		meta_out << YAML::EndMap;
		meta_file.open(path + ".meta", FileMode::Out);
		meta_file << meta_out.c_str();

		YAML::Emitter assets_out;
		assets_out << YAML::BeginMap;
		assets_out << YAML::Key << uuid << YAML::Value;
		assets_out << YAML::BeginMap;
		assets_out << YAML::Key << "Path" << YAML::Value << path.relative(_assets).string().data();
		assets_out << YAML::EndMap;
		assets_out << YAML::EndMap;
		File assets_file(_cache / "assets", FileMode::Out | FileMode::Append);
		assets_file << assets_out.c_str() << '\n';	
	}

	void Assets::import_model(const Path& path, ImportMode import_mode) {
		File meta_file(path + ".meta", FileMode::In);
		YAML::Node meta_file_node = YAML::Load(meta_file.string().data());
		if (meta_file_node["Self"] && import_mode == ImportMode::Default)
			return;

		meta_file.close();

		Model model(path);
		List<UUID> uuids;	
		for (int i = 0; i < model.meshes().count(); i++) {
			const auto& mesh = model.meshes()[i];
			const auto& vertex_buffer = mesh->vertex_buffer(0);
			YAML::Emitter out;
			UUID uuid;
			uuids.add(uuid);

			out << YAML::BeginMap;
			out << YAML::Key << uuid << YAML::Value;

			out << YAML::BeginMap;
			out << YAML::Key << "Type" << YAML::Value << "Mesh";
			out << YAML::Key << "Name" << YAML::Value << model.names()[i].data();
			out << YAML::Key << "VertexBufferLayout" << YAML::Value;
			out << YAML::BeginSeq;
			for (const auto& vertex_buffer_descriptor : mesh->vertex_buffer_layout()) {
				out << YAML::BeginMap;
				out << YAML::Key << "Format" << YAML::Value << (int)vertex_buffer_descriptor.format;
				out << YAML::Key << "Dimension" << YAML::Value << vertex_buffer_descriptor.dimension;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::Key << "VertexBufferStride" << YAML::Value << vertex_buffer->stride();

			// TODO: Use layout to save vertex buffer!!!
			// const auto& layout = model.meshes()[i]->vertex_buffer_layout();
			out << YAML::Key << "VertexBufferData" << YAML::Value << YAML::Flow << YAML::BeginSeq;
			// Temporary.
			auto count = vertex_buffer->count() * vertex_buffer->stride() / 4;
			auto vertices = new float[count]; 
			vertex_buffer->data(vertices, 0, vertex_buffer->count());
			for (int j = 0; j < count; j++) {
				out << vertices[j];
			}
			delete[] vertices;
			out << YAML::EndSeq;
			
			auto& index_buffer = mesh->index_buffer();
			out << YAML::Key << "IndexBufferFormat" << YAML::Value << (int)mesh->index_format();
			out << YAML::Key << "IndexBufferData" << YAML::Value << YAML::Flow << YAML::BeginSeq;
			switch (mesh->index_format()) {
			case IndexFormat::UShort: {
				auto indices = new unsigned short[index_buffer->count()];
				index_buffer->data(indices, 0, index_buffer->count());
				for (int j = 0; j < index_buffer->count(); j++) {
					out << indices[j];
				}
				delete[] indices;
				break;
			}
			case IndexFormat::UInt: {
				auto indices = new unsigned int[index_buffer->count()];
				index_buffer->data(indices, 0, index_buffer->count());
				for (int j = 0; j < index_buffer->count(); j++) {
					out << indices[j];
				}	
				delete[] indices;
				break;
			}
			}
			out << YAML::EndSeq;

			const auto& sub_meshes = mesh->sub_meshes();
			out << YAML::Key << "SubMeshes" << YAML::Value << YAML::BeginSeq;
			for (int i = 0; i < sub_meshes.count(); i++) {
				out << YAML::BeginMap;
				out << YAML::Key << "Start" << YAML::Value << sub_meshes[i].start;
				out << YAML::Key << "Count" << YAML::Value << sub_meshes[i].count;
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;

			out << YAML::EndMap;
			out << YAML::EndMap;

			auto artifacts_path = _cache / "artifacts";
			File artifacts_file(artifacts_path, FileMode::Out | FileMode::Append);
			artifacts_file << out.c_str() << '\n';
		}

		YAML::Emitter out;
		UUID uuid;
		out << YAML::BeginMap;
		out << YAML::Key << "Self" << YAML::Value << uuid;
		out << YAML::Key << "SubAssets" << YAML::Value;
		out << YAML::BeginSeq;
		for (int j = 0; j < uuids.count(); j++) {
			out << uuids[j];
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		File file(path + ".meta", FileMode::Out);
		file << out.c_str();

		File assets_file(_cache / "assets", FileMode::Out | FileMode::Append);
		YAML::Emitter assets_out;
		assets_out << YAML::BeginMap;
		assets_out << YAML::Key << uuid << YAML::Value;
		assets_out << YAML::BeginMap;
		assets_out << YAML::Key << "Path" << YAML::Value << path.relative(_assets).string().data();
		assets_out << YAML::Key << "SubAssets" << YAML::Value;
		assets_out << YAML::BeginSeq;
		for (int j = 0; j < uuids.count(); j++) {
			assets_out << uuids[j];
		}
		assets_out << YAML::EndSeq;
		assets_out << YAML::EndMap;
		assets_out << YAML::EndMap;
		assets_file << assets_out.c_str() << '\n';
	}

	SharedPtr<Shader> Assets::load_shader(const ULong& uuid) {
		File artifacts(_cache / "artifacts", FileMode::In);
		YAML::Node node = YAML::Load(artifacts.string().data());
		if (!node[uuid]) {
			Log::error("Failed to load shader.");
			return nullptr;
		}

		artifacts.close();

		auto i = _loaded.find_if([&](const auto& pair) { return pair.value() == uuid; });
		if (i != _loaded.end()) {
			if (i->key().is_valid())
				return static_pointer_cast<Shader>(SharedPtr<Object>(i->key()));
			else 
				_loaded.remove(i);
		}

		String source = node[uuid]["Source"].as<std::string>(); 

		int vertex_shader_position = source.find_first_of("#type vertex");
		if (vertex_shader_position == -1) {
			Log::error("Syntax error.");
			return nullptr;
		}

		int fragment_shader_position = source.find_first_of("#type fragment");
		if (fragment_shader_position == -1) {
			Log::error("Syntax error.");
			return nullptr;
		}

		auto vertex_source = source.substring(vertex_shader_position, fragment_shader_position);
		auto fragment_source = source.substring(fragment_shader_position);

		auto shader = make_shared<Shader>(vertex_source, fragment_source);
		_loaded.add(shader, uuid);
		return shader;
	}

	SharedPtr<Material> Assets::load_material(const ULong& uuid) {
		File assets(_cache / "assets", FileMode::In);
		YAML::Node node = YAML::Load(assets.string().data());
		if (!node[uuid]) {
			Log::error("Failed to load material.");
			return nullptr;
		}
		
		node = node[uuid];

		auto i = _loaded.find_if([&](const auto& pair) { return pair.value() == uuid; });
		if (i != _loaded.end()) {
			if (i->key().is_valid())
				return static_pointer_cast<Material>(SharedPtr<Object>(i->key()));
			else
				_loaded.remove(i);
		}

		File meta(_assets / String(node["Path"].as<std::string>()), FileMode::In);
		YAML::Node meta_node = YAML::Load(meta.string().data());
		SharedPtr<Shader> shader;
		if (meta_node["Shader"])
			shader = load_shader(meta_node["Shader"].as<ULong>());

		if (!shader)
			shader = make_shared<Shader>("../assets/shaders/fallback.glsl");

		auto material = make_shared<Material>(shader);
		_loaded.add(material, uuid);
		return material;
	}

	SharedPtr<Mesh> Assets::load_mesh(const ULong& uuid) {
		File file(_cache / "artifacts", FileMode::In);
		YAML::Node node = YAML::Load(file.string().data());
		if (!node[uuid]) {
			Log::error("Failed to load mesh.");
			return nullptr;
		}

		file.close();

		auto i = _loaded.find_if([&](const auto& pair) { return pair.value() == uuid; });
		if (i != _loaded.end() && i->key().is_valid())
			return static_pointer_cast<Mesh>(SharedPtr<Object>(i->key()));

		node = node[uuid];
		
		auto mesh = make_shared<Mesh>();
		
		List<VertexAttributeDescriptor> vertex_attribute_descriptors;
		for (auto vertex_attribute_descriptor : node["VertexBufferLayout"]) {
			vertex_attribute_descriptors.add({
				(VertexAttributeFormat)vertex_attribute_descriptor["Format"].as<int>(), 
				vertex_attribute_descriptor["Dimension"].as<int>()
			});
		}

		// Temporary.
		List<float> vertices;
		for (auto vertex : node["VertexBufferData"]) {
			vertices.add(vertex.as<float>());
		}

		mesh->vertex_buffer_params(vertices.count() / (node["VertexBufferStride"].as<int>() / 4), 
			{vertex_attribute_descriptors});
		mesh->vertex_buffer_data(vertices.data());

		switch ((IndexFormat)node["IndexBufferFormat"].as<int>()) {
		case IndexFormat::UShort: {
			List<unsigned short> indices;
			for (auto index : node["IndexBufferData"]) {
				indices.add(index.as<unsigned short>());
			}
			mesh->index_buffer_params(indices.count(), IndexFormat::UShort);
			mesh->index_buffer_data(indices.data());
			break;
		}
		case IndexFormat::UInt: {
			List<unsigned int> indices;
			for (auto index : node["IndexBufferData"]) {
				indices.add(index.as<unsigned int>());
			}
			mesh->index_buffer_params(indices.count(), IndexFormat::UInt);
			mesh->index_buffer_data(indices.data());
			break;
		}
		}

		List<SubMeshDescriptor> sub_mesh_descriptors;
		for (auto sub_mesh_descriptor : node["SubMeshes"]) {
			sub_mesh_descriptors.add({
				sub_mesh_descriptor["Start"].as<int>(),
				sub_mesh_descriptor["Count"].as<int>()
			});
		}
		mesh->sub_meshes(sub_mesh_descriptors);

		_loaded.add(mesh, uuid);

		return mesh;
	}
}