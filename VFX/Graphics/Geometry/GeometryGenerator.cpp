#include "FoxRenderer.h"

namespace Fox {

	namespace Graphics {

		namespace Geometry {
				
			void GeometryGenerator::GenerateIcosahedron(std::vector<Fox::Graphics::Vulkan::Vertex>& outVertices, std::vector<uint32_t>& outIndices, Fox::Graphics::Vulkan::MeshInfo& outMeshInfo)
			{
				outVertices.clear();
				outIndices.clear();

				const float t = (1.0f + std::sqrt(5.0f)) * 0.5f; // golden ratio
				const float invLen = 1.0f / std::sqrt(1.0f + t * t);

				const glm::vec3 positions[12] = {
					invLen * glm::vec3(-1,  t,  0),
					invLen * glm::vec3(1,  t,  0),
					invLen * glm::vec3(-1, -t,  0),
					invLen * glm::vec3(1, -t,  0),

					invLen * glm::vec3(0, -1,  t),
					invLen * glm::vec3(0,  1,  t),
					invLen * glm::vec3(0, -1, -t),
					invLen * glm::vec3(0,  1, -t),

					invLen * glm::vec3(t,  0, -1),
					invLen * glm::vec3(t,  0,  1),
					invLen * glm::vec3(-t,  0, -1),
					invLen * glm::vec3(-t,  0,  1)
				};

				const uint32_t indices[60] = {
					0, 11, 5,  0, 5, 1,  0, 1, 7,  0, 7, 10,  0, 10, 11,
					1, 5, 9,   5, 11, 4,  11, 10, 2, 10, 7, 6, 7, 1, 8,
					3, 9, 4,   3, 4, 2,   3, 2, 6,   3, 6, 8,   3, 8, 9,
					4, 9, 5,   2, 4, 11,  6, 2, 10,  8, 6, 7,   9, 8, 1
				};

				outIndices.assign(std::begin(indices), std::end(indices));

				// Compute vertices with normals and UVs
				outVertices.reserve(12);
				for (int i = 0; i < 12; ++i)
				{
					glm::vec3 p = glm::normalize(positions[i]);
					glm::vec3 n = p;

					// spherical UV mapping
					float u = 0.5f + std::atan2(p.z, p.x) / (2.0f * glm::pi<float>());
					float v = 0.5f - std::asin(p.y) / glm::pi<float>();

					outVertices.push_back({ p, n, glm::vec2(u, v) });
				}

				outMeshInfo.vertexOffset = 0;
				outMeshInfo.indexOffset = 0;
				outMeshInfo.indexCount = static_cast<uint32_t>(outIndices.size());
				outMeshInfo.modelIndex = 0;
			}

		void GeometryGenerator::GenerateSphere(
			std::vector<Fox::Graphics::Vulkan::Vertex>& outVertices,
			std::vector<uint32_t>& outIndices,
			Fox::Graphics::Vulkan::MeshInfo& outMeshInfo,
			uint32_t sliceCount,
			uint32_t stackCount,
			float radius)
		{
			outVertices.clear();
			outIndices.clear();

			// Top vertex (north pole)
			outVertices.push_back({
				glm::vec3(0.0f, +radius, 0.0f),
				glm::vec3(0.0f, +1.0f, 0.0f),
				glm::vec2(0.0f, 0.0f)
				});

			// Compute vertices for each stack ring (latitude)
			for (uint32_t i = 1; i <= stackCount - 1; ++i)
			{
				float phi = glm::pi<float>() * i / stackCount; // polar angle

				for (uint32_t j = 0; j <= sliceCount; ++j)
				{
					float theta = 2.0f * glm::pi<float>() * j / sliceCount; // azimuth angle

					// Spherical to Cartesian
					glm::vec3 p(
						radius * sinf(phi) * cosf(theta),
						radius * cosf(phi),
						radius * sinf(phi) * sinf(theta)
					);

					glm::vec3 n = glm::normalize(p);

					glm::vec2 uv(
						theta / (2.0f * glm::pi<float>()),
						phi / glm::pi<float>()
					);

					outVertices.push_back({ p, n, uv });
				}
			}

			// Bottom vertex (south pole)
			outVertices.push_back({
				glm::vec3(0.0f, -radius, 0.0f),
				glm::vec3(0.0f, -1.0f, 0.0f),
				glm::vec2(0.0f, 1.0f)
				});

			// Compute indices for top stack
			for (uint32_t i = 1; i <= sliceCount; ++i)
			{
				outIndices.push_back(0);
				outIndices.push_back(i);
				outIndices.push_back(i + 1);
			}

			// Compute indices for inner stacks
			uint32_t baseIndex = 1;
			uint32_t ringVertexCount = sliceCount + 1;
			for (uint32_t i = 0; i < stackCount - 2; ++i)
			{
				for (uint32_t j = 0; j < sliceCount; ++j)
				{
					outIndices.push_back(baseIndex + i * ringVertexCount + j);
					outIndices.push_back(baseIndex + i * ringVertexCount + j + 1);
					outIndices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

					outIndices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
					outIndices.push_back(baseIndex + i * ringVertexCount + j + 1);
					outIndices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
				}
			}

			// Compute indices for bottom stack
			uint32_t southPoleIndex = static_cast<uint32_t>(outVertices.size() - 1);
			baseIndex = southPoleIndex - ringVertexCount;

			for (uint32_t i = 0; i < sliceCount; ++i)
			{
				outIndices.push_back(southPoleIndex);
				outIndices.push_back(baseIndex + i + 1);
				outIndices.push_back(baseIndex + i);
			}

			// Fill mesh info
			outMeshInfo.vertexOffset = 0;
			outMeshInfo.indexOffset = 0;
			outMeshInfo.indexCount = static_cast<uint32_t>(outIndices.size());
			outMeshInfo.modelIndex = 0;
		}

