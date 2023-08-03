/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanBVHBuilder.h"
#include "VulkanRTShader.h"
#include "../VulkanTutorials/VulkanTutorialRenderer.h"
#include "VulkanShaderBindingTableBuilder.h"

namespace NCL::Rendering {
	using UniqueVulkanRTShader = std::unique_ptr<VulkanRTShader>;
	using SharedVulkanRTShader = std::shared_ptr<VulkanRTShader>;

	class TestRayTrace : public VulkanTutorialRenderer	{
	public:
		TestRayTrace(Window& window);
		~TestRayTrace();

		void SetupTutorial() override;

		void RenderFrame() override;
		void Update(float dt) override;

	protected:
		void SetupDevice(vk::PhysicalDeviceFeatures2& deviceFeatures) override;

		UniqueVulkanRTShader LoadRTShader(const string& filename);

		VulkanPipeline		pipeline;
		UniqueVulkanMesh	triMesh;
		VulkanBVH			sceneBVH;

		UniqueVulkanMesh	triangle;

		UniqueVulkanRTShader	raygenShader;
		UniqueVulkanRTShader	hitShader;
		UniqueVulkanRTShader	missShader;
		//const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR&  GetRayTracingPipelineProperties() const { return rayPipelineProperties; }
		//const vk::PhysicalDeviceAccelerationStructureFeaturesKHR& GetRayTracingAccelerationStructureProperties() const { return rayAccelFeatures; }
	

		vk::AccelerationStructureKHR	tlas;

		vk::UniqueDescriptorSetLayout	rayTraceLayout;
		vk::UniqueDescriptorSet			rayTraceDescriptor;

		vk::UniqueDescriptorSetLayout	imageLayout;
		vk::UniqueDescriptorSet			imageDescriptor;

		vk::UniqueDescriptorSetLayout	inverseCamLayout;
		vk::UniqueDescriptorSet			inverseCamDescriptor;

		VulkanBuffer					inverseMatrices;

		UniqueVulkanTexture				rayTexture;

		vk::ImageView	imageWriteView;

		ShaderBindingTable bindingTable;

		///*
		//* RayTracing Stuff!
		//*/
		vk::PhysicalDeviceRayTracingPipelinePropertiesKHR	rayPipelineProperties;
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR	rayAccelFeatures;
	};
}

