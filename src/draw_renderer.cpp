#include "draw_renderer.hpp"

#include "util/cio/cio.hpp"
#include "third-party/variant.h"

#include <allegro5/allegro_primitives.h>

#include <cstdint>
#include <deque>
#include <map>
#include <stack>
#include <string>

namespace {

struct Draw_Vertex
{
	float x, y, z;
	float u, v;
};

static_assert(
	offsetof(Draw_Vertex, y) == offsetof(Draw_Vertex, x) + sizeof(float),
	"Bad offset for Draw_Vertex::y: not contiguous"
);

static_assert(
	offsetof(Draw_Vertex, z) == offsetof(Draw_Vertex, x) + sizeof(float)*2,
	"Bad offset for Draw_Vertex::z: not contiguous"
);

static_assert(
	offsetof(Draw_Vertex, v) == offsetof(Draw_Vertex, u) + sizeof(float),
	"Bad offset for Draw_Vertex::v: not contiguous"
);

static const ALLEGRO_VERTEX_ELEMENT draw_vert_elems[] = {
	{ ALLEGRO_PRIM_POSITION, ALLEGRO_PRIM_FLOAT_3, offsetof(Draw_Vertex, x) },
	{ ALLEGRO_PRIM_TEX_COORD_PIXEL, ALLEGRO_PRIM_FLOAT_2, offsetof(Draw_Vertex, u) },
	{ 0, 0, 0 }
};

struct Draw_Render_Dump_Visitor
{
	std::map<std::uintptr_t, int> bmp_id_table;
	int bmp_id_next = 0;

	void operator()(drawcmd::Push_Target& cmd)
	{
		cio::out << "[Push_Target, image:" << cmd.bmp.get() << "]\n";
	}

	void operator()(drawcmd::Pop_Target& cmd)
	{
		cio::out << "[Pop_Target]\n";
	}

	void operator()(drawcmd::Push_Clipper& cmd)
	{
		cio::out << "[Push_Clipper, x:" << cmd.x << ", y:" << cmd.y <<
		            ", w:" << cmd.w << ", h:" << cmd.h << "]\n";
	}

	void operator()(drawcmd::Pop_Clipper& cmd)
	{
		cio::out << "[Pop_Clipper]\n";
	}

	void operator()(drawcmd::Clear& cmd)
	{
		cio::out << "[Clear, color:(" << cmd.color.r << ',' << cmd.color.g
		         << ',' << cmd.color.b << ',' << cmd.color.a << ")]\n";
	}

	void operator()(drawcmd::Clear_Depth& cmd)
	{
		cio::out << "[Clear_Depth, depth:" << cmd.depth << "]\n";
	}

	void operator()(drawcmd::Line& cmd)
	{
		cio::out << "[Line, x:" << cmd.x << ", y:" << cmd.y << ", x2:"
		         << cmd.x2 << ", y2:" << cmd.y2 << ", color:("
		         << cmd.color.r << ',' << cmd.color.g << ',' << cmd.color.b
		         << ',' << cmd.color.a << "]\n";
	}

	void operator()(drawcmd::Filled_Rect& cmd)
	{
		cio::out << "[Filled_Rect, x:" << cmd.x << ", y:" << cmd.y << ", x2:"
		         << cmd.x2 << ", y2:" << cmd.y2 << ", color:("
		         << cmd.color.r << ',' << cmd.color.g << ',' << cmd.color.b
		         << ',' << cmd.color.a << "]\n";
	}

	void operator()(drawcmd::Blit& cmd)
	{
		cio::out << "[Blit, image:" << cmd.bmp.get() << ", x:"
		         << cmd.x << ", y:" << cmd.y << ", z:" << cmd.z << ", flags:"
		         << cmd.flags << "]\n";
	}

	void operator()(drawcmd::Draw_ImGui& cmd)
	{
		cio::out << "[Draw_ImGui]\n";
	}

	void operator()(drawcmd::Sync& cmd)
	{
		cio::out << "[Sync]\n";
	}
};

struct Draw_Render_Visitor
{
	struct clip_rect
	{
		int x, y, w, h;
	};

	std::stack<ALLEGRO_BITMAP*> old_target;
	std::stack<clip_rect> old_clipper;

	Draw_Render_Data& data;
	std::map<ALLEGRO_BITMAP*, std::vector<Draw_Vertex>> vertmap;

	static ALLEGRO_VERTEX_DECL* draw_vert_decl;

	struct Real_Bitmap
	{
		ALLEGRO_BITMAP* bmp;
		int x, y, w, h;
	};

	Real_Bitmap get_real_bitmap(ALLEGRO_BITMAP* bmp)
	{
		ALLEGRO_BITMAP* parent = al_get_parent_bitmap(bmp);
		Real_Bitmap result;

		if (!parent)
		{
			result.bmp = bmp;
			result.x = 0;
			result.y = 0;
		}
		else
		{
			result.bmp = parent;
			result.x = al_get_bitmap_x(bmp);
			result.y = al_get_bitmap_y(bmp);
		}

		result.w = al_get_bitmap_width(bmp);
		result.h = al_get_bitmap_height(bmp);

		return result;
	}

