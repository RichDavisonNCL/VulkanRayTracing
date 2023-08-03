/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../VulkanRendering/VulkanRenderer.h"

namespace NCL::Maths {
	class Matrix4;
}

namespace NCL::Rendering {
	class VulkanBVH {
		vk::AccelerationStructureGeometryKHR		geometry;
		vk::AccelerationStructureBuildRangeInfoKHR	info;
		//TODO
	};

	struct VulkanBVHEntry {
		Matrix4		modelMat;
		uint32_t	meshID;
	};

	struct BLASEntry {
		vk::AccelerationStructureGeometryKHR			geometry;
		vk::AccelerationStructureBuildRangeInfoKHR		rangeInfo;
		vk::AccelerationStructureBuildGeometryInfoKHR	buildInfo;
		vk::AccelerationStructureBuildSizesInfoKHR		sizeInfo;
	};

	class VulkanBVHBuilder	{
	public:
		VulkanBVHBuilder(const std::string& debugName = "");
		~VulkanBVHBuilder();

		//VulkanBVHBuilder& WithMesh(VulkanMesh* m, const Matrix4& transform);//how to set binding table???

		VulkanBVHBuilder& WithObject(VulkanMesh* m, const Matrix4& transform);

		VulkanBVHBuilder& WithCommandQueue(vk::Queue inQueue);
		VulkanBVHBuilder& WithCommandPool(vk::CommandPool inPool);

		VulkanBVH Build(VulkanRenderer& renderer);

		vk::AccelerationStructureKHR Build(vk::Device device, VmaAllocator allocator, vk::BuildAccelerationStructureFlagsKHR flags);

	protected:

		void BuildBLAS(vk::Device device, VmaAllocator allocator, vk::BuildAccelerationStructureFlagsKHR flags);

		vk::BuildAccelerationStructureFlagsKHR flags;

		std::map<VulkanMesh*, uint32_t> uniqueMeshes;

		std::vector<VulkanBVHEntry> entries;

		std::vector<VulkanMesh*>	meshes;
		std::vector<Matrix4>		transforms;


		std::vector< BLASEntry> blasBuildInfo;

		vk::Queue queue;
		vk::CommandPool pool;

		unsigned int vertexCount;
		unsigned int indexCount;
	};
}