#include "gui.hpp"

#include "common_alsmart.hpp"
#include "draw_buffer.hpp"
#include "engine.hpp"

GUI::GUI(Engine& engine)
	: m_engine(engine)
{ }

GUI::~GUI()
{ }

void GUI::resize(int w, int h)
{
	// dummied out: not compositing
}

void GUI::draw(Draw_Buffer& draw, int x, int y)
{
	static const ALLEGRO_COLOR trans = al_map_rgba_f(0.f, 0.f, 0.f, 0.f);

	m_engine.do_gui();

	// TODO: I'd like to move all the GUI logic here instead of being in Engine
	draw.draw_imgui();
}
