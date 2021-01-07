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

#include <unistd.h>
#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <mach-o/loader.h>
#include <mach-o/fat.h>

#include "machine_gpu.h"

void
agx_disassemble(void *_code, size_t maxlen, FILE *fp);

uint32_t swap_uint32(uint32_t value)
{
	value = ((value << 8) & 0xFF00FF00) | ((value >> 8) & 0xFF00FF); 
	return (value << 16) | (value >> 16);
}

uint64_t swap_uint64(uint64_t value)
{
	value = ((value << 8) & 0xFF00FF00FF00FF00ULL) | ((value >> 8) & 0x00FF00FF00FF00FFULL);
	value = ((value << 16) & 0xFFFF0000FFFF0000ULL) | ((value >> 16) & 0x0000FFFF0000FFFFULL);
	return (value << 32) | (value >> 32);
}

typedef struct
{
	void *address;
	size_t size;
} section_info_t;

static int get_mach_text_section(section_info_t *text_section_info, char *segment_name, void *address, size_t size)
{
	struct mach_header_64* header = (struct mach_header_64*)address;

	if(header->magic == MH_MAGIC_64)
	{
		struct load_command *commands = (struct load_command*)&header[1];

		struct load_command *command = commands;

		for(int command_index = 0; command_index < header->ncmds; command_index++)
		{
			if (command->cmdsize == 0 || (uintptr_t)command + command->cmdsize - (uintptr_t)header > size)
			{
				return 2;
			}

			if (command->cmd == LC_SEGMENT_64)
			{
				struct segment_command_64* segment = (struct segment_command_64*)command;

				if(strncmp(segment->segname, segment_name, 16) == 0)
				{
					struct section_64* sections = (struct section_64*)&segment[1];
					for(int j = 0; j < segment->nsects; j++)
					{
						if(strncmp(sections[j].sectname, "__text", 16) == 0)
						{
							text_section_info->address = address + sections[j].offset;
							text_section_info->size = sections[j].size;
							return 0;
						}
					}
				}
			}

			command = (struct load_command*)((char*)command + command->cmdsize);
		}

		fprintf(stderr, "text section not found!\n");
		return 3;
	} else
	{
		fprintf(stderr, "Invalid file magic %08X\n", header->magic);
		return 1;
	}
}

static int get_agx_code_info(section_info_t *agx_text_section_info, void *file, size_t file_size)
{
	section_info_t text_section;
	bzero(&text_section, sizeof(section_info_t));

	uint32_t magic = *((uint32_t*)file);

	void *start = NULL;
	size_t size = 0;

	if (magic == FAT_CIGAM_64)
	{
		struct fat_header *header = (struct fat_header*)file;
		struct fat_arch_64 *archs = (struct fat_arch_64*)&header[1];

		for (int arch_index = 0; arch_index < swap_uint32(header->nfat_arch); arch_index++)
		{
			fprintf(stderr, "0x%x, 0x%x\n", swap_uint32(archs[arch_index].cputype), CPU_TYPE_APPLE_GPU);
			fprintf(stderr, "0x%x, 0x%x\n", swap_uint32(archs[arch_index].cpusubtype), CPU_SUBTYPE_APPLE_GPU_A14X);
			fprintf(stderr, "0x%llx, 0x%x\n", swap_uint64(archs[arch_index].offset), CPU_SUBTYPE_APPLE_GPU_A14X);
			fprintf(stderr, "0x%llx, 0x%x\n", swap_uint64(archs[arch_index].size), CPU_SUBTYPE_APPLE_GPU_A14X);

			// Match on M1 GPU.
			if (swap_uint32(archs[arch_index].cputype) == CPU_TYPE_APPLE_GPU && swap_uint32(archs[arch_index].cpusubtype) == CPU_SUBTYPE_APPLE_GPU_A14X)
			{
				start = file + swap_uint64(archs[arch_index].offset);
				size = swap_uint64(archs[arch_index].size);

				break;
			}
		}

		if (start == NULL)
		{
			fprintf(stderr, "Cannot find embedded macho inside metalib file!\n");
			return 1;
		}
	}
	else if (magic == MH_MAGIC_64)
	{
		start = file;
		size = file_size;
	}
	else
	{
		fprintf(stderr, "Invalid file magic %08X\n", magic);
		return 1;
	}

	int ret = get_mach_text_section(&text_section, "__TEXT", start, size);

	if (ret == 0)
	{
		return get_mach_text_section(agx_text_section_info, "", text_section.address, text_section.size);
	}

	return ret;
}

int main(int argc, char **argv)
{
	if(argc != 2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		return 1;
	}
	
	int fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		perror(argv[1]);
		return 1;
	}
	
	size_t file_size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	
	void* map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if(map == MAP_FAILED) {
		perror("mmap");
		close(fd);
		return 1;
	}

	section_info_t info;
	bzero(&info, sizeof(section_info_t));

	int ret = get_agx_code_info(&info, map, file_size);

	if (ret == 0)
	{
		agx_disassemble(info.address, info.size, stderr);
	}

	munmap(map, file_size);
	close(fd);
	
	return 0;
}