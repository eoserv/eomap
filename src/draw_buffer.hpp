#ifndef EOMAP_DRAW_BUFFER_HPP
#define EOMAP_DRAW_BUFFER_HPP

#include "common.hpp"
#include "common_alsmart.hpp"

#include "third-party/variant.h"

#include <type_traits>
#include <vector>

namespace drawcmd
{
	struct Push_Target { s_bmp bmp; };
	struct Pop_Target { };
	struct Push_Clipper { int x, y, w, h; };
	struct Pop_Clipper { };

	struct Clear { ALLEGRO_COLOR color; };
	struct Clear_Depth { float depth; };

	struct Line { float x, y, x2, y2; ALLEGRO_COLOR color; };
	struct Filled_Rect { float x, y, x2, y2; ALLEGRO_COLOR color; };

	struct Blit { s_bmp bmp; float x, y, z; int flags; };

	struct Draw_ImGui { };

	struct Sync { };
}

using Draw_Command = jss::variant<
	drawcmd::Push_Target,
	drawcmd::Pop_Target,
	drawcmd::Push_Clipper,
	drawcmd::Pop_Clipper,
	drawcmd::Clear,
	drawcmd::Clear_Depth,
	drawcmd::Blit,
	drawcmd::Line,
	drawcmd::Filled_Rect,
	drawcmd::Draw_ImGui,
	drawcmd::Sync
>;

class Draw_Buffer
{
	public:
		std::vector<Draw_Command> m_cmd_buffer;

		Draw_Buffer();
		~Draw_Buffer();

		void add(const Draw_Command& cmd);

		void push_target(const s_bmp& bmp)
		{ add(drawcmd::Push_Target{ bmp }); }

		void pop_target()
		{ add(drawcmd::Pop_Target{}); }

		void push_clipper(int x, int y, int w, int h)
		{ add(drawcmd::Push_Clipper{ x, y, w, h }); }

		void pop_clipper()
		{ add(drawcmd::Pop_Clipper{}); }

		void clear(const ALLEGRO_COLOR& color)
		{ add(drawcmd::Clear{ color }); }

		void clear_depth(float depth)
		{ add(drawcmd::Clear_Depth{ depth }); }

		void draw(const s_bmp& bmp, float x, float y, float z, int flags)
		{ add(drawcmd::Blit{ bmp, x, y, z, flags }); }

		void line(float x, float y, float x2, float y2,
		          const ALLEGRO_COLOR& color)
		{ add(drawcmd::Line{ x, y, x2, y2, color }); }

		void filled_rect(float x, float y, float x2, float y2,
		                 const ALLEGRO_COLOR& color)
		{ add(drawcmd::Filled_Rect{ x, y, x2, y2, color }); }

		void draw_imgui()
		{ add(drawcmd::Draw_ImGui{}); }

		void sync()
		{ add(drawcmd::Sync{}); }
};

class Draw_Unique_Target
{
	private:
		Draw_Buffer& m_buffer;

	public:
		Draw_Unique_Target(Draw_Buffer& buffer, s_bmp target)
			: m_buffer(buffer)
		{
			m_buffer.push_target(target);
		}

		~Draw_Unique_Target()
		{
			m_buffer.pop_target();
		}
};

class Draw_Unique_Clipper
{
	private:
		Draw_Buffer& m_buffer;

	public:
		Draw_Unique_Clipper(Draw_Buffer& buffer, int x, int y, int x2, int y2)
			: m_buffer(buffer)
		{
			m_buffer.push_clipper(x, y, x2, y2);
		}

		~Draw_Unique_Clipper()
		{
			m_buffer.pop_clipper();
		}
};

#endif // EOMAP_DRAW_BUFFER_HPP
