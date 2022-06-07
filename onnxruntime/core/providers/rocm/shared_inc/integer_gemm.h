// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once
#include <hip/hip_runtime.h>


#include "core/providers/rocm/rocm_kernel.h"

namespace onnxruntime {
namespace rocm {
Status GemmInt8(int m,
                int n,
                int k,
                int32_t alpha_matmul,
                int32_t beta_matmul,
                const int8_t* a,
                int lda,
                const int8_t* b,
                int ldb,
                int32_t* c,
                int ldc,
                const RocmKernel* rocm_kernel);
}
}
