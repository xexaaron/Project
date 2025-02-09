#pragma once
#include "vk/VkCommon.h"

namespace aby::vk {
	
	class CmdPool {
	public:
		CmdPool() = default;
		CmdPool(VkDevice logical, std::uint32_t queue_family_idx);

		void destroy(VkDevice logical);
		void create(VkDevice logical, std::uint32_t queue_family_idx);
		explicit operator VkCommandPool();
		explicit operator const VkCommandPool() const;
	private:
		VkCommandPool m_Pool = VK_NULL_HANDLE;
	};



}