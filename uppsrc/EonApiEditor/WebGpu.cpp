#include "EonApiEditor.h"

NAMESPACE_UPP

void InterfaceBuilder::AddWebGpu() {
    Package("WebGpu", "Wg");
    SetColor(0, 100, 200);
    Dependency("ParallelLib");
    Dependency("ports/webgpu", "BUILTIN_WEBGPU");
    Library("webgpu", "WEBGPU");
    
    Interface("GpuAdapter");
    Interface("GpuDevice");
    Interface("GpuQueue");
    Interface("GpuCommandEncoder");
    Interface("GpuRenderPassEncoder");
    Interface("GpuComputePassEncoder");
    Interface("GpuBindGroup");
    Interface("GpuBindGroupLayout");
    Interface("GpuPipelineLayout");
    Interface("GpuRenderPipeline");
    Interface("GpuComputePipeline");
    Interface("GpuShaderModule");
    Interface("GpuBuffer");
    Interface("GpuTexture");
    Interface("GpuTextureView");
    Interface("GpuSampler");
    Interface("GpuQuerySet");
    
    Vendor("WebGpuNative", "BUILTIN_WEBGPU|WEBGPU_NATIVE");
    Vendor("Dawn", "DAWN");
    Vendor("WGPU", "WGPU");
}

END_UPP_NAMESPACE