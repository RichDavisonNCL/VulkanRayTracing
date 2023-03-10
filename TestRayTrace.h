/******************************************************************************
This file is part of the Newcastle Vulkan Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VulkanTutorialRenderer.h"
#include "../VulkanRendering/VulkanBVHBuilder.h"

namespace NCL::Rendering {
	class TestRayTrace : public VulkanTutorialRenderer
	{
	public:
		TestRayTrace(Window& window);
		~TestRayTrace();

		void SetupTutorial() override;

		void RenderFrame() override;
		void Update(float dt) override;

	protected:
		VulkanPipeline		pipeline;
		UniqueVulkanMesh	triMesh;
		VulkanBVH			sceneBVH;
	};
}

