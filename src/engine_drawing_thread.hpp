#ifndef EOMAP_ENGINE_DRAWING_THREAD_HPP
#define EOMAP_ENGINE_DRAWING_THREAD_HPP

#include "common.hpp"
#include "common_alsmart.hpp"
#include "common_signal.hpp"
#include "engine.hpp"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>

class Engine_Drawing_Thread
{
	private:
		enum signal_t
		{
			signal_die,
			signal_draw
		};

		Engine& m_engine;

		std::thread m_thread;

		ALLEGRO_EVENT_SOURCE m_event_source;

		alsmart::unique_display m_display;
		alsmart::unique_event_queue m_event_queue;
		ALLEGRO_TRANSFORM m_display_transform;
		ALLEGRO_TRANSFORM m_display_transform_inverted;

		std::deque<Engine::Job> m_pre_frame_jobs;
		std::mutex m_pre_frame_jobs_mutex;

		std::deque<Engine::Job> m_post_frame_jobs;
		std::mutex m_post_frame_jobs_mutex;

		s_ptr<Draw_Buffer> m_draw_buf;

		std::mutex m_imgui_mutex;

		double m_frame_time;
		double m_draw_times[32];
		int m_frames_drawn = 0;
		int m_draw_times_idx = 0;

		bool m_first_draw = true;
		bool m_running;

		bool m_display_ready = false;
		std::condition_variable m_display_ready_cond;
		std::mutex m_display_ready_mutex;

		void display_ready();

		void thread_main();
		void allegro_event(ALLEGRO_EVENT& e);
		void resize_display();

		void thread_die();
		void thread_draw(s_ptr<Draw_Buffer>);
		void thread_signal(intptr_t sig, s_ptr<void> data);

		void signal(intptr_t sig, s_ptr<void> data);

	public:
		sig<void(ALLEGRO_EVENT)> sig_allegro_event;
		sig<void(int, int)> sig_display_update;

		Engine_Drawing_Thread(Engine& engine, sig<void(s_ptr<Draw_Buffer>)>& draw_signal);
		~Engine_Drawing_Thread();

		void emplace_pre_frame_job(Engine::Job&&);
		void emplace_post_frame_job(Engine::Job&&);

		void die();
		void draw(s_ptr<Draw_Buffer>);

		std::unique_lock<std::mutex> lock_imgui();
		void wait_for_imgui();
};

#endif // EOMAP_ENGINE_DRAWING_THREAD_HPP