	void flush_verts()
	{
		for (const auto& p : vertmap)
		{
			auto&& tex = p.first;
			auto&& verts = p.second;

			al_draw_prim(verts.data(), draw_vert_decl, tex,
			             0, verts.size(), ALLEGRO_PRIM_TRIANGLE_LIST);
		}

		vertmap.clear();
	}

	Draw_Render_Visitor(Draw_Render_Data& data)
		: data(data)
	{
		if (!draw_vert_decl)
		{
			draw_vert_decl = al_create_vertex_decl(draw_vert_elems,
			                                       sizeof(Draw_Vertex));

			assert(draw_vert_decl != NULL);
		}
	}

	~Draw_Render_Visitor()
	{
		flush_verts();
	}

	void operator()(drawcmd::Push_Target& cmd)
	{
		flush_verts();
		old_target.push(al_get_target_bitmap());
		al_set_target_bitmap(cmd.bmp.get());
	}

	void operator()(drawcmd::Pop_Target& cmd)
	{
		flush_verts();
		if (old_target.empty() || old_target.top() == nullptr)
			;
		else
			al_set_target_bitmap(old_target.top());
		old_target.pop();
	}

	void operator()(drawcmd::Push_Clipper& cmd)
	{
		flush_verts();
		clip_rect clip;
		al_get_clipping_rectangle(&clip.x, &clip.y, &clip.w, &clip.h);
		old_clipper.push(clip);
		al_set_clipping_rectangle(cmd.x, cmd.y, cmd.w, cmd.h);
	}

	void operator()(drawcmd::Pop_Clipper& cmd)
	{
		flush_verts();
		auto& clip = old_clipper.top();
		al_set_clipping_rectangle(clip.x, clip.y, clip.w, clip.h);
		old_clipper.pop();
	}

	void operator()(drawcmd::Clear& cmd)
	{
		flush_verts();
		al_clear_to_color(cmd.color);
	}

	void operator()(drawcmd::Clear_Depth& cmd)
	{
		flush_verts();
		al_clear_depth_buffer(cmd.depth);
	}

	void operator()(drawcmd::Line& cmd)
	{
		flush_verts();
		al_draw_line(cmd.x, cmd.y, cmd.x2, cmd.y2, cmd.color, 0.0);
	}

	void operator()(drawcmd::Filled_Rect& cmd)
	{
		flush_verts();
		al_draw_filled_rectangle(cmd.x, cmd.y, cmd.x2, cmd.y2, cmd.color);
	}

	void operator()(drawcmd::Blit& cmd)
	{
		auto real = get_real_bitmap(cmd.bmp.get());

		float cmd_x = float(cmd.x);
		float cmd_y = float(cmd.y);
		float cmd_z = cmd.z;

		float real_x = float(real.x);
		float real_y = float(real.y);
		float w = float(real.w);
		float h = float(real.h);

		Draw_Vertex v1 = {
			cmd_x, cmd_y, cmd_z,
			real_x, real_y
		};

		Draw_Vertex v2 = {
			cmd_x + w, cmd_y, cmd_z,
			real_x + w, real_y
		};

		Draw_Vertex v3 = {
			cmd_x + w, cmd_y + h, cmd_z,
			real_x + w, real_y + h
		};

		Draw_Vertex v4 = {
			cmd_x, cmd_y + h, cmd_z,
			real_x, real_y + h
		};

		vertmap[real.bmp].push_back(v1);
		vertmap[real.bmp].push_back(v2);
		vertmap[real.bmp].push_back(v4);
		vertmap[real.bmp].push_back(v2);
		vertmap[real.bmp].push_back(v3);
		vertmap[real.bmp].push_back(v4);
	}

	void operator()(drawcmd::Draw_ImGui& cmd)
	{
		flush_verts();
		al_set_render_state(ALLEGRO_DEPTH_TEST, false);
		if (data.draw_imgui)
			data.draw_imgui();
		al_set_render_state(ALLEGRO_DEPTH_TEST, true);
	}

	void operator()(drawcmd::Sync& cmd)
	{
		flush_verts();
	}
};

ALLEGRO_VERTEX_DECL* Draw_Render_Visitor::draw_vert_decl = nullptr;

}

void draw_render(Draw_Render_Data& data, Draw_Buffer& draw_buf)
{
/*
	Draw_Render_Dump_Visitor dv;

	cio::out << "{\n";

	for (auto& cmd : draw_buf.m_cmd_buffer)
		jss::visit(dv, cmd);

	cio::out << '}' << cio::endl;
*/

	Draw_Render_Visitor v(data);

	for (auto& cmd : draw_buf.m_cmd_buffer)
		jss::visit(v, cmd);
}
