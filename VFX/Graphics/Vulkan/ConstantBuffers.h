#pragma once

namespace Fox {

    namespace Graphics {

	    namespace Vulkan {

			struct OldFrame {
				alignas(16) glm::mat4 model;
				alignas(16) glm::mat4 view;
				alignas(16) glm::mat4 proj;
			};

		    struct PerFrame {
			    alignas(16) glm::mat4 view;
			    alignas(16) glm::mat4 proj;
		    };

			struct PostProcess {
				alignas(16) glm::vec4 mergeOperation; // 0 = alpha blend, 1 = additive, 2 = multiplicative (.x component)
				alignas(16) glm::vec4 parameters; 
			};

			struct MeshTransforms {
				std::vector<glm::mat4> models;
			};

			struct MeshInfo {
				uint32_t vertexOffset;
				uint32_t indexOffset;
				uint32_t indexCount;
				uint32_t modelIndex;
				uint32_t materialIndex;
				float depth;
			};

			struct MeshInfos {
				std::vector<MeshInfo> meshInfos;
			};
	    }
    }
}