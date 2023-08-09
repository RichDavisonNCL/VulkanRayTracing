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

TestRayTrace::TestRayTrace(Window& window) : VulkanTutorialRenderer(window) , bvhBuilder("Test"){
	majorVersion = 1;
	minorVersion = 3;

	autoBeginDynamicRendering = false;
}

TestRayTrace::~TestRayTrace() {

}

void TestRayTrace::SetupDevice(vk::PhysicalDeviceFeatures2& deviceFeatures) {
	deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	deviceExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	deviceExtensions.emplace_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);

	static vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelFeatures(true);

	static vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayFeatures(true);
	rayFeatures.setRayTracingPipeline(true);

	static vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR deviceAddressfeature(true);

	deviceFeatures.setPNext(&accelFeatures);
	accelFeatures.pNext = &rayFeatures;
	rayFeatures.pNext	= &deviceAddressfeature;
	
	allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
}

void TestRayTrace::SetupTutorial() {
	VulkanTutorialRenderer::SetupTutorial();

	vk::PhysicalDeviceProperties2 props;
	props.pNext = &rayPipelineProperties;

	GetPhysicalDevice().getProperties2(&props);

	triangle = GenerateTriangle();

	raygenShader	= UniqueVulkanRTShader(new VulkanRTShader("RayTrace/raygen.rgen.spv", GetDevice())); 
	hitShader		= UniqueVulkanRTShader(new VulkanRTShader("RayTrace/closesthit.rchit.spv", GetDevice())); 
	missShader		= UniqueVulkanRTShader(new VulkanRTShader("RayTrace/miss.rmiss.spv", GetDevice()));

	rayTraceLayout = VulkanDescriptorSetLayoutBuilder("Ray Trace TLAS Layout")
		.WithAccelStructures(1)
		.Build(GetDevice());

	imageLayout = VulkanDescriptorSetLayoutBuilder("Ray Trace Image Layout")
		.WithStorageImages(1)
		.Build(GetDevice());

	inverseCamLayout = VulkanDescriptorSetLayoutBuilder("Camera Inverse Matrix Layout")
		.WithUniformBuffers(1)
		.Build(GetDevice());

	tlas = bvhBuilder
		.WithObject(&*triangle, Matrix4::Translation({ 0,0,-100.0f }) * Matrix4::Scale({2,4,2}))
		.WithCommandQueue(GetQueue(CommandBufferType::AsyncCompute))
		.WithCommandPool(GetCommandPool(CommandBufferType::AsyncCompute)) 
		.Build(GetDevice(), GetMemoryAllocator(), vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);

	rayTraceDescriptor		= BuildUniqueDescriptorSet(*rayTraceLayout);
	imageDescriptor			= BuildUniqueDescriptorSet(*imageLayout);
	inverseCamDescriptor	= BuildUniqueDescriptorSet(*inverseCamLayout);

	inverseMatrices = VulkanBufferBuilder(sizeof(Matrix4) * 2, "InverseMatrices")
		.WithBufferUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.WithHostVisibility()
		.WithPersistentMapping()
		.Build(GetDevice(), GetMemoryAllocator());

	rayTexture = VulkanTexture::CreateColourTexture(this, windowSize.x, windowSize.y, "RayTraceResult", 
		vk::Format::eR32G32B32A32Sfloat, 
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage,
		vk::ImageLayout::eGeneral
	);
	WriteStorageImageDescriptor(*imageDescriptor, 0, 0, *rayTexture, *defaultSampler, vk::ImageLayout::eGeneral);

	WriteBufferDescriptor(*inverseCamDescriptor, 0, vk::DescriptorType::eUniformBuffer, inverseMatrices);

	WriteTLASDescriptor(*rayTraceDescriptor, 0, *tlas);

	auto rtPipeBuilder = VulkanRayTracingPipelineBuilder("RT Pipeline")
		.WithRecursionDepth(1)
		.WithShader(*raygenShader, vk::ShaderStageFlagBits::eRaygenKHR)		//0
		.WithShader(*missShader, vk::ShaderStageFlagBits::eMissKHR)			//1
		.WithShader(*hitShader, vk::ShaderStageFlagBits::eClosestHitKHR)	//2

		.WithGeneralGroup(0)	//Group for the raygen shader	//Uses shader 0
		.WithGeneralGroup(1)	//Group for the miss shader		//Uses shader 1
		.WithTriangleHitGroup(2)								//Uses shader 2

		.WithDescriptorSetLayout(0, *rayTraceLayout)
		.WithDescriptorSetLayout(1, *cameraLayout)
		.WithDescriptorSetLayout(2, *inverseCamLayout)
		.WithDescriptorSetLayout(3, *imageLayout);

	rtPipeline = rtPipeBuilder.Build(GetDevice());

	bindingTable = VulkanShaderBindingTableBuilder("SBT")
		.WithProperties(rayPipelineProperties)
		.WithPipeline(rtPipeline, rtPipeBuilder.GetCreateInfo())
		.Build(GetDevice(), GetMemoryAllocator());

	//We also need some Vulkan things for displaying the result!
	displayImageLayout = VulkanDescriptorSetLayoutBuilder("Raster Image Layout")
		.WithSamplers(1, vk::ShaderStageFlagBits::eFragment)
		.Build(GetDevice());

	displayImageDescriptor = BuildUniqueDescriptorSet(*displayImageLayout);
	WriteImageDescriptor(*displayImageDescriptor, 0, 0, *rayTexture, *defaultSampler, vk::ImageLayout::eShaderReadOnlyOptimal);

	quadMesh = GenerateQuad();

	displayShader = VulkanShaderBuilder("Result Display Shader")
		.WithVertexBinary("Display.vert.spv")
		.WithFragmentBinary("Display.frag.spv")
		.Build(GetDevice());

	displayPipeline = VulkanPipelineBuilder("Result display pipeline")
		.WithVertexInputState(quadMesh->GetVertexInputState())
		.WithTopology(vk::PrimitiveTopology::eTriangleStrip)
		.WithShader(displayShader)
		.WithDescriptorSetLayout(0, *displayImageLayout)
		.WithColourFormats({ GetSurfaceFormat() })
		.WithDepthStencilFormat(depthBuffer->GetFormat())
		.Build(GetDevice());
}

