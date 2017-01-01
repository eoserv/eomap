#include "palette_renderer.hpp"

#include "atlas.hpp"
#include "common.hpp"
#include "common_alsmart.hpp"
#include "draw_buffer.hpp"
#include "engine.hpp"
#include "gfx_manager.hpp"

void Palette_Renderer::calculate_layout(int fileid)
{
	bool dirty = true;
	auto&& dirty_it = m_tile_layouts_dirty.find(fileid);

	if (dirty_it != m_tile_layouts_dirty.end())
		dirty = dirty_it->second;

	if (!dirty)
		return;

	// Copy-pasted algorithm from Atlas::add
	static constexpr int x_res = 8;
	static constexpr int y_res = 8;

	static constexpr int page_width = (320 + x_res - 1) / x_res;

	auto&& gfx = m_engine.gfx();

	auto&& tiles = m_layer_tiles.find(fileid)->second;

	std::vector<int> col_heights;
	col_heights.resize((320 + x_res - 1) / x_res);

	auto&& layout = m_tile_layouts.insert({fileid, {}}).first->second;

	for (auto&& tileid : tiles)
	{
		auto tile_gfx = gfx.get_image(fileid, tileid);

		if (!tile_gfx)
		{
			//cio::err << fileid << '/' << tileid << " is missing" << cio::endl;
			continue;
		}

		int width = 64;
		int height = 32;

		if (fileid != 3)
		{
			width = al_get_bitmap_width(tile_gfx.get());
			height = al_get_bitmap_height(tile_gfx.get());
		}

		int block_width = (width + x_res - 1) / x_res;
		int block_height = (height + y_res - 1) / y_res;

		if (block_width > page_width)
			block_width = page_width;

		int start_y = std::accumulate(col_heights.begin(), col_heights.end(),
		                              0x7FFFFFFF, [](int x, int y) { return std::min(x, y); });

		for (int y = start_y; ; ++y)
		{
			for (int x = 0; x <= page_width - block_width; ++x)
			{
				int max_y = std::accumulate(col_heights.begin() + x,
				                            col_heights.begin() + x + block_width,
				                            0, [](int x, int y) { return std::max(x, y); });

				if (max_y < y)
				{
					for (auto it  = col_heights.begin() + x;
					          it != col_heights.begin() + x + block_width;
					        ++it)
					{
						*it = max_y + block_height;
					}

					layout.insert({tileid, {x * x_res, y * y_res}});
					goto next_tile;
				}
			}
		}

		next_tile:
			;
	}

	m_tile_layouts_dirty.emplace(fileid, false);
}

void Palette_Renderer::calculate_layouts()
{
	for (auto&& layer : m_layer_tiles)
	{
		calculate_layout(layer.first);
	}
}

Palette_Renderer::Palette_Renderer(Engine& engine)
	: m_engine(engine)
{
	static constexpr int map_files[] = {3, 4, 5, 6, 7, 22};

	for (int fileid : map_files)
	{
		auto&& atlas = m_engine.gfx().get_atlas(fileid);

		if (atlas)
		{
			auto&& atlas_lock = atlas.value().get();
			m_layer_tiles.emplace(fileid, atlas_lock.get_entry_list());
		}
	}

	calculate_layouts();
}

Palette_Renderer::~Palette_Renderer()
{ }

void Palette_Renderer::resize(int w, int h)
{
	m_view_w = w;
	m_view_h = h;
}

void Palette_Renderer::draw(Draw_Buffer& draw, int x, int y)
{
	auto&& gfx = m_engine.gfx();

	int draw_x = x;
	int draw_y = y;

	auto&& layout = m_tile_layouts.find(m_layer)->second;

	for (auto&& tile : layout)
	{
		auto&& tileid = tile.first;
		auto&& x = tile.second.first;
		auto&& y = tile.second.second;

		auto tile_gfx = gfx.get_image(m_layer, tileid);

		if (tile_gfx)
			draw.draw(tile_gfx, draw_x + x, draw_y + y, 0.f, 0);
	}
}
