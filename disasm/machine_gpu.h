/*
 * Copyright (C) 2021 Mary <me@thog.eu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// Extended informations for machine.h used by Metal's object files.
// NOTE: This was retrieved via air-lipo -info output.
// TODO: define other vendors GPU subtypes?

#pragma once
#include <mach/machine.h>

// CPU types
#define CPU_TYPE_APPLE_GPU  (0x13 | CPU_ARCH_ABI64)
#define CPU_TYPE_AMD_GPU    (0x14 | CPU_ARCH_ABI64)
#define CPU_TYPE_INTEL_GPU  (0x15 | CPU_ARCH_ABI64)
#define CPU_TYPE_NVIDIA_GPU (0x16 | CPU_ARCH_ABI64)
#define CPU_TYPE_AIR        (0x17 | CPU_ARCH_ABI64)

#define CPU_SUBTYPE_AGX(f, m) ((cpu_subtype_t) (f) + ((m) << 4))

// AGX0
#define CPU_SUBTYPE_APPLE_S3    CPU_SUBTYPE_AGX(1, 2)

// AGX1
#define CPU_SUBTYPE_APPLE_GPU_A8   CPU_SUBTYPE_AGX(1, 1)
#define CPU_SUBTYPE_APPLE_GPU_A8X  CPU_SUBTYPE_AGX(1, 3)
#define CPU_SUBTYPE_APPLE_GPU_A9   CPU_SUBTYPE_AGX(1, 4)
#define CPU_SUBTYPE_APPLE_GPU_A10  CPU_SUBTYPE_AGX(1, 5)
#define CPU_SUBTYPE_APPLE_GPU_A10X CPU_SUBTYPE_AGX(1, 6)

// AGX2
#define CPU_SUBTYPE_APPLE_GPU_A11  CPU_SUBTYPE_AGX(2, 2)
#define CPU_SUBTYPE_APPLE_GPU_S4   CPU_SUBTYPE_AGX(2, 5)
#define CPU_SUBTYPE_APPLE_GPU_A12  CPU_SUBTYPE_AGX(2, 7)
#define CPU_SUBTYPE_APPLE_GPU_A12X CPU_SUBTYPE_AGX(2, 8)
#define CPU_SUBTYPE_APPLE_GPU_A13  CPU_SUBTYPE_AGX(2, 13)
#define CPU_SUBTYPE_APPLE_GPU_A14  CPU_SUBTYPE_AGX(2, 18)
#define CPU_SUBTYPE_APPLE_GPU_A14X CPU_SUBTYPE_AGX(2, 20)
#define CPU_SUBTYPE_APPLE_GPU_A12Z CPU_SUBTYPE_AGX(2, 100)
