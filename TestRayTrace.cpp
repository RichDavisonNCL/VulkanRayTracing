/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "TestRayTrace.h"
#include "../GLTFLoader/GLTFLoader.h"
using namespace NCL;
using namespace Rendering;

//VulkanInitInfo info(
//	{
//		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
//		VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
//		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
//		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
//		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
//		VK_KHR_SPIRV_1_4_EXTENSION_NAME,
//		VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
//	}
//	, 
//	{
//		//nullptr
//	}
//);

TestRayTrace::TestRayTrace(Window& window) : VulkanRenderer(window) {
	//EnableRayTracing();
	//GLTFLoader::LoadGLTF("DamagedHelmet.gltf", [](void) ->  MeshGeometry* {return new VulkanMesh(); });
}

TestRayTrace::~TestRayTrace() {

}

//void TestRayTrace::SetupTutorial() {
//	VulkanTutorialRenderer::SetupTutorial();
//	triMesh = GenerateTriangle();
//	triMesh->UploadToGPU(this);
//
//	sceneBVH = VulkanBVHBuilder()
//		.WithMesh(&*triMesh, Matrix4())
//		.Build(*this);
//}

void TestRayTrace::RenderFrame() {

}

void TestRayTrace::Update(float dt) {
	//vk::
}

//
//bool VulkanRenderer::EnableRayTracing() {
//	vk::PhysicalDeviceRayTracingPipelinePropertiesKHR	pipeProperties;
//	vk::PhysicalDeviceAccelerationStructureFeaturesKHR	accelFeatures;
//
//	vk::PhysicalDeviceProperties2 props;
//	props.pNext = &pipeProperties;
//
//	gpu.getProperties2(&props, *Vulkan::dispatcher);
//	//gpu.getFeatures2KHR(accelFeatures);
//
//	auto properties =
//		gpu.getProperties2<vk::PhysicalDeviceProperties2,
//		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR>();
//
//	return true;
//}