void TestRayTrace::RenderFrame() {
	frameCmds.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rtPipeline);

	vk::DescriptorSet sets[4] = {
		*rayTraceDescriptor,		//Set 0
		*cameraDescriptor,			//Set 1
		*inverseCamDescriptor,		//Set 2
		*imageDescriptor			//Set 3
	};

	frameCmds.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, *rtPipeline.layout, 0, 4, sets, 0, nullptr);

	frameCmds.traceRaysKHR(
		&bindingTable.regions[(int)BindingTableOrder::RayGen],
		&bindingTable.regions[(int)BindingTableOrder::Miss],
		&bindingTable.regions[(int)BindingTableOrder::Hit],
		&bindingTable.regions[(int)BindingTableOrder::Call],
		windowSize.x, windowSize.y, 1
	);

	//Flip the texture we rendered into into display mode
	Vulkan::ImageTransitionBarrier(frameCmds, *rayTexture,
		vk::ImageLayout::eGeneral,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR,
		vk::PipelineStageFlagBits::eFragmentShader
	);

	//Now display the results on screen!
	frameCmds.bindPipeline(vk::PipelineBindPoint::eGraphics, displayPipeline);

	BeginDefaultRendering(frameCmds);
	frameCmds.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *displayPipeline.layout, 0, 1, &*displayImageDescriptor, 0, nullptr);
	SubmitDrawCall(frameCmds, *quadMesh);
	EndRendering(frameCmds);

	//Flip texture back for the next frame
	Vulkan::ImageTransitionBarrier(frameCmds, *rayTexture,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eGeneral,
		vk::ImageAspectFlagBits::eColor,
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR
	);
}

void TestRayTrace::Update(float dt) {
	VulkanTutorialRenderer::Update(dt);

	Matrix4* inverseMatrixData = (Matrix4*)inverseMatrices.Data();

	inverseMatrixData[0] = camera.BuildViewMatrix().Inverse();
	inverseMatrixData[1] = camera.BuildProjectionMatrix(hostWindow.GetScreenAspect()).Inverse();
}
