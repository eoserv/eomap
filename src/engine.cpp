#include "engine.hpp"

#include "draw_buffer.hpp"
#include "engine_drawing_thread.hpp"
#include "eo_stream.hpp"
#include "gui.hpp"
#include "map_renderer.hpp"
#include "palette_renderer.hpp"

#include "util/alsmart/native_dialog.hpp"

#include "third-party/imgui/imgui.h"
#include "third-party/imgui/imgui_impl_a5.h"

#define WIN32_LEAN_AND_MEAN
#define ASIO_STANDALONE
#include <asio.hpp>

// Has to be below asio for Windows
#include <allegro5/allegro_opengl.h>

#include <cmath>

void Framebuffer::destroy()
{
	if (m_bmp)
		m_engine.destroy_framebuffer(m_bmp);
}

Framebuffer::Framebuffer(Engine& engine)
	: m_engine(engine)
{ }

Framebuffer::Framebuffer(Engine& engine, s_bmp bmp)
	: m_engine(engine)
	, m_bmp(bmp)
{ }

Framebuffer::~Framebuffer()
{
	destroy();
}

Framebuffer::Framebuffer(Framebuffer&& other)
	: m_engine(other.m_engine)
	, m_bmp(std::move(other.m_bmp))
{ }

const Framebuffer& Framebuffer::operator=(Framebuffer&& other)
{
	destroy();
	m_bmp = std::move(other.m_bmp);
}

Framebuffer& Framebuffer::operator=(s_bmp&& bmp)
{
	destroy();
	m_bmp = std::move(bmp);
}

void Framebuffer::resize(int w, int h)
{
	destroy();
	m_bmp = m_engine.create_framebuffer(w, h).m_bmp;
}

const s_bmp& Framebuffer::bmp() const
{
	return m_bmp;
}

struct Engine::impl_t
{
	asio::io_service io_service;
	asio::io_service::work work;

	impl_t()
		: work(io_service)
	{ }
};

void Engine::allegro_event(ALLEGRO_EVENT e)
{
	{
		// TODO: This breaks window dragging - need a better way to check mouse
		//   moving on to the GUI.
		do_gui();

		auto lock = m_drawing_thread->lock_imgui();

		if (ImGui_ImplA5_ProcessEvent(&e))
		{
			m_dirty = true;
			return;
		}
	}

	switch (e.type)
	{
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			die();
		break;

		// Only timer is the animation timer
		case ALLEGRO_EVENT_TIMER:
			++m_anim_frame;
			m_dirty = true;
		break;

		case ALLEGRO_EVENT_DISPLAY_EXPOSE:
			m_dirty = true;
		break;

		case ALLEGRO_EVENT_DISPLAY_RESIZE:
			m_dirty = true;
		break;

		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			//if (m_scene)
			//	m_scene->mouse_down(e.mouse.x + 0.5f, e.mouse.y + 0.5f,
			//	                    e.mouse.button);
		break;

		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			//if (m_scene)
			//	m_scene->mouse_up(e.mouse.x + 0.5f, e.mouse.y + 0.5f,
			//	                  e.mouse.button);
		break;

		case ALLEGRO_EVENT_MOUSE_AXES:
			//if (m_scene)
			//	m_scene->mouse_move(e.mouse.x + 0.5f, e.mouse.y + 0.5f);
		break;

		case ALLEGRO_EVENT_KEY_DOWN:
			//if (m_scene)
			//	m_scene->key_down(e.keyboard.keycode);
		break;

		case ALLEGRO_EVENT_KEY_UP:
			//if (m_scene)
			//	m_scene->key_down(e.keyboard.keycode);
		break;

		case ALLEGRO_EVENT_KEY_CHAR:
			//if (m_scene)
			//	m_scene->key_char(e.keyboard.keycode, e.keyboard.unichar,
			//	                  e.keyboard.modifiers, e.keyboard.repeat);
		break;

	}
}

void Engine::display_update(int width, int height)
{
	m_display_w = width;
	m_display_h = height;

	if (m_gui)
		m_gui->resize(width, height);

	if (m_mapview)
		m_mapview->resize(width, height);

	if (m_palview)
		m_palview->resize(width, height);

	// TODO: Remove these - hard-coded for full-screen views
	m_mapview_fb.resize(width - 320, height - 19);
	m_palview_fb.resize(320, height - 19);
}

void Engine::load_data()
{
	m_fail = nullptr;

	if (!m_gfx.load_file(3, "./gfx/gfx003.egf"))
	{
		m_fail = "Failed to load gfx/gfx003.egf";
		return;
	}

	if (!m_gfx.load_file(4, "./gfx/gfx004.egf"))
	{
		m_fail = "Failed to load gfx/gfx004.egf";
		return;
	}

	if (!m_gfx.load_file(5, "./gfx/gfx005.egf"))
	{
		m_fail = "Failed to load gfx/gfx005.egf";
		return;
	}

	if (!m_gfx.load_file(6, "./gfx/gfx006.egf"))
	{
		m_fail = "Failed to load gfx/gfx006.egf";
		return;
	}

	if (!m_gfx.load_file(7, "./gfx/gfx007.egf"))
	{
		m_fail = "Failed to load gfx/gfx007.egf";
		return;
	}

	if (!m_gfx.load_file(22, "./gfx/gfx022.egf"))
	{
		m_fail = "Failed to load gfx/gfx022.egf";
		return;
	}
}

