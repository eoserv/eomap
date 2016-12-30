#ifndef EOMAP_PE_READER_HPP
#define EOMAP_PE_READER_HPP

#include "cio/cio.hpp"
#include "common.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>

class pe_reader
{
	public:
		struct BitmapInfo
		{
			size_t start;
			size_t size;
			int width;
			int height;
		};

	private:
		enum class ResourceType : uint32_t
		{
			Cursor = 1,
			Bitmap = 2,
			Icon = 3,
			Menu = 4,
			Dialog = 5,
			StringTable = 6,
			FontDirectory = 7,
			Font = 8,
			Accelerator = 9,
			Unformatted = 10,
			MessageTable = 11,
			GroupCursor = 12,
			GroupIcon = 14,
			VersionInformation = 16
		};

		struct ResourceDirectory
		{
			uint32_t Characteristics;
			uint32_t TimeDateStamp;
			uint16_t MajorVersion;
			uint16_t MinorVersion;
			uint16_t NumberOfNamedEntries;
			uint16_t NumberOfIdEntries;
		};

		struct ResourceDirectoryEntry
		{
			ResourceType ResourceType_;
			uint32_t SubDirectoryOffset;
		};

		struct ResourceDataEntry
		{
			uint32_t OffsetToData;
			uint32_t Size;
			uint32_t CodePage;
			uint32_t unused;
		};

		cio::stream file;
		
		uint32_t root_address = 0;
		uint32_t virtual_address = 0;
		ResourceDirectoryEntry bitmap_directory_entry = {ResourceType{}, 0};

		uint16_t read_u16_le();
		uint32_t read_u32_le();

		ResourceDirectory read_ResourceDirectory();
		ResourceDirectoryEntry read_ResourceDirectoryEntry();
		ResourceDataEntry read_ResourceDataEntry();

	public:
		pe_reader(cio::stream&& file)
			: file(std::move(file))
		{ }

		bool read_header();

		std::map<int, BitmapInfo> read_bitmap_table();

		bool read_resource(char* buf, size_t start, size_t size);

		cio::stream& get_file()
		{
			return file;
		}

		cio::stream&& finish()
		{
			return std::move(file);
		}
};

#endif // EOMAP_PE_READER_HPP
