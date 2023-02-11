/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanBVHBuilder.h"
#include "../VulkanRendering/VulkanRenderer.h"

namespace NCL::Rendering {
	class TestRayTrace : public VulkanRenderer
	{
	public:
		TestRayTrace(Window& window);
		~TestRayTrace();

		//void SetupTutorial() override;

		void RenderFrame() override;
		void Update(float dt) override;

	protected:
		VulkanPipeline		pipeline;
		UniqueVulkanMesh	triMesh;
		VulkanBVH			sceneBVH;
		//bool EnableRayTracing();

		//const vk::PhysicalDeviceRayTracingPipelinePropertiesKHR&  GetRayTracingPipelineProperties() const { return rayPipelineProperties; }
		//const vk::PhysicalDeviceAccelerationStructureFeaturesKHR& GetRayTracingAccelerationStructureProperties() const { return rayAccelFeatures; }
	
		///*
		//* RayTracing Stuff!
		//*/
		//vk::PhysicalDeviceRayTracingPipelinePropertiesKHR	rayPipelineProperties;
		//vk::PhysicalDeviceAccelerationStructureFeaturesKHR	rayAccelFeatures;
	};
}

