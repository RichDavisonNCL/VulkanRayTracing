/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanShaderBindingTableBuilder.h"
#include "VulkanBufferBuilder.h"

using namespace NCL;
using namespace Rendering;

VulkanShaderBindingTableBuilder::VulkanShaderBindingTableBuilder(const std::string& inDebugName) {
	debugName = debugName;
}

VulkanShaderBindingTableBuilder::~VulkanShaderBindingTableBuilder() {

}

VulkanShaderBindingTableBuilder& VulkanShaderBindingTableBuilder::WithProperties(vk::PhysicalDeviceRayTracingPipelinePropertiesKHR deviceProps) {
	properties = deviceProps;
	return *this;
}

VulkanShaderBindingTableBuilder& VulkanShaderBindingTableBuilder::WithPipeline(vk::Pipeline inPipe, const vk::RayTracingPipelineCreateInfoKHR& inPipeCreateInfo) {
	pipeline = inPipe;
	pipeCreateInfo = &inPipeCreateInfo;
	return *this;
}

VulkanShaderBindingTableBuilder& VulkanShaderBindingTableBuilder::WithLibrary(const vk::RayTracingPipelineCreateInfoKHR& createInfo) {
	libraries.push_back(&createInfo);
	return *this;
}

void VulkanShaderBindingTableBuilder::FillIndices(const vk::RayTracingPipelineCreateInfoKHR* fromInfo, int& offset) {
	for (int i = 0; i < fromInfo->groupCount; ++i) {
		if (fromInfo->pGroups[i].type == vk::RayTracingShaderGroupTypeKHR::eGeneral) {
			int shaderType = fromInfo->pGroups[i].generalShader;

			if (fromInfo->pStages[shaderType].stage == vk::ShaderStageFlagBits::eRaygenKHR) {
				handleIndices[(int)BindingTableOrder::RayGen].push_back(i + offset);
			}
			else if (fromInfo->pStages[shaderType].stage == vk::ShaderStageFlagBits::eMissKHR) {
				handleIndices[(int)BindingTableOrder::Miss].push_back(i + offset);
			}
			else if(fromInfo->pStages[shaderType].stage == vk::ShaderStageFlagBits::eCallableKHR) {
				handleIndices[(int)BindingTableOrder::Call].push_back(i + offset);
			}
		}
		else { //Must be a hit group
			handleIndices[(int)BindingTableOrder::Hit].push_back(i + offset);
		}
	}
	offset += fromInfo->groupCount;
}

int MakeMultipleOf(int input, int multiple) {
	int count = input / multiple;
	int r = input % multiple;
	
	if (r != 0) {
		count += 1;
	}

	return count * multiple;
}

ShaderBindingTable VulkanShaderBindingTableBuilder::Build(vk::Device device, VmaAllocator allocator) {
	assert(pipeCreateInfo);
	assert(pipeline);

	ShaderBindingTable table;

	uint32_t handleCount = 1;
	uint32_t handleSize = properties.shaderGroupHandleSize;
	uint32_t alignedHandleSize = handleSize; //TODO

	uint32_t groupAlign = handleSize; //TODO

	table.regions[(int)BindingTableOrder::RayGen].stride = groupAlign;
	table.regions[(int)BindingTableOrder::Hit].stride	= alignedHandleSize;
	table.regions[(int)BindingTableOrder::Miss].stride	= alignedHandleSize;
	table.regions[(int)BindingTableOrder::Call].stride	= alignedHandleSize;

	table.regions[(int)BindingTableOrder::RayGen].size	= 0;
	table.regions[(int)BindingTableOrder::Hit].size		= 0;
	table.regions[(int)BindingTableOrder::Miss].size	= 0;
	table.regions[(int)BindingTableOrder::Call].size	= 0;

	int offset = 0;
	FillIndices(pipeCreateInfo, offset);

	uint32_t numShaderGroups = pipeCreateInfo->groupCount;
	for (auto& i : libraries) {
		numShaderGroups += i->groupCount;
		FillIndices(i, offset);
	}



	uint32_t bindingCounts[(int)BindingTableOrder::SIZE];

	uint32_t totalHandleSize = numShaderGroups * handleSize;

	std::vector<uint8_t> handles(totalHandleSize);

	auto result = device.getRayTracingShaderGroupHandlesKHR(pipeline, 0, numShaderGroups, totalHandleSize, handles.data());

	uint32_t totalAllocSize = 0;
	for (int i = 0; i < 4; ++i) {
		table.regions[i].size = table.regions[i].stride * handleIndices[i].size();
		totalAllocSize += table.regions[i].size;
		totalAllocSize = MakeMultipleOf(totalAllocSize, properties.shaderGroupBaseAlignment);
	}

	table.tableBuffer = VulkanBufferBuilder(totalAllocSize, debugName + " SBT Buffer")
		.WithBufferUsage(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eShaderBindingTableKHR)
		.WithDeviceAddresses()
		.WithHostVisibility()
		.Build(device, allocator);

	vk::DeviceAddress address = device.getBufferAddress(vk::BufferDeviceAddressInfo(table.tableBuffer.buffer));

	table.regions[0].deviceAddress = address;
	for (int i = 1; i < 4; ++i) {
		table.regions[i].deviceAddress = MakeMultipleOf(table.regions[i - 1].deviceAddress + table.regions[i - 1].size, properties.shaderGroupBaseAlignment);
	}

	char* bufData = (char*)table.tableBuffer.Map();
	int dataOffset = 0;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < handleIndices[i].size(); ++j) {
			memcpy(bufData + dataOffset, handles.data() + handleIndices[i][j], handleSize);
			dataOffset += handleSize;
		}
		dataOffset = MakeMultipleOf(dataOffset, properties.shaderGroupBaseAlignment);
	}

	table.tableBuffer.Unmap();

	return table;
}