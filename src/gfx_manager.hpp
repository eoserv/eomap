#ifndef EOMAP_GFX_MANAGER_HPP
#define EOMAP_GFX_MANAGER_HPP

#include "common.hpp"
#include "common_alsmart.hpp"

#include "atlas.hpp"
#include "pe_reader.hpp"

#include <array>
#include <cstdint>
#include <future>
#include <map>
#include <mutex>
#include <thread>

class Engine;

class GFX_Manager
{
	public:
		static constexpr size_t num_image_tables = 25;

	private:
		Engine& m_engine;

		std::map<int, Atlas> gfx_atlasses;

	public:
		GFX_Manager(Engine& engine);
		~GFX_Manager();

		bool load_file(int fileid, const char* filename);

		Atlas& get_atlas(int fileid)
		{
			return gfx_atlasses.find(fileid)->second;
		}

		ALLEGRO_BITMAP* get_image(int fileid, int imageid);
};

#endif // EOMAP_GFX_MANAGER_HPP
