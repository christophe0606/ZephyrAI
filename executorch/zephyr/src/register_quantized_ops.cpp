/**
 * Minimal quantized operator registration for ExecuTorch.
 * Registers only quantize_per_tensor and dequantize_per_tensor operators
 * needed by the simple Add model delegated to Ethos-U.
 */

#include <executorch/runtime/kernel/operator_registry.h>
#include <executorch/runtime/core/exec_aten/exec_aten.h>

namespace torch {
namespace executor {
namespace native {

// Forward declarations of the kernel functions from libquantized_kernels.a
Tensor& quantize_per_tensor_out(
    KernelRuntimeContext& context,
    const Tensor& input,
    double scale,
    int64_t zero_point,
    int64_t quant_min,
    int64_t quant_max,
    ScalarType dtype,
    Tensor& out);

Tensor& dequantize_per_tensor_out(
    KernelRuntimeContext& context,
    const Tensor& input,
    double scale,
    int64_t zero_point,
    int64_t quant_min,
    int64_t quant_max,
    ScalarType dtype,
    std::optional<ScalarType> out_dtype,
    Tensor& out);

} // namespace native
} // namespace executor
} // namespace torch

namespace {

using ::executorch::runtime::EValue;
using ::executorch::runtime::Error;
using ::executorch::runtime::Kernel;
using ::executorch::runtime::KernelRuntimeContext;
using ::executorch::runtime::Span;
using ::executorch::aten::ScalarType;

// Wrapper for quantize_per_tensor that unpacks EValues
void quantize_per_tensor_wrapper(
    KernelRuntimeContext& ctx,
    Span<EValue*> args) {
    // args[0]: input tensor
    // args[1]: scale (double)
    // args[2]: zero_point (int64)
    // args[3]: quant_min (int64)
    // args[4]: quant_max (int64)
    // args[5]: dtype (ScalarType)
    // args[6]: output tensor

    ::torch::executor::native::quantize_per_tensor_out(
        ctx,
        args[0]->toTensor(),
        args[1]->toDouble(),
        args[2]->toInt(),
        args[3]->toInt(),
        args[4]->toInt(),
        static_cast<ScalarType>(args[5]->toInt()),
        args[6]->toTensor());
}

// Wrapper for dequantize_per_tensor that unpacks EValues
void dequantize_per_tensor_wrapper(
    KernelRuntimeContext& ctx,
    Span<EValue*> args) {
    // args[0]: input tensor
    // args[1]: scale (double)
    // args[2]: zero_point (int64)
    // args[3]: quant_min (int64)
    // args[4]: quant_max (int64)
    // args[5]: dtype (ScalarType)
    // args[6]: out_dtype (optional ScalarType)
    // args[7]: output tensor

    std::optional<ScalarType> out_dtype;
    if (!args[6]->isNone()) {
        out_dtype = static_cast<ScalarType>(args[6]->toInt());
    }

    ::torch::executor::native::dequantize_per_tensor_out(
        ctx,
        args[0]->toTensor(),
        args[1]->toDouble(),
        args[2]->toInt(),
        args[3]->toInt(),
        args[4]->toInt(),
        static_cast<ScalarType>(args[5]->toInt()),
        out_dtype,
        args[7]->toTensor());
}

// Kernel registration array
static Kernel kernels[] = {
    Kernel(
        "quantized_decomposed::quantize_per_tensor.out",
        quantize_per_tensor_wrapper),
    Kernel(
        "quantized_decomposed::dequantize_per_tensor.out",
        dequantize_per_tensor_wrapper),
};

// Static registration via constructor
struct RegisterQuantizedOps {
    RegisterQuantizedOps() {
        (void)::executorch::runtime::register_kernels(
            Span<const Kernel>(kernels, sizeof(kernels) / sizeof(kernels[0])));
    }
};

static RegisterQuantizedOps register_quantized_ops;

} // anonymous namespace
