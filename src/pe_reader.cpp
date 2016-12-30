
#include "pe_reader.hpp"

#include "dib_reader.hpp"
#include "int_pack.hpp"

#include <cstring>
#include <vector>

uint16_t pe_reader::read_u16_le()
{
	char buf[2];

	if (file.read(buf, 2) == 2)
	{
		return int_pack_16_le(buf[0], buf[1]);
	}

	return 0;
}

uint32_t pe_reader::read_u32_le()
{
	char buf[4];

	if (file.read(buf, 4) == 4)
	{
		return int_pack_32_le(buf[0], buf[1], buf[2], buf[3]);
	}

	return 0;
}

pe_reader::ResourceDirectory pe_reader::read_ResourceDirectory()
{
	return pe_reader::ResourceDirectory{
		read_u32_le(),
		read_u32_le(),
		read_u16_le(),
		read_u16_le(),
		read_u16_le(),
		read_u16_le()
	};
}

pe_reader::ResourceDirectoryEntry pe_reader::read_ResourceDirectoryEntry()
{
	return pe_reader::ResourceDirectoryEntry{
		pe_reader::ResourceType(read_u32_le()),
		read_u32_le()
	};
}

pe_reader::ResourceDataEntry pe_reader::read_ResourceDataEntry()
{
	return pe_reader::ResourceDataEntry{
		read_u32_le(),
		read_u32_le(),
		read_u32_le(),
		read_u32_le()
	};
}

bool pe_reader::read_header()
{
	char buf[8];

	file.seek(0x3C);
	uint16_t pe_header_address = read_u16_le();

	file.skip(pe_header_address - 0x3C - 0x02);
	file.read(buf, 4);

	if (std::memcmp(buf, "PE\0", 4) != 0)
		return false;

	file.skip(0x02);
	uint16_t sections = read_u16_le();

	file.skip(0x78 - 0x04 + 0x0C);
	virtual_address = read_u32_le();

	file.skip(0x6C + 0x08 + 0x04);

	for (unsigned int i = 0; i < sections; ++i)
	{
		uint32_t check_virtual_address = read_u32_le();

		if (check_virtual_address == virtual_address)
		{
			file.skip(0x04);

			root_address = read_u32_le();
			break;
		}

		file.skip(0x24);
	}

	if (!root_address)
		return false;

	file.seek(root_address);

	ResourceDirectory root_directory = read_ResourceDirectory();

	unsigned int directory_entries = root_directory.NumberOfNamedEntries + root_directory.NumberOfIdEntries;

	ResourceDirectoryEntry entry;

	for (unsigned int i = 0; i < directory_entries; ++i)
	{
		entry = read_ResourceDirectoryEntry();

		if (uint32_t(entry.ResourceType_) < 0x80000000 && entry.ResourceType_ == ResourceType::Bitmap)
		{
			if (entry.SubDirectoryOffset < 0x80000000)
				return false;

			entry.SubDirectoryOffset -= 0x80000000;

			bitmap_directory_entry = entry;

			file.skip(8 * (directory_entries - i - 1));

			break;
		}
	}

	return true;
}

std::map<int, pe_reader::BitmapInfo> pe_reader::read_bitmap_table()
{
	char buf[16];

	std::map<int, BitmapInfo> bitmap_pointers;

	if (bitmap_directory_entry.ResourceType_ != ResourceType::Bitmap)
		return bitmap_pointers;

	file.seek(root_address + bitmap_directory_entry.SubDirectoryOffset);

	ResourceDirectory bitmap_directory;
	file.read(buf, 16);
	bitmap_directory.NumberOfNamedEntries = int_pack_16_le(buf + 12);
	bitmap_directory.NumberOfIdEntries = int_pack_16_le(buf + 14);

	unsigned int directory_entries = bitmap_directory.NumberOfNamedEntries + bitmap_directory.NumberOfIdEntries;

	std::vector<ResourceDirectoryEntry> bitmap_entries;
	bitmap_entries.reserve(directory_entries);

	ResourceDirectoryEntry entry;

	for (unsigned int i = 0; i < directory_entries; ++i)
	{
		file.read(buf, 8);
		entry.ResourceType_ = ResourceType(int_pack_32_le(buf));
		entry.SubDirectoryOffset = int_pack_32_le(buf + 4);

		if (entry.SubDirectoryOffset > 0x80000000)
		{
			entry.SubDirectoryOffset -= 0x80000000;
			bitmap_entries.push_back(entry);
		}
	}

	ResourceDataEntry data_entry;

	for (auto it = bitmap_entries.begin(); it != bitmap_entries.end(); ++it)
	{
		file.seek(root_address + it->SubDirectoryOffset + 16);

		file.read(buf, 8);
		entry.SubDirectoryOffset = int_pack_32_le(buf + 4);

		file.seek(root_address + entry.SubDirectoryOffset);

		file.read(buf, 16);
		data_entry.OffsetToData = int_pack_32_le(buf);
		data_entry.Size = int_pack_32_le(buf + 4);

		size_t start = data_entry.OffsetToData - virtual_address + root_address;
		size_t size = data_entry.Size;

		file.seek(start + 4);

		int width = read_u32_le();
		int height = read_u32_le();

		size_t type = uint32_t(it->ResourceType_);

		bitmap_pointers.insert({type, {start, size, width, height}});
	}

	return bitmap_pointers;
}

bool pe_reader::read_resource(char* buf, size_t start, size_t size)
{
	if (!file.seek(start))
		return false;

	if (file.read(buf, size) != size)
		return false;

	return true;
}
