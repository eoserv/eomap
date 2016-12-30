#include "map_renderer.hpp"

#include "common.hpp"
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
	static const ALLEGRO_COLOR black = al_map_rgb_f(0.f, 0.f, 0.f);
	draw.clear(black);

	auto&& gfx = m_engine.gfx();
	auto&& emf = m_engine.emf();
	auto&& emfh = emf.header;

	static constexpr int file_map[9] = { 3, 4, 5, 6, 6, 7, 3, 22, 5 };
	static constexpr int xoff_map[9] = { 0, 0, 0, 0, 32, 32, 32, -54, 0 };
	static constexpr int yoff_map[9] = { 0, 0, 0, 0, 0, -64, -64, -42, 0 };

	auto base_gfx = gfx.get_image(3, emfh.fill_tile);

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

		for (; y >= 0 && x <= emfh.width; --y, ++x)
		{
			int xoff = xoff_map[0] - m_offset_x;
			int yoff = yoff_map[0] - m_offset_y;
			short tile = emf.gfx(x, y)[0];

			int tilex = xoff + (x * 32) - (y * 32);
			int tiley = yoff + (x * 16) + (y * 16);

			if (tilex < -32 || tiley < -16
			 || tilex > m_view_w || tiley > m_view_h)
				continue;

			if (tile)
			{
				auto tile_gfx = gfx.get_image(file_map[0], tile);

				if (tile_gfx)
				{
					draw.draw(tile_gfx, tilex, tiley, 1.f, 0);
				}
				else
				{
					cio::out << "Could not load tile " << 3 << '/' << tile << cio::endl;
				}
			}
			else
			{
				draw.draw(base_gfx, tilex, tiley, 1.f, 0);
			}
		}
	}

	for (int layer = 1; layer < 7; ++layer)
	{
		if (layer == 2)
			continue;

		draw.sync();

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

			for (; y >= 0 && x <= emfh.width; --y, ++x)
			{
				int xoff = xoff_map[layer] - m_offset_x;
				int yoff = yoff_map[layer] - m_offset_y;
				short tile = emf.gfx(x, y)[layer];

				int tilex = xoff + (x * 32) - (y * 32);
				int tiley = yoff + (x * 16) + (y * 16);

				if (tilex < -320 || tiley < -160
				 || tilex > m_view_w || tiley > m_view_h)
					continue;

				if (tile)
				{
					auto tile_gfx = gfx.get_image(file_map[layer], tile);

					if (tile_gfx)
					{
						int tile_w = al_get_bitmap_width(tile_gfx);
						int tile_h = al_get_bitmap_height(tile_gfx);
						tilex -= tile_w / (1 + (layer == 1)) - 32;
						tiley -= tile_h - 32;
						draw.draw(tile_gfx, tilex, tiley, -1.f, 0);
					}
					else
					{
						cio::out << "Could not load tile " << file_map[layer] << '/' << tile << cio::endl;
					}
				}
			}
		}
	}
}
