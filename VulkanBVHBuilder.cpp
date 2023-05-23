/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "VulkanBVHBuilder.h"

#include "../VulkanRendering/VulkanMesh.h"
#include "../VulkanRendering/VulkanUtils.h"

using namespace NCL;
using namespace Rendering;

VulkanBVHBuilder::VulkanBVHBuilder(const std::string& debugName) {
	vertexCount = 0;
	indexCount	= 0;
}

VulkanBVHBuilder::~VulkanBVHBuilder() {

}

//VulkanBVHBuilder& VulkanBVHBuilder::WithMesh(VulkanMesh* m, const Matrix4& transform) {
//	meshes.push_back(m);
//	transforms.push_back(transform);
//
//	auto inserted = uniqueMeshes.insert(m);
//
//	if (inserted.second) {
//		vertexCount += m->GetVertexCount();
//		indexCount  += m->GetIndexCount();
//	}
//
//	return *this;
//}

VulkanBVHBuilder& VulkanBVHBuilder::WithObject(VulkanMesh* m, const Matrix4& transform) {
	auto savedMesh = uniqueMeshes.find(m);

	uint32_t meshID = 0;

	if (savedMesh == uniqueMeshes.end()) {
		meshes.push_back(m);
		meshID = meshes.size() - 1;
		uniqueMeshes.insert({m, meshID});
	}
	else {
		meshID = savedMesh->second;
	}

	VulkanBVHEntry entry;
	entry.modelMat	= transform;
	entry.meshID	= meshID;

	entries.push_back(entry);

	return *this;
}

VulkanBVH VulkanBVHBuilder::Build(VulkanRenderer& renderer) {
	VulkanBVH e;

	//We need to first create the BLAS entries for the unique meshes
	for (const auto& i : uniqueMeshes) {

	}

	//Now we can provide the TLAS entries for the unique objects we want to actually make
	for (const auto& i : entries) {
		vk::AccelerationStructureGeometryTrianglesDataKHR triData;
		vk::AccelerationStructureGeometryKHR geometry;
		vk::AccelerationStructureBuildRangeInfoKHR offsets;

		geometry.geometryType = vk::GeometryTypeKHR::eTriangles;

		offsets.firstVertex = 0;
		offsets.primitiveCount = meshes[i.meshID]->GetPrimitiveCount();
		offsets.primitiveOffset = 0;
		offsets.transformOffset = 0;
	}
	//Finalise our BVH

	//VulkanAccelerationStructure vBuffer = renderer.CreateBuffer(vertexCount * sizeof(Vector3), vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);
	//VulkanAccelerationStructure iBuffer = renderer.CreateBuffer(indexCount * sizeof(int), vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	//vector<unsigned int> vStarts;
	//vector<unsigned int> iStarts;

	//int vOffset = 0;
	//int iOffset = 0;

	//char* vData = (char*)renderer.GetDevice().mapMemory(vBuffer.deviceMem.get(), 0, vBuffer.allocInfo.allocationSize);
	//char* iData = (char*)renderer.GetDevice().mapMemory(iBuffer.deviceMem.get(), 0, iBuffer.allocInfo.allocationSize);

	//for (const auto& i : uniqueMeshes) {
	//	vStarts.push_back(vOffset);
	//	iStarts.push_back(iOffset);
	//	vOffset += i->GetVertexCount();
	//	iOffset += i->GetIndexCount();

	//	memcpy(vData, i->GetPositionData().data(), i->GetVertexCount() * sizeof(Vector3));
	//	memcpy(iData, i->GetIndexData().data(), i->GetIndexCount() * sizeof(unsigned int));
	//	vData += i->GetVertexCount() * sizeof(Vector3);
	//	iData += i->GetIndexCount() * sizeof(unsigned int);
	//}

	//renderer.GetDevice().unmapMemory(vBuffer.deviceMem.get());
	//renderer.GetDevice().unmapMemory(iBuffer.deviceMem.get());







	////Let's build the bottom of our tree, where the mesh data is

	//VulkanAccelerationStructure tlas = renderer.CreateBuffer(0, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);
	//VulkanAccelerationStructure blas = renderer.CreateBuffer(0, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress);

	//std::vector<vk::AccelerationStructureInstanceKHR> accelStructures(meshes.size());



	//for (int i = 0; i < meshes.size(); ++i) {
	//	VulkanMesh* mesh	= meshes[i];
	//	Matrix4 matrix		= transforms[i].Transposed();

	//	memcpy(&accelStructures[i].transform.matrix, matrix.array, sizeof(float) * 12);
	//}

	////Now that the bottom 
	//vk::AccelerationStructureGeometryKHR			tlasGeometry;
	//vk::AccelerationStructureBuildGeometryInfoKHR	tlasGeomInfo;
	//vk::AccelerationStructureBuildSizesInfoKHR		tlasSizeInfo;


	//std::vector< vk::AccelerationStructureBuildRangeInfoKHR>  geomRanges(meshes.size());
	//std::vector< vk::AccelerationStructureBuildRangeInfoKHR*> geomRangePointers(meshes.size());

	//for (int i = 0; i < meshes.size(); ++i) {
	//	vk::AccelerationStructureBuildRangeInfoKHR& range = geomRanges[i];

	//	geomRangePointers[i] = &geomRanges[i];
	//}

	//bool hostBuild = renderer.GetRayTracingAccelerationStructureProperties().accelerationStructureHostCommands;

	//if (hostBuild) {
	//	vk::Device device = renderer.GetDevice();

	//	auto r = device.buildAccelerationStructuresKHR({}, 1, &tlasGeomInfo, geomRangePointers.data(), *Vulkan::dispatcher);
	//}
	//else {
	//	vk::CommandBuffer cmd = renderer.BeginCmdBuffer();

	//	cmd.buildAccelerationStructuresKHR(1, &tlasGeomInfo, geomRangePointers.data(), *Vulkan::dispatcher);

	//	renderer.SubmitCmdBufferWait(cmd);
	//}

	return e;
}