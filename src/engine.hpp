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

class Engine
{
	public:
		struct Job
		{
			static constexpr double Important = -1.0;
			double cost_sec;
			func<void()> fn;
		};

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

		Full_EMF m_emf;

		bool m_gui_dirty = false;
		bool m_dirty = false;
		s_ptr<Map_Renderer> m_mapview;
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

	public:
		u_ptr<Engine_Drawing_Thread> m_drawing_thread;
		sig<void(s_ptr<Draw_Buffer>)> sig_draw;

		Engine();
		~Engine();

		Full_EMF& emf() { return m_emf; }

		GFX_Manager& gfx() { return m_gfx; }

		void add_post_frame_job(Job);
		void emplace_post_frame_job(Job&&);

		void run();
		void do_gui();

		void queue_upload(const s_bmp& bmp);

		void post(func<void()>);
		void die();

	friend class NewMapDialog;
	friend class AboutDialog;
};

#endif // EOMAP_GAME_HPP
