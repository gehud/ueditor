#include "ueditor/core/assets.h"

#include <uengine/core/uuid.h>
#include <uengine/core/io/file.h>
#include <uengine/core/io/directory.h>
#include <uengine/core/rendering/model.h>

#include <yaml-cpp/yaml.h>

namespace ueditor {
	Dictionary<ULong, Path> Assets::_paths;
	Dictionary<SharedPtr<Object>, ULong> Assets::_uids;
	Path Assets::_path;

	const Path& Assets::path() {
		return _path;
	}

	void Assets::path(const Path& path) {
		_path = path;
	}

	Path Assets::path(const ULong& uuid) {
		auto i = _paths.find(uuid);
		if (i != _paths.end())
			return i->value;

		return "";
	}

	ULong Assets::uuid(const SharedPtr<Object>& object) {
		auto i = _uids.find(object);
		if (i != _uids.end())
			return i->value;

		return 0;
	}

	void Assets::initialize(const Path& path) {
		Directory::for_each(path, [&](DirectoryEntry entry) {
			if (entry.is_directory()) {
				initialize(entry.path());
			} else if (entry.path().extension() == ".meta" || entry.path().extension() == ".mesh") {
				File file(entry.path(), OpenMode::In);
				YAML::Node node = YAML::Load(file.string().data());
				_paths.add(node["UUID"].as<ULong>(), entry.path());
				if (entry.path().extension() == ".mesh") {
					Assets::load<Mesh>(entry.path());
				}
			}
		});
	}

	void Assets::import(const Path& path) {
		auto absolute_path = Assets::path() + "/" + path;
		if (absolute_path.extension() == ".fbx") {
			auto model = Model(absolute_path);
			List<UUID> uuids;
			for (int i = 0; i < model.meshes().count(); i++) {
				const auto& mesh = model.meshes()[i];
				const auto& vertex_buffer = mesh->vertex_buffer(0);
				YAML::Emitter out;
				out << YAML::BeginMap;

				UUID uuid;
				uuids.add(uuid);
				out << YAML::Key << "UUID" << YAML::Value << uuid;

				out << YAML::Key << "VertexBufferLayout" << YAML::Value;
				out << YAML::BeginSeq;
				for (const auto& vertex_buffer_descriptor : mesh->vertex_buffer_layout()) {
					out << YAML::BeginMap;
					out << YAML::Key << "Format" << YAML::Value << (int)vertex_buffer_descriptor.format;
					out << YAML::Key << "Dimension" << YAML::Value << vertex_buffer_descriptor.dimension;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::Key << "VertexBufferCount" << YAML::Value << vertex_buffer->count();
				out << YAML::Key << "VertexBufferStride" << YAML::Value << vertex_buffer->stride();

				// TODO: Use layout to save vertex buffer!!!
				// const auto& layout = model.meshes()[i]->vertex_buffer_layout();
				out << YAML::Key << "VertexBufferData" << YAML::Value << YAML::BeginSeq;
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
				out << YAML::Key << "IndexBufferCount" << YAML::Value << index_buffer->count();
				out << YAML::Key << "IndexBufferFormat" << YAML::Value << (int)mesh->index_format();
				out << YAML::Key << "IndexBufferData" << YAML::Value << YAML::BeginSeq;
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

				const auto& submeshes = mesh->sub_meshes();
				out << YAML::Key << "SubmeshDescriptors" << YAML::Value << YAML::BeginSeq;
				for (int i = 0; i < submeshes.count(); i++) {
					out << YAML::BeginMap;
					out << YAML::Key << "Start" << YAML::Value << submeshes[i].start;
					out << YAML::Key << "Count" << YAML::Value << submeshes[i].count;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::EndMap;

				auto path = absolute_path.parent() + "/" + model.names()[i] + ".mesh";
				auto mesh_file = File(path, OpenMode::Out);
				mesh_file << out.c_str();
			}

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "UUID" << YAML::Value << UUID();
			out << YAML::Key << "Meshes" << YAML::Value;
			out << YAML::BeginSeq;
			for (int j = 0; j < uuids.count(); j++) {
				out << uuids[j];
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;

			auto meta_path = absolute_path + ".meta";
			auto meta_file = File(meta_path, OpenMode::Out);
			meta_file << out.c_str();
			return;
		}

		Log::warning("Connot import asset because of missing importer.");
	}

	ULong Assets::uuid(const Path& path) {
		if (path.extension() != ".meta" || path.extension() != ".mesh") {
			Log::error("Unsupported asset format.");
			return 0;
		}

		File file(path, OpenMode::In);
		YAML::Node node = YAML::Load(file.string().data());
		return node["UUID"].as<ULong>();
	}

	SharedPtr<Mesh> Assets::load_mesh(const Path& path) {
		if (path.extension() != ".mesh") {
			Log::error("Unsupported asset format.");
			return nullptr;
		}

		File file(path, OpenMode::In);
		YAML::Node node = YAML::Load(file.string().data());
		
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

		List<SubMeshDescriptor> submesh_descriptors;
		for (auto submesh_descriptor : node["SubmeshDescriptors"]) {
			submesh_descriptors.add({
				submesh_descriptor["Start"].as<int>(),
				submesh_descriptor["Count"].as<int>()
			});
		}
		mesh->sub_meshes(submesh_descriptors);

		_uids.add(mesh, node["UUID"].as<ULong>());

		return mesh;
	}
}