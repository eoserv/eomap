#ifndef EOMAP_GAME_HPP
#define EOMAP_GAME_HPP

#include "common.hpp"
#include "common_signal.hpp"

#include "eo_data.hpp"
#include "gfx_manager.hpp"

#include "dialog/newmapdialog.hpp"
#include "dialog/aboutdialog.hpp"

class Draw_Buffer;

class Engine;
class Engine_Drawing_Thread;

class GUI;
class Map_Renderer;
class Palette_Renderer;

class Framebuffer
{
	private:
		Engine& m_engine;
		s_bmp m_bmp;

		void destroy();

	public:
		explicit Framebuffer(Engine&);
		Framebuffer(Engine&, s_bmp);
		~Framebuffer();

		Framebuffer(Framebuffer&&);

		const Framebuffer& operator=(Framebuffer&&);

		Framebuffer& operator=(s_bmp&&);

		void resize(int w, int h);

		const s_bmp& bmp() const;
};

class Engine
{
	public:
		struct Job
		{
			static constexpr double Important = -1.0;
			double cost_sec;
			func<void()> fn;
		};

		u_ptr<Engine_Drawing_Thread> m_drawing_thread;

	private:
		struct impl_t;
		u_ptr<impl_t> impl;

		GFX_Manager m_gfx;

		int m_display_w = 0;
		int m_display_h = 0;
		const char* m_fail = nullptr;
		const char* m_map_filename = nullptr;
		bool m_show_demo = false;
		opt<AboutDialog> m_about_dialog;
		opt<NewMapDialog> m_new_map_dialog;

		opt<Full_EMF> m_emf;

		bool m_gui_dirty = false;
		bool m_dirty = false;
		int m_anim_frame = 0;

		s_ptr<Map_Renderer> m_mapview;
		Framebuffer m_mapview_fb;
		s_ptr<Palette_Renderer> m_palview;
		Framebuffer m_palview_fb;
		s_ptr<GUI> m_gui;

		void allegro_event(ALLEGRO_EVENT e);
		void display_update(int width, int height);

		void load_data();

		void do_new();
		void do_open(const char* filename);
		void do_save(const char* filename);

		void gui_new();
		void gui_open();
		void gui_save();
		void gui_save_as();

		void draw_scene();
		void draw_mapview(Draw_Buffer&);
		void draw_palview(Draw_Buffer&);

	public:
		sig<void(s_ptr<Draw_Buffer>)> sig_draw;

		Engine();
		~Engine();

		bool has_emf() { return m_emf != nullopt; }
		Full_EMF& emf() { return m_emf.value(); }

		GFX_Manager& gfx() { return m_gfx; }

		void add_post_frame_job(Job&&);

		void run();
		void do_gui();

		void queue_upload(const s_bmp& bmp);

		Framebuffer create_framebuffer(int w, int h);
		void destroy_framebuffer(s_bmp bmp);

		void post(func<void()>);
		void die();

	friend class NewMapDialog;
	friend class AboutDialog;
};

#endif // EOMAP_GAME_HPP
