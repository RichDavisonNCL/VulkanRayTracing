/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#include "TestRayTrace.h"
#include "VulkanRayTracingPipelineBuilder.h"
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

TestRayTrace::TestRayTrace(Window& window) : VulkanTutorialRenderer(window) {

}

TestRayTrace::~TestRayTrace() {

}

void TestRayTrace::SetupDevice(vk::PhysicalDeviceFeatures2& deviceFeatures) {
	deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	deviceExtensions.emplace_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

	static vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelFeatures;
	static vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayFeatures;
	rayFeatures.setRayTracingPipeline(true);
	static vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR deviceAddressfeature(true);

	accelFeatures.pNext = &rayFeatures;
	rayFeatures.pNext	= &deviceAddressfeature;
	deviceFeatures.setPNext(&accelFeatures);

	allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
}

void TestRayTrace::SetupTutorial() {
	VulkanTutorialRenderer::SetupTutorial();

	vk::PhysicalDeviceProperties2 props;
	props.pNext = &rayPipelineProperties;

	GetPhysicalDevice().getProperties2(&props);

	triangle = GenerateTriangle();

	sceneBVH = VulkanBVHBuilder("Scene")
		.WithObject(&(*triangle), Matrix4())
		.Build(*this);	

	raygenShader	= LoadRTShader("RayTrace/raygen.rgen.spv");
	hitShader		= LoadRTShader("RayTrace/closesthit.rchit.spv");
	missShader		= LoadRTShader("RayTrace/miss.rmiss.spv");


	rayTraceLayout = VulkanDescriptorSetLayoutBuilder("Ray Trace TLAS Layout")
		.WithAccelStructures(1)
		.Build(GetDevice());

	imageLayout = VulkanDescriptorSetLayoutBuilder("Ray Trace Image Layout")
		.WithStorageImages(1)
		.Build(GetDevice());

	inverseCamLayout = VulkanDescriptorSetLayoutBuilder("Camera Inverse Matrix Layout")
		.WithUniformBuffers(1)
		.Build(GetDevice());

	tlas = VulkanBVHBuilder("TLAS")
		.WithObject(&*triangle, Matrix4())
		.WithCommandQueue(GetQueue(CommandBufferType::AsyncCompute))
		.WithCommandPool(GetCommandPool(CommandBufferType::AsyncCompute))
		.Build(GetDevice(), GetMemoryAllocator(), {});

	rayTraceDescriptor		= BuildUniqueDescriptorSet(*rayTraceLayout);
	imageDescriptor			= BuildUniqueDescriptorSet(*imageLayout);
	inverseCamDescriptor	= BuildUniqueDescriptorSet(*inverseCamLayout);

	//vk::UniqueDescriptorSet			rayTraceDescriptor;

	inverseMatrices = VulkanBufferBuilder(sizeof(Matrix4) * 2, "InverseMatrices")
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.Build(GetDevice(), GetMemoryAllocator());

	rayTexture = VulkanTexture::CreateColourTexture(this, windowSize.x, windowSize.y, "RayTraceResult", 
		vk::Format::eR8G8B8A8Unorm, 
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		vk::ImageLayout::eGeneral
	);
	WriteStorageImageDescriptor(*imageDescriptor, 0, 0, *rayTexture, *defaultSampler, vk::ImageLayout::eGeneral);

	WriteBufferDescriptor(*inverseCamDescriptor, 0, vk::DescriptorType::eUniformBuffer, inverseMatrices);

	WriteTLASDescriptor(*rayTraceDescriptor, 0, tlas);

	auto pipeBuilder = VulkanRayTracingPipelineBuilder("RT Pipeline")
		.WithRecursionDepth(1)
		.WithShader(*raygenShader, vk::ShaderStageFlagBits::eRaygenKHR)		//0
		.WithShader(*missShader, vk::ShaderStageFlagBits::eMissKHR)			//1
		.WithShader(*hitShader, vk::ShaderStageFlagBits::eClosestHitKHR)	//2

		.WithGeneralGroup(0)	//Group for the raygen shader
		.WithGeneralGroup(1)	//Group for the miss shader

		.WithTriangleHitGroup(2)

		.WithDescriptorSetLayout(0, *rayTraceLayout)
		.WithDescriptorSetLayout(1, *cameraLayout)
		.WithDescriptorSetLayout(2, *inverseCamLayout)
		.WithDescriptorSetLayout(3, *imageLayout);

	pipeline = pipeBuilder.Build(GetDevice());

	bindingTable = VulkanShaderBindingTableBuilder("SBT")
		.WithProperties(rayPipelineProperties)
		.WithPipeline(pipeline, pipeBuilder.GetCreateInfo())

		.Build(GetDevice(), GetMemoryAllocator());
}

UniqueVulkanRTShader TestRayTrace::LoadRTShader(const string& filename) {
	VulkanRTShader* rtShader = new VulkanRTShader(filename, GetDevice());

	return UniqueVulkanRTShader(rtShader);
}

void TestRayTrace::RenderFrame() {
	frameCmds.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline);

	vk::DescriptorSet sets[4] = {
		*rayTraceDescriptor,		//Set 0
		*cameraDescriptor,			//Set 1
		*inverseCamDescriptor,		//Set 2
		*imageDescriptor			//Set 3
	};

	frameCmds.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, *pipeline.layout, 0, 4, sets, 0, nullptr);

	frameCmds.traceRaysKHR(
		&bindingTable.regions[(int)BindingTableOrder::RayGen],
		&bindingTable.regions[(int)BindingTableOrder::Miss],
		&bindingTable.regions[(int)BindingTableOrder::Hit],
		&bindingTable.regions[(int)BindingTableOrder::Call],
		windowSize.x, windowSize.y, 1 
	);
}

void TestRayTrace::Update(float dt) {
	//vk::
}
