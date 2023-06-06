
#if defined(AUTOMATA_ENGINE_VK_BACKEND)

#define VOLK_IMPLEMENTATION
#include <automata_engine.hpp>

// TODO: maybe we can have some sort of AE com_release idea?
#define COM_RELEASE(comPtr) (comPtr != nullptr) ? comPtr->Release() : 0;

// TODO: maybe we can have like an AE_vk_check kind of idea?
#define VK_CHECK(x)                                                                                                    \
    do {                                                                                                               \
        VkResult err = x;                                                                                              \
        if (err) {                                                                                                     \
            AELoggerError("Detected Vulkan error: %s", ae::VK::VkResultToString(err));                                 \
            abort();                                                                                                   \
        }                                                                                                              \
    } while (0)

namespace automata_engine {
    namespace VK {

        // TODO: so since we can have VK on different platforms, it would be nice to use things that are not WCHAR here.
        VkShaderModule loadShaderModule(
            VkDevice vkDevice, const char *filePathIn, const WCHAR *entryPoint, const WCHAR *profile)
        {
            IDxcBlob *spirvBlob;
            defer(COM_RELEASE(spirvBlob));

            bool emitSpirv = true;
            ae::HLSL::compileBlobFromFile(filePathIn, entryPoint, profile, &spirvBlob, emitSpirv);

            if (spirvBlob->GetBufferPointer()) {
                VkShaderModuleCreateInfo module_info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
                module_info.codeSize                 = spirvBlob->GetBufferSize();
                module_info.pCode                    = (uint32_t *)spirvBlob->GetBufferPointer();

                VkShaderModule shader_module;
                VK_CHECK(vkCreateShaderModule(vkDevice,
                    &module_info,
                    nullptr,  // this is an allocator. if we provided one it means the VK driver would not alloc and would use our impl to alloc. this is an alloc on the host (system RAM, not VRAM).
                    &shader_module));
                return shader_module;
            }

            return VK_NULL_HANDLE;
        }

        const char *VkResultToString(VkResult result)
        {
#define STR(r)                                                                                                         \
    case VK_##r:                                                                                                       \
        return #r

            switch (result) {
                STR(NOT_READY);
                STR(TIMEOUT);
                STR(EVENT_SET);
                STR(EVENT_RESET);
                STR(INCOMPLETE);
                STR(ERROR_OUT_OF_HOST_MEMORY);
                STR(ERROR_OUT_OF_DEVICE_MEMORY);
                STR(ERROR_INITIALIZATION_FAILED);
                STR(ERROR_DEVICE_LOST);
                STR(ERROR_MEMORY_MAP_FAILED);
                STR(ERROR_LAYER_NOT_PRESENT);
                STR(ERROR_EXTENSION_NOT_PRESENT);
                STR(ERROR_FEATURE_NOT_PRESENT);
                STR(ERROR_INCOMPATIBLE_DRIVER);
                STR(ERROR_TOO_MANY_OBJECTS);
                STR(ERROR_FORMAT_NOT_SUPPORTED);
                STR(ERROR_SURFACE_LOST_KHR);
                STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
                STR(SUBOPTIMAL_KHR);
                STR(ERROR_OUT_OF_DATE_KHR);
                STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
                STR(ERROR_VALIDATION_FAILED_EXT);
                STR(ERROR_INVALID_SHADER_NV);
                default:
                    return "UNKNOWN_ERROR";
            }

#undef STR
        }

        VKAPI_ATTR VkBool32 VKAPI_CALL ValidationDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT                                                           messageType,
            const VkDebugUtilsMessengerCallbackDataEXT                                               *pCallbackData,
            void                                                                                     *pUserData)
        {
            if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
                AELoggerError("Validation Layer: Error: %s", pCallbackData->pMessage);
            } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                AELoggerWarn("Validation Layer: Warning: %s", pCallbackData->pMessage);
            } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
                AELoggerLog("Validation Layer: Info: %s", pCallbackData->pMessage);
            }
#if 0
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		AELoggerLog("Validation Layer: Verbose: %s", pCallbackData->pMessage);
	}
#endif
            return VK_FALSE;
        }

    }  // namespace VK
};     // namespace automata_engine

#endif