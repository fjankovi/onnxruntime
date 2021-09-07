// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License

#pragma once

#include "core/framework/ortdevice.h"
#include "core/framework/provider_options.h"
#include "core/session/onnxruntime_c_api.h"

// #include "core/framework/execution_provider.h"
// #include "core/graph/indexed_sub_graph.h"
#include "core/providers/shared_library/provider_api.h"
#include "core/platform/ort_mutex.h"
#include <map>
#include "migraphx_inc.h"

namespace onnxruntime {

namespace migraphx_env_vars {
static const std::string kFP16Enable = "ORT_MIGRAPHX_FP16_ENABLE";
};

// Information needed to construct amdmigraphx execution providers.
struct MIGraphXExecutionProviderInfo {
  std::string target_device;
  int device_id {0};
};

// Information to construct kernel function state.
struct MIGraphXFuncState {
  AllocateFunc allocate_func = nullptr;
  DestroyFunc release_func = nullptr;
  AllocatorHandle allocate_handle = nullptr;
  migraphx::program prog{};
  std::string onnx_string;
  migraphx::onnx_options options;
  migraphx::target t{};
  std::unordered_map<std::string, std::size_t> input_name_indexes;
  OrtMutex* mgx_mu_ptr = nullptr;
  bool no_input_shape = false;
  bool fp16_enable = false;
};

// Logical device representation.
class MIGraphXExecutionProvider : public IExecutionProvider {
 public:
  explicit MIGraphXExecutionProvider(const MIGraphXExecutionProviderInfo& info);
  ~MIGraphXExecutionProvider() = default;

  std::vector<std::unique_ptr<ComputeCapability>>
  GetCapability(const onnxruntime::GraphViewer& graph_viewer,
                const std::vector<const KernelRegistry*>& kernel_registries) const override;

  Status Compile(const std::vector<onnxruntime::Node*>& fused_nodes,
                 std::vector<NodeComputeInfo>& node_compute_funcs) override;

  virtual std::shared_ptr<KernelRegistry> GetKernelRegistry() const override;
  std::unique_ptr<onnxruntime::IDataTransfer> GetDataTransfer() const override;
  AllocatorPtr GetAllocator(int id, OrtMemType mem_type) const override;

  // void RegisterAllocator(std::shared_ptr<AllocatorManager> allocator_manager) override;
  // Status OnRunEnd() override;

  // Status SetComputeStream(void* stream) override;

  // void* GetComputeStream() const override { return static_cast<void*>(stream_); }

  // ProviderOptions GetProviderOptions() const override {
  //   return TensorrtExecutionProviderInfo::ToProviderOptions(info_);
  // }

  std::unique_ptr<IndexedSubGraph> GetSubGraph(const std::vector<std::size_t>& graph_nodes_index, const GraphViewer& graph) const;

private:
  bool fp16_enable_ = false;
  int device_id_;
  migraphx::target t_; 
  OrtMutex mgx_mu_;

  std::unordered_map<std::string, migraphx::program> map_progs_;
  std::unordered_map<std::string, std::string> map_onnx_string_;
  std::unordered_map<std::string, std::unordered_map<std::string, std::size_t>> map_input_index_;
  std::unordered_map<std::string, bool> map_no_input_shape_;

  AllocatorPtr allocator_;
};

}