void Engine::do_new()
{
	m_map_filename = nullptr;
	m_emf = {};
}

void Engine::do_open(const char* filename)
{
	m_map_filename = filename;

	cio::stream file(filename);
	file.seek_reverse(0);
	long filesize = file.tell();
	file.seek(0);

	u_ptr<char[]> data(new char[filesize]);
	file.read(data.get(), filesize);

	EO_Stream_Reader reader(data.get(), filesize);
	m_emf = Full_EMF{};
	m_emf->unserialize(reader);
}

void Engine::do_save(const char* filename)
{
	m_map_filename = filename;
}

void Engine::gui_new()
{
	m_new_map_dialog.emplace(NewMapDialog(*this));
}

void Engine::gui_open()
{
	// TODO: Check if map is modified and warn before clobbering

	auto dialog = alsmart::create_native_file_dialog_unique(
		nullptr, "Open Map", "*.emf", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST
	);

	if (dialog && al_show_native_file_dialog(nullptr, dialog.get()))
	{
		if (al_get_native_file_dialog_count(dialog.get()) > 0)
		{
			const char* filename = al_get_native_file_dialog_path(dialog.get(), 0);
			do_open(filename);
		}
	}
}

void Engine::gui_save()
{
	if (m_map_filename)
	{
		do_save(m_map_filename);
	}
	else
	{
		gui_save_as();
	}
}

void Engine::gui_save_as()
{
	auto dialog = alsmart::create_native_file_dialog_unique(
		nullptr, "Save Map", "*.emf", ALLEGRO_FILECHOOSER_SAVE
	);

	if (dialog && al_show_native_file_dialog(nullptr, dialog.get()))
	{
		if (al_get_native_file_dialog_count(dialog.get()) > 0)
		{
			const char* filename = al_get_native_file_dialog_path(dialog.get(), 0);
			do_save(filename);
		}
	}
}

void Engine::draw_scene()
{
	auto draw = std::make_shared<Draw_Buffer>();

	draw->clear(al_map_rgb_f(0.f, 0.f, 0.f));

	draw_mapview(*draw);
	draw_palview(*draw);

	// TODO: Remove these - temporary full-screen views
	draw->draw(m_mapview_fb.bmp(), 0, 19, 0.f, 0);
	draw->draw(m_palview_fb.bmp(), m_display_w - 320, 19, 0.f, 0);

	m_gui->draw(*draw, 0, 0);

	sig_draw(draw);
}

void Engine::draw_mapview(Draw_Buffer& draw)
{
	Draw_Unique_Target target(draw, m_mapview_fb.bmp());
	draw.clear(al_map_rgb_f(0.f, 0.f, 0.f));
	draw.clear_depth(1.0f);
	m_mapview->draw(draw, 0, 0);
}

void Engine::draw_palview(Draw_Buffer& draw)
{
	Draw_Unique_Target target(draw, m_palview_fb.bmp());
	draw.clear(al_map_rgb_f(0.f, 0.0f, 0.f));
	draw.clear_depth(1.0f);
	m_palview->draw(draw, 0, 0);
}

Engine::Engine()
	: impl(std::make_unique<impl_t>())
	, m_gfx(*this)
	, m_mapview_fb(*this)
	, m_palview_fb(*this)
{
	al_set_new_bitmap_flags(ALLEGRO_ALPHA_TEST);

	using namespace std::placeholders;

	m_drawing_thread.reset(new Engine_Drawing_Thread(*this, sig_draw));

	m_drawing_thread->sig_allegro_event.connect(
		[this](ALLEGRO_EVENT e)
		{
			impl->io_service.post([this, e]()
			{
				allegro_event(e);
			});
		}
	);

	m_drawing_thread->sig_display_update.connect(
		std::bind(&Engine::display_update, this, _1, _2)
	);
}

Engine::~Engine()
{

}

void Engine::add_post_frame_job(Engine::Job&& job)
{
	if (job.cost_sec == Job::Important)
		m_drawing_thread->add_pre_frame_job(std::move(job));
	else
		m_drawing_thread->add_post_frame_job(std::move(job));
}

