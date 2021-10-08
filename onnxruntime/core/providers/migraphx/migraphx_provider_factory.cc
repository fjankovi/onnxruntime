// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License

#include "core/providers/shared_library/provider_api.h"
#include "core/providers/migraphx/migraphx_provider_factory.h"
#include "migraphx_execution_provider.h"
#include "core/framework/provider_options.h"
#include <atomic>

#include "core/session/onnxruntime_c_api.h"

using namespace onnxruntime;

void Shutdown_DeleteRegistry();

namespace onnxruntime {

struct MIGraphXProviderFactory : IExecutionProviderFactory {
  MIGraphXProviderFactory(const MIGraphXExecutionProviderInfo& info) : info_{info} {}
  ~MIGraphXProviderFactory() override {}

  std::unique_ptr<IExecutionProvider> CreateProvider() override;

 private:
  MIGraphXExecutionProviderInfo info_;
};

std::unique_ptr<IExecutionProvider> MIGraphXProviderFactory::CreateProvider() {
  return std::make_unique<MIGraphXExecutionProvider>(info_);
}

std::shared_ptr<IExecutionProviderFactory> CreateExecutionProviderFactory_MIGraphX(int device_id) {
  MIGraphXExecutionProviderInfo info;
  info.device_id = device_id;
  // info.has_trt_options = false;
  return std::make_shared<onnxruntime::MIGraphXProviderFactory>(info);
}

std::shared_ptr<IExecutionProviderFactory> CreateExecutionProviderFactory_MIGraphX(const MIGraphXExecutionProviderInfo& info) {
  return std::make_shared<onnxruntime::MIGraphXProviderFactory>(info);
}


struct ProviderInfo_MIGraphX_Impl : ProviderInfo_MIGraphX {
  std::unique_ptr<IAllocator> CreateHIPAllocator(int16_t device_id, const char* name) override {
    return std::make_unique<HIPAllocator>(device_id, name);
  }

  std::unique_ptr<IAllocator> CreateHIPPinnedAllocator(int16_t device_id, const char* name) override {
    return std::make_unique<HIPPinnedAllocator>(device_id, name);
  }

  std::unique_ptr<IDataTransfer> CreateGPUDataTransfer(void* stream) override {
    return std::make_unique<GPUDataTransfer>(static_cast<hipStream_t>(stream));
  }
} g_info;

struct MIGraphX_Provider : Provider {
  std::shared_ptr<IExecutionProviderFactory> CreateExecutionProviderFactory(int device_id) override {
    MIGraphXExecutionProviderInfo info;
    info.device_id = device_id;
    return std::make_shared<MIGraphXProviderFactory>(info);
  }


  int migraphx_fp16_enable;                     // enable MIGraphX FP16 precision. Default 0 = false, nonzero = true
  int migraphx_int8_enable;                     // enable MIGraphX INT8 precision. Default 0 = false, nonzero = true

  std::shared_ptr<IExecutionProviderFactory> CreateExecutionProviderFactory(const void* provider_options) override {
    auto& options = *reinterpret_cast<const OrtMIGraphXProviderOptions*>(provider_options);
    MIGraphXExecutionProviderInfo info;
    info.device_id = options.device_id;
    info.fp16_enable = options.migraphx_fp16_enable;
    info.int8_enable = options.migraphx_int8_enable;
    return std::make_shared<MIGraphXProviderFactory>(info);
  }

  void UpdateProviderOptions(void* provider_options, const ProviderOptions& options) override {
    auto internal_options = onnxruntime::MIGraphXExecutionProviderInfo::FromProviderOptions(options);
    auto& trt_options = *reinterpret_cast<OrtMIGraphXProviderOptions*>(provider_options);
    trt_options.device_id = internal_options.device_id;
    trt_options.migraphx_fp16_enable = internal_options.fp16_enable;
    trt_options.migraphx_int8_enable = internal_options.int8_enable;
  }

  ProviderOptions GetProviderOptions(const void* provider_options) override {
    auto& options = *reinterpret_cast<const OrtMIGraphXProviderOptions*>(provider_options);
    return onnxruntime::MIGraphXExecutionProviderInfo::ToProviderOptions(options);
  }

  void Shutdown() override {
    Shutdown_DeleteRegistry();
  }

} g_provider;

}  // namespace onnxruntime

extern "C" {

ORT_API(onnxruntime::Provider*, GetProvider) {
  return &onnxruntime::g_provider;
}

}
