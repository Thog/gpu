/*
 * Copyright (C) 2021 Alyssa Rosenzweig <alyssa@rosenzweig.io>
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

#include <assert.h>
#include <IOKit/IOKitLib.h>
#include "io.h"
#include "selectors.h"

struct agx_allocation
agx_alloc_mem(mach_port_t connection, size_t size)
{
	uint32_t mode = 0x430; // shared, ?
	uint32_t cache = 0x400;// 0x400 for write-combine, 0x0 default

	uint32_t args_in[24] = { 0 };
	args_in[1] = cache;
	args_in[2] = 0x18000; // unk
	args_in[3] = 0x1; // unk;
	args_in[4] = 0x1000101; // unk
	args_in[5] = mode;
	args_in[16] = size;
	args_in[20] = 0x18000000;// unk

	uint64_t out[10] = { 0 };
	size_t out_sz = sizeof(out);

	kern_return_t ret = IOConnectCallMethod(connection,
			AGX_SELECTOR_ALLOCATE_MEM, NULL, 0, args_in,
			sizeof(args_in), NULL, 0, out, &out_sz);

	assert(ret == 0);
	assert(out_sz == sizeof(out));

	return (struct agx_allocation) {
		.type = AGX_ALLOC_REGULAR,
		.index = (out[3] >> 32ull),
		.gpu_va = out[0],
		.map = (void *) out[1],
		.size = size
	};
}

struct agx_allocation
agx_alloc_cmdbuf(mach_port_t connection, size_t size, bool cmdbuf)
{
	struct agx_create_cmdbuf_resp out = {};
	size_t out_sz = sizeof(out);

	uint64_t inputs[2] = {
		size,
		cmdbuf ? 1 : 0
	};

	kern_return_t ret = IOConnectCallMethod(connection,
			AGX_SELECTOR_CREATE_CMDBUF, inputs, 2, NULL, 0, NULL,
			NULL, &out, &out_sz);

	assert(ret == 0);
	assert(out_sz == sizeof(out));
	assert(out.size == size);

	return (struct agx_allocation) {
		.type = cmdbuf ? AGX_ALLOC_CMDBUF : AGX_ALLOC_MEMMAP,
		.index = out.id,
		.map = out.map,
		.size = out.size
	};
}
