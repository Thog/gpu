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

// clang metal_compiler.m -fobjc-arc -fmodules -mmacosx-version-min=11.0 -framework Foundation -framework CoreGraphics -o metal_compiler
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

void compile_to_gpu_code(id<MTLDevice> device, NSString *programString, NSString *destinationPath) {
	NSError *error;

	MTLCompileOptions *options = [MTLCompileOptions new];
	options.libraryType = MTLLibraryTypeDynamic;
	options.installName = [NSString stringWithFormat:@"@executable_path/userCreatedDylib.metallib"];

	id<MTLLibrary> lib = [device newLibraryWithSource:programString
											   options:options
												 error:&error];

	if(!lib && error)
	{
		NSLog(@"Error compiling library from source: %@", error);
		return;
	}
	
	id<MTLDynamicLibrary> dynamicLib = [device newDynamicLibrary:lib
															error:&error];
	if(!dynamicLib && error)
	{
		NSLog(@"Error creating dynamic library from source library: %@", error);
		return;
	}

	NSURL *destinationURL = [NSURL URLWithString:destinationPath];

	bool result = [dynamicLib serializeToURL:destinationURL error:&error];

	if (!result && error)
	{
		NSLog(@"Error serializing dynamic library: %@", error);
		return;
	}

	NSLog(@"Compilation done.");
}

int main(int argc, const char * argv[]) {
	@autoreleasepool {
		NSError *error;
		
		id<MTLDevice> device = MTLCreateSystemDefaultDevice();

		NSString *sourcePath = [NSString stringWithUTF8String:argv[1]];
		NSString *destinationPath = [NSString stringWithUTF8String:argv[2]];

		NSString *source = [NSString stringWithContentsOfFile:sourcePath encoding:NSUTF8StringEncoding error:&error];

		compile_to_gpu_code(device, source, destinationPath);
	}
	return 0;
}
