#ifndef EOMAP_PALETTE_RENDERER_HPP
#define EOMAP_PALETTE_RENDERER_HPP

#include <map>
#include <vector>

class Draw_Buffer;
class Engine;

class Palette_Renderer
{
	private:
		Engine& m_engine;

		int m_view_w = 0;
		int m_view_h = 0;

		std::map<int, std::vector<int>> m_layer_tiles;
		std::map<int, std::map<int, std::pair<int, int>>> m_tile_layouts;
		std::map<int, bool> m_tile_layouts_dirty;

		void calculate_layout(int fileid);
		void calculate_layouts();

	public:
		int m_layer = 3;
		int m_margin_top = 0;
		int m_offset = 0;

		Palette_Renderer(Engine& engine);
		~Palette_Renderer();

		void resize(int w, int h);

		void draw(Draw_Buffer& draw, int x, int y);
};

#endif // EOMAP_PALETTE_RENDERER_HPP
