#include "ueditor/core/assets.h"

#include <uengine/core/uuid.h>
#include <uengine/core/rendering/model.h>
#include <uengine/core/io/file.h>

#include <yaml-cpp/yaml.h>

namespace ueditor {
	Path Assets::_path;

	const Path& Assets::path() {
		return _path;
	}

	void Assets::path(const Path& path) {
		_path = path;
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
				out << YAML::Key << "Mesh" << YAML::Value << uuid;

				out << YAML::Key << "VertexBufferLayout" << YAML::Value;
				out << YAML::BeginSeq;
				for (const auto& vertex_buffer_descriptor : mesh->vertex_buffer_layout()) {
					out << YAML::BeginMap;
					out << YAML::Key << "VertexBufferDescriptor" << YAML::Value;
						out << YAML::BeginMap;
						out << YAML::Key << "VertexAttributeFormat" << YAML::Value << (int)vertex_buffer_descriptor.format;
						out << YAML::Key << "VertexAttributeDimension" << YAML::Value << vertex_buffer_descriptor.dimension;
						out << YAML::EndMap;
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
					out << YAML::BeginMap << "SubmeshDescriptor" << YAML::Value;
						out << YAML::BeginMap;
						out << YAML::Key << "Start" << YAML::Value << submeshes[i].start;
						out << YAML::Key << "Count" << YAML::Value << submeshes[i].count;
						out << YAML::EndMap;
					out << YAML::EndMap;
				}
				out << YAML::EndSeq;

				out << YAML::EndMap;

				auto mesh_file = FileStream(absolute_path.parent() + "/" + model.names()[i] + ".mesh", OpenMode::Out);
				mesh_file << out.c_str();
			}

			YAML::Emitter out;
			out << YAML::BeginMap;
			out << YAML::Key << "Model" << YAML::Value << UUID();
			out << YAML::Key << "Meshes" << YAML::Value;
			out << YAML::BeginSeq;
			for (int j = 0; j < uuids.count(); j++) {
				out << uuids[j];
			}
			out << YAML::EndSeq;
			out << YAML::EndMap;

			auto meta_path = absolute_path + ".meta";
			auto meta_file = FileStream(meta_path, OpenMode::Out);
			meta_file << out.c_str();
			return;
		}

		Log::warning("Connot import asset because of missing importer.");
	}
}