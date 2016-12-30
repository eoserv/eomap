#ifndef EOMAP_DRAW_BUFFER_HPP
#define EOMAP_DRAW_BUFFER_HPP

#include "common.hpp"
#include "common_alsmart.hpp"

#include "third-party/variant.h"

#include <type_traits>
#include <vector>

namespace drawcmd
{
	struct Clear { ALLEGRO_COLOR color; };

	struct Blit { ALLEGRO_BITMAP* bmp; float x, y, z; int flags; };

	struct Draw_ImGui { };

	struct Sync { };
}

using Draw_Command = jss::variant<
	drawcmd::Clear,
	drawcmd::Blit,
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

		void clear(const ALLEGRO_COLOR& color)
		{ add(drawcmd::Clear{ color }); }

		void draw(ALLEGRO_BITMAP* bmp, float x, float y, float z,
		          int flags)
		{ add(drawcmd::Blit{ bmp, x, y, z, flags }); }

		void draw_imgui()
		{ add(drawcmd::Draw_ImGui{}); }

		void sync()
		{ add(drawcmd::Sync{}); }
};

#endif // EOMAP_DRAW_BUFFER_HPP
