#ifndef EOMAP_DRAW_RENDERER_HPP
#define EOMAP_DRAW_RENDERER_HPP

#include "common.hpp"
#include "common_alsmart.hpp"
#include "draw_buffer.hpp"

#include <deque>
#include <vector>

struct Draw_Render_Data
{
	func<void()> draw_imgui;
};

void draw_render(Draw_Render_Data& data, Draw_Buffer& draw_buf);

#endif // EOMAP_DRAW_RENDERER_HPP
