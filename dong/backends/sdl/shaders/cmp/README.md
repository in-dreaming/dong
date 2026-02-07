# GPU Texture Compression Shaders

This directory contains compute shaders for GPU-accelerated texture compression.

## Formats Supported

- **BC7** - High quality RGBA compression, 4x4 blocks, 16 bytes/block
- **ASTC** - Adaptive Scalable Texture Compression, variable block sizes

## Files

- `bc7_common.hlsl` - BC7 common utilities and constants
- `bc7_encode_cs.hlsl` - BC7 compute shader encoder
- `astc_common.hlsl` - ASTC common utilities
- `astc_encode_cs.hlsl` - ASTC compute shader encoder

## References

Implementation based on:
- AMD Compressonator (BC7)
- Unity ASTC GPU Encoder (ASTC)

## Usage

These shaders are loaded by `GPUTextureCompressor` class in the SDL backend.