void Engine::run()
{
	m_gui = std::make_shared<GUI>(*this);

	m_drawing_thread->wait_for_imgui();

	// Draw loading screen
	auto loading_msg = [&](const char* msg)
	{
		bool always_open = true;
		auto draw = std::make_shared<Draw_Buffer>();
		auto lock = m_drawing_thread->lock_imgui();

		ImGui_ImplA5_NewFrame();

		ImGui::OpenPopup("Loading");

		auto flags = ImGuiWindowFlags_NoResize
		           | ImGuiWindowFlags_NoTitleBar
		           | ImGuiWindowFlags_NoSavedSettings;

		ImGui::SetNextWindowPosCenter();

		ImGui::OpenPopup("Loading");

		if (ImGui::BeginPopupModal("Loading", &always_open, flags))
		{
			ImGui::NewLine();
			ImGui::Text("%s", msg);
			ImGui::NewLine();

			ImGui::EndPopup();
		}

		draw->clear(al_map_rgb_f(0, 0, 0));
		draw->draw_imgui();
		sig_draw(draw);
	};

	// Extra call to set up window width
	loading_msg("                    ");
	loading_msg("     Loading...     ");

	load_data();

	m_mapview = std::make_shared<Map_Renderer>(*this);
	m_palview = std::make_shared<Palette_Renderer>(*this);

	// Re-trigger the display resize handler now the views are created
	{
		int w = m_display_w;
		int h = m_display_h;
		display_update(w, h);
	}

	draw_scene();

	while (!impl->io_service.stopped())
	{
		impl->io_service.run_one();

		if (m_dirty)
		{
			m_dirty = false;
			draw_scene();
		}
	}
}

void Engine::do_gui()
{
	bool mutated = false;
	bool always_open = true;
	auto lock = m_drawing_thread->lock_imgui();

	ImGui_ImplA5_NewFrame();

	if (m_fail)
	{
		ImGui::SetNextWindowPos(ImVec2(10.f, 10.f));
		if (ImGui::Begin("Startup fail", &always_open, ImVec2(0.f, 0.f), 0.f, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::TextColored(ImVec4(1.f, 0.5f, 0.5f, 1.f), "%s", m_fail);

			ImGui::BeginGroup();
				if (ImGui::Button("Retry"))
				{
					load_data();
				}

				ImGui::SameLine();

				if (ImGui::Button("Exit"))
				{
					die();
				}
			ImGui::EndGroup();
		}
		ImGui::End();

		return;
	}

	if (ImGui::BeginMainMenuBar())
    {
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "CTRL+N"))
			{
				gui_new();
			}

			if (ImGui::MenuItem("Open", "CTRL+O"))
			{
				gui_open();
			}

			if (ImGui::MenuItem("Save", "CTRL+S", false, m_map_filename))
			{
				gui_save();
			}

			if (ImGui::MenuItem("Save As", "CTRL+A", false, has_emf()))
			{
				gui_save_as();
			}

			if (ImGui::MenuItem("Quit", "CTRL+Q"))
			{
				die();
			}

			int menu_height = std::round(ImGui::GetWindowSize().y);

			if (menu_height != m_mapview->m_margin_top)
			{
				m_mapview->m_margin_top = menu_height;
				m_palview->m_margin_top = menu_height;
				m_dirty = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Map"))
		{
			if (ImGui::MenuItem("Properties", "CTRL+P", false, has_emf()))
			{
				//gui_map_properties();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Layers"))
		{
			static const char* const layer_names[10] = {
				"Ground",    "Objects",    "Overlay",
				"Down Wall", "Right Wall", "Roof",
				"Top",       "Shadow",     "Overlay 2",
				"Special"
			};

			static const char* const layer_hotkeys[10] = {
				"Alt+1", "Alt+2", "Alt+3",
				"Alt+4", "Alt+5", "Alt+6",
				"Alt+7", "Alt+8", "Alt+9",
				"Alt+0",
			};

			for (size_t i = 0; i < 10; ++i)
			{
				if (ImGui::MenuItem(layer_names[i], layer_hotkeys[i], &m_mapview->m_layer_visible[i]))
					m_dirty = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				m_about_dialog.emplace(AboutDialog(*this));
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
    }

	if (m_about_dialog)
		mutated |= m_about_dialog.value().do_gui();

	if (m_new_map_dialog)
		mutated |= m_new_map_dialog.value().do_gui();

	if (mutated)
	{
		lock.unlock();
		do_gui();
	}
}

void Engine::queue_upload(const s_bmp& bmp)
{
	assert(bmp);

	add_post_frame_job({Job::Important, [bmp]()
	{
		al_convert_bitmap(bmp.get());
	}});
}

Framebuffer Engine::create_framebuffer(int w, int h)
{
	// TODO: This is triggering some bug in allegro that I need to work around
	// Hacked around by removing liens 169-172 of ogl_fbo.c in Allegro 5.2.1.1
	auto bmp = alsmart::create_bitmap_shared(w, h);

	add_post_frame_job({Job::Important, [bmp]()
	{
		al_convert_bitmap(bmp.get());
		al_get_opengl_fbo(bmp.get());
	}});

	return Framebuffer(*this, bmp);
}

void Engine::destroy_framebuffer(s_bmp bmp)
{
	add_post_frame_job({0.001, [this, bmp]()
	{
		// The existence of this function will keep a reference to the bitmap
		//   alive long enough for the draw thread to finish with it.

		al_remove_opengl_fbo(bmp.get());
	}});
}

void Engine::post(func<void()> fn)
{
	impl->io_service.post(fn);
}

void Engine::die()
{
	m_drawing_thread->die();
	m_drawing_thread->join();
	impl->io_service.stop();
}
