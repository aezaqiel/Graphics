#pragma once

#include <volk.h>

#include "Core/Log.hpp"

#define VK_CHECK(fn) do { VkResult res_ = fn; if (res_ != VK_SUCCESS) { LOG_ERROR("VK_CHECK FAILED: {}", #fn); } } while (false)