		void GeometryGenerator::GenerateSphereFlipped(
			std::vector<Fox::Graphics::Vulkan::Vertex>& outVertices,
			std::vector<uint32_t>& outIndices,
			std::vector<Fox::Graphics::Vulkan::MeshInfo>& outMeshInfos,
			uint32_t sliceCount,
			uint32_t stackCount,
			float radius)
		{
			outVertices.clear();
			outIndices.clear();
			outMeshInfos.clear();

			// --- Vertex generation ---
			outVertices.push_back({ {0, +radius, 0}, {0, -1, 0}, {0, 0} }); // top

			for (uint32_t i = 1; i <= stackCount - 1; ++i)
			{
				float phi = glm::pi<float>() * i / stackCount;
				for (uint32_t j = 0; j <= sliceCount; ++j)
				{
					float theta = 2.0f * glm::pi<float>() * j / sliceCount;
					glm::vec3 p(
						radius * sinf(phi) * cosf(theta),
						radius * cosf(phi),
						radius * sinf(phi) * sinf(theta)
					);
					outVertices.push_back({
						p,
						-glm::normalize(p), // flipped inward normal
						{ theta / (2.0f * glm::pi<float>()), phi / glm::pi<float>() }
						});
				}
			}

			outVertices.push_back({ {0, -radius, 0}, {0, 1, 0}, {0, 1} }); // bottom

			// --- Index generation (flipped winding) ---
			for (uint32_t i = 1; i <= sliceCount; ++i)
				outIndices.insert(outIndices.end(), { 0, i, i + 1 });

			uint32_t baseIndex = 1;
			uint32_t ringVerts = sliceCount + 1;
			for (uint32_t i = 0; i < stackCount - 2; ++i)
			{
				for (uint32_t j = 0; j < sliceCount; ++j)
				{
					outIndices.insert(outIndices.end(), {
						baseIndex + (i + 1) * ringVerts + j,
						baseIndex + i * ringVerts + j + 1,
						baseIndex + i * ringVerts + j,

						baseIndex + (i + 1) * ringVerts + j + 1,
						baseIndex + i * ringVerts + j + 1,
						baseIndex + (i + 1) * ringVerts + j
						});
				}
			}

			uint32_t southPole = static_cast<uint32_t>(outVertices.size() - 1);
			baseIndex = southPole - ringVerts;
			for (uint32_t i = 0; i < sliceCount; ++i)
				outIndices.insert(outIndices.end(), { southPole, baseIndex + i + 1, baseIndex + i });

			// --- MeshInfo subdivision ---
			const uint32_t trisPerMeshlet = 128;
			const uint32_t indicesPerMeshlet = trisPerMeshlet * 3;
			uint32_t totalIndices = static_cast<uint32_t>(outIndices.size());
			uint32_t offset = 0;
			uint32_t meshletIndex = 0;

			while (offset < totalIndices)
			{
				uint32_t count = std::min(indicesPerMeshlet, totalIndices - offset);

				// ensure count is multiple of 3 (complete triangles)
				count -= count % 3;

				Fox::Graphics::Vulkan::MeshInfo meshInfo{};
				meshInfo.vertexOffset = 0;
				meshInfo.indexOffset = offset;
				meshInfo.indexCount = count;
				meshInfo.modelIndex = 0;

				outMeshInfos.push_back(meshInfo);

				offset += count;
				meshletIndex++;
			}

			// Safety: ensure all indices are covered
			assert(offset == totalIndices && "Index split did not cover all triangles cleanly");
		}

		Fox::Graphics::Geometry::Vulkan::Mesh GeometryGenerator::GeneratePlaneMesh(uint32_t xSegments, uint32_t ySegments, float dimension, float tileScale, float tileOffset) {
			const float w2 = static_cast<float>(xSegments) * 0.5f;
			const float h2 = static_cast<float>(ySegments) * 0.5f;

			Fox::Graphics::Geometry::Vulkan::Mesh mesh;
			std::vector<uint32_t> indices;

			// --- Generate vertices ---
			for (float x = /*-static_cast<float>(w2)*/ 0.0f; x <= static_cast<float>(w2); ++x)
			{
				for (float y = /*-static_cast<float>(h2) */ 0.0f; y <= static_cast<float>(h2); ++y)
				{
					for(int i = 0; i < 2; i++) 
					{
						for (int j = 0; j < 2; j++) {
							float xPercent = static_cast<float>(x + i) / (w2 * 2.0f);
							float yPercent = 1.0f - static_cast<float>(y + j) / (h2 * 2.0f);

							Fox::Graphics::Vulkan::Vertex vertex{};
							vertex.position = glm::vec3(static_cast<float>(x + i) * dimension, static_cast<float>(y + j) * dimension, 0.0f);
							vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
							vertex.uv = glm::vec2(xPercent * tileScale + tileOffset,
								yPercent * tileScale + tileOffset);

							uint32_t index = mesh.AddVertex(vertex);
							indices.push_back(index);

						}
					}

					if (indices.size() == 4) {
						mesh.AddTriangle(indices[0], indices[1], indices[2], false);
						mesh.AddTriangle(indices[1], indices[3], indices[2], mesh.GetSubmeshes()[mesh.GetSubmeshes().size()  - 1].IsFull());
						indices.clear();
					}

				}
			}

			return mesh;
		}

		Fox::Graphics::Geometry::Vulkan::Mesh GeometryGenerator::GenerateCubeMesh(float dimension) {
				Fox::Graphics::Geometry::Vulkan::Mesh mesh;

				// Cube faces: 6 faces, 4 vertices per face
				struct Face {
					glm::vec3 v0, v1, v2, v3;
					glm::vec3 normal;
				};

				const Face faces[6] = {
					// Front (-Z)
					{ {-dimension,-dimension,-dimension}, { dimension,-dimension,-dimension}, { dimension, dimension,-dimension }, {-dimension, dimension,-dimension}, {0,0,-1} },
					// Back (+Z)
					{ { dimension,-dimension, dimension}, {-dimension,-dimension, dimension}, {-dimension, dimension, dimension }, { dimension, dimension, dimension }, {0,0,1} },
					// Left (-X)
					{ {-dimension,-dimension, dimension}, {-dimension,-dimension,-dimension}, {-dimension, dimension,-dimension}, {-dimension, dimension, dimension}, {-1,0,0} },
					// Right (+X)
					{ { dimension,-dimension,-dimension}, { dimension,-dimension, dimension}, { dimension, dimension, dimension}, { dimension, dimension,-dimension}, {1,0,0} },
					// Top (+Y)
					{ {-dimension, dimension,-dimension}, { dimension, dimension,-dimension}, { dimension, dimension, dimension}, {-dimension, dimension, dimension}, {0,1,0} },
					// Bottom (-Y)
					{ {-dimension,-dimension, dimension}, { dimension,-dimension, dimension}, { dimension, -dimension,-dimension}, {-dimension,-dimension,-dimension}, {0,-1,0} }
				};


				for (int f = 0; f < 6; ++f)
				{
					const Face& face = faces[f];

					// UVs: bottom-left, bottom-right, top-right, top-left
					glm::vec2 uvs[4] = { {0,0}, {1,0}, {1,1}, {0,1} };

					// Add vertices
					uint32_t index0 = mesh.AddVertex({ face.v0, face.normal, uvs[0] });
					uint32_t index1 = mesh.AddVertex({ face.v1, face.normal, uvs[1] });
					uint32_t index2 = mesh.AddVertex({ face.v2, face.normal, uvs[2] });
					uint32_t index3 = mesh.AddVertex({ face.v3, face.normal, uvs[3] });

					// Add indices (two triangles per face)
					mesh.AddTriangle(index0, index1, index2, false);
					mesh.AddTriangle(index2, index3, index0, false);
				}

				return mesh;
			}
		}
	}
}