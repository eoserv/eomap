#include "gfx_manager.hpp"

#include "dib_reader.hpp"
#include "engine.hpp"
#include "pe_reader.hpp"

#include "util/cio/cio.hpp"
#include "util/int_pack.hpp"

GFX_Manager::GFX_Manager(Engine& engine)
	: m_engine(engine)
{ }

GFX_Manager::~GFX_Manager()
{ }

bool GFX_Manager::load_file(int fileid, const char* filename)
{
	if (m_gfx_atlasses.count(fileid) != 0)
	{
		m_gfx_atlasses.clear();
	}

	cio::stream file(filename, cio::stream::mode_read);

	if (!file)
		return false;

	pe_reader pe(std::move(file));

	if (!pe.read_header())
		return false;

	auto table = pe.read_bitmap_table();

	// TODO: Get the max texture size from the display driver
	auto result = m_gfx_atlasses.emplace(pair<int, Atlas>{fileid, Atlas(2048, 2048)});
	auto&& atlas = result.first->second;

	for (const auto& x : table)
	{
		auto&& imageid = x.first;
		auto&& info = x.second;

		u_ptr<char[]> buf(new char[info.size]);
		pe.read_resource(buf.get(), info.start, info.size);

		auto&& bmp = atlas.add(imageid, info.width, info.height);

		dib_reader dib(buf.get(), info.size);

		dib.start();

		auto check_result = dib.check_format();

		if (check_result != nullptr)
		{
			cio::err << "Bad DIB format: " << check_result << cio::endl;
			return false;
		}

		auto lock = al_lock_bitmap(bmp.get(), ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE,
		                           ALLEGRO_LOCK_WRITEONLY);

		for (int i = 0; i < info.height; ++i)
		{
			char* row = static_cast<char*>(lock->data) + lock->pitch * i;
			dib.read_line(row, i);
		}

		al_unlock_bitmap(bmp.get());
	}

	atlas.upload(m_engine);

	return true;
}

s_bmp GFX_Manager::get_image(int fileid, int imageid)
{
	imageid += 100;

	auto atlas_it = m_gfx_atlasses.find(fileid);

	if (atlas_it == m_gfx_atlasses.end())
		return nullptr;

	auto& atlas = atlas_it->second;

	return atlas.get(imageid);
}
