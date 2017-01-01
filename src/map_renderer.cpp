#include "map_renderer.hpp"

#include "common.hpp"
#include "common_alsmart.hpp"
#include "draw_buffer.hpp"
#include "engine.hpp"

Map_Renderer::Map_Renderer(Engine& engine)
	: m_engine(engine)
{ }

Map_Renderer::~Map_Renderer()
{ }

void Map_Renderer::resize(int w, int h)
{
	m_view_w = w;
	m_view_h = h;
}

void Map_Renderer::draw(Draw_Buffer& draw, int x, int y)
{
	static const ALLEGRO_COLOR black = al_map_rgba_f(0.f, 0.f, 0.f, 1.f);

	//draw.clear(black);
	//draw.clear_depth(1.f);

	if (!m_engine.has_emf())
		return;

	auto&& gfx = m_engine.gfx();
	auto&& emf = m_engine.emf();
	auto&& emfh = emf.header;

	int draw_x = x;
	int draw_y = y;

	constexpr float ep  = 0.0001f;  // gap between depth of each tile on a layer
	constexpr float epi = 0.00001f; // gap between each interleaved layer

	struct LayerInfo
	{
		int file;
		int xoff;
		int yoff;
		float depth;
	};

	static constexpr LayerInfo layer_info[9] = {
		{ 3,    0,   0, 1.0f - epi*0 }, // Ground
		{ 4,    0,   0, 0.8f - epi*0 }, // Objects
		{ 5,    0,   0, 0.7f - epi*0 }, // Overlay
		{ 6,    0,   0, 0.8f - epi*1 }, // Down Wall
		{ 6,   32,   0, 0.8f - epi*2 }, // Right Wall
		{ 7,   32, -64, 0.7f - epi*1 }, // Roof
		{ 3,   32, -64, 0.7f - epi*2 }, // Top
		{ 22, -54, -42, 0.9f - epi*0 }, // Shadow
		{ 5,    0,   0, 0.7f - epi*3 }, // Overlay 2
	};

	float depth = layer_info[0].depth;

	auto next_depth = [&depth, ep]() { return depth -= ep; };

	auto base_gfx = gfx.get_image(3, emfh.fill_tile);

	// Separate loop so base tile graphics can be substituted
	for (int i = 0; i < emfh.width + emfh.height; ++i)
	{
		int x, y;

		if (i < emfh.height)
		{
			x = 0;
			y = i;
		}
		else
		{
			x = i - emfh.height;
			y = emfh.height;
		}

		for (next_depth(); y >= 0 && x <= emfh.width; --y, ++x, next_depth())
		{
			int xoff = layer_info[0].xoff - m_offset_x;
			int yoff = layer_info[0].yoff - m_offset_y;
			short tile = emf.gfx(x, y)[0];

			int tilex = xoff + (x * 32) - (y * 32);
			int tiley = yoff + (x * 16) + (y * 16);

			if (tilex < -32 || tiley < -16
			 || tilex > m_view_w || tiley > m_view_h)
				continue;

			if (tile)
			{
				auto tile_gfx = gfx.get_image(layer_info[0].file, tile);

				if (tile_gfx)
				{
					draw.draw(tile_gfx, tilex, tiley, depth, 0);
				}
				else
				{
					cio::out << "Could not load tile " << 3 << '/' << tile << cio::endl;
				}
			}
			else
			{
				draw.draw(base_gfx, tilex, tiley, depth, 0);
			}
		}
	}

	for (int layer = 1; layer < 7; ++layer)
	{
		depth = layer_info[layer].depth;

		for (int i = 0; i < emfh.width + emfh.height; ++i)
		{
			int x, y;

			if (i < emfh.height)
			{
				x = 0;
				y = i;
			}
			else
			{
				x = i - emfh.height;
				y = emfh.height;
			}

			for (next_depth(); y >= 0 && x <= emfh.width; --y, ++x, next_depth())
			{
				int xoff = layer_info[layer].xoff - m_offset_x;
				int yoff = layer_info[layer].yoff - m_offset_y;
				short tile = emf.gfx(x, y)[layer];

				int tilex = xoff + (x * 32) - (y * 32);
				int tiley = yoff + (x * 16) + (y * 16);

				if (tilex < -240 || tiley < -320
				 || tilex > m_view_w + 240 || tiley > m_view_h + 320)
					continue;

				if (tile)
				{
					auto tile_gfx = gfx.get_image(layer_info[layer].file, tile);

					if (tile_gfx)
					{
						int tile_w = al_get_bitmap_width(tile_gfx.get());
						int tile_h = al_get_bitmap_height(tile_gfx.get());
						tilex -= tile_w / (1 + (layer == 1)) - 32;
						tiley -= tile_h - 32;
						draw.draw(tile_gfx, tilex, tiley, depth, 0);
					}
					else
					{

						//cio::out << "Could not load tile " << layer_info[layer].file << '/' << tile << cio::endl;
					}
				}
			}
		}
	}

	draw.sync();
}
