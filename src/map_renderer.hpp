#ifndef EOMAP_MAP_RENDERER_HPP
#define EOMAP_MAP_RENDERER_HPP

class Draw_Buffer;
class Engine;

class Map_Renderer
{
	private:
		Engine& m_engine;

		int m_view_w = 0;
		int m_view_h = 0;

	public:
		bool m_layer_visible[10] = { true, true, true, true, true,
		                             true, true, true, true, false };
		int m_margin_top = 0;
		bool m_is_open = false;
		int m_offset_x = 0;
		int m_offset_y = 0;

		Map_Renderer(Engine& engine);
		~Map_Renderer();

		void resize(int w, int h);

		void draw(Draw_Buffer& draw, int x, int y);
};

#endif // EOMAP_MAP_RENDERER_HPP
