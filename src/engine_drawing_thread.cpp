#include "engine_drawing_thread.hpp"

#include "draw_renderer.hpp"

#include "cio/cio.hpp"

#include "third-party/imgui/imgui.h"
#include "third-party/imgui/imgui_impl_a5.h"

#include <numeric>

enum
{
	EOMAP_ENGINE_SIGNAL_TYPE = ALLEGRO_GET_EVENT_TYPE('E', 'O', 'M', 'A')
};

void Engine_Drawing_Thread::display_ready()
{
	std::unique_lock<std::mutex> lock(m_display_ready_mutex);
	m_display_ready = true;
	lock.unlock();
	m_display_ready_cond.notify_all();
}

void Engine_Drawing_Thread::thread_main()
{
	al_set_new_display_flags(ALLEGRO_RESIZABLE | ALLEGRO_GENERATE_EXPOSE_EVENTS);
	al_set_new_display_option(ALLEGRO_AUTO_CONVERT_BITMAPS, 0, ALLEGRO_REQUIRE);
	//al_set_new_display_option(ALLEGRO_DEPTH_SIZE, 16, ALLEGRO_SUGGEST);
	// Main loop is event-triggered, causing input lag with vsync on
	al_set_new_display_option(ALLEGRO_SINGLE_BUFFER, 1, ALLEGRO_SUGGEST);
	al_set_new_display_option(ALLEGRO_VSYNC, 0, ALLEGRO_SUGGEST);
	m_display = alsmart::create_display_unique(640, 480);

	int refresh_rate = al_get_display_refresh_rate(m_display.get());

	if (refresh_rate < 1)
		refresh_rate = 60;

	m_frame_time = 1.0 / refresh_rate;

	for (int i = 0; i < 32; ++i)
		m_draw_times[i] = m_frame_time;

	resize_display();

	al_register_event_source(m_event_queue.get(), al_get_display_event_source(m_display.get()));
	al_register_event_source(m_event_queue.get(), al_get_keyboard_event_source());
	al_register_event_source(m_event_queue.get(), al_get_mouse_event_source());

	al_set_target_backbuffer(m_display.get());
	//al_set_render_state(ALLEGRO_DEPTH_TEST, true);

	ImGui_ImplA5_Init(m_display.get());
	Imgui_ImplA5_CreateDeviceObjects();

	display_ready();

	m_running = true;
	m_first_draw = true;

	ALLEGRO_EVENT e;

	while (m_running)
	{
		// TODO: Try delay the draw request based on frame timings for input lag
		if (m_draw_buf || m_post_frame_jobs.size() > 0)
		{
			thread_draw(m_draw_buf);
			m_draw_buf.reset();
		}

		al_wait_for_event(m_event_queue.get(), &e);
		allegro_event(e);

		while (al_get_next_event(m_event_queue.get(), &e))
		{
			allegro_event(e);
		}
	}

	m_running = true;
}

void Engine_Drawing_Thread::allegro_event(ALLEGRO_EVENT& e)
{
	switch (e.type)
	{
		case ALLEGRO_EVENT_DISPLAY_RESIZE:
		{
			auto lock = lock_imgui();
			ImGui_ImplA5_InvalidateDeviceObjects();
			al_acknowledge_resize(e.display.source);
			resize_display();
			Imgui_ImplA5_CreateDeviceObjects();
			sig_allegro_event(e);
		}
		break;

		case EOMAP_ENGINE_SIGNAL_TYPE:
		{
			intptr_t sig = e.user.data1;
			std::unique_ptr<s_ptr<void>> data(
				reinterpret_cast<s_ptr<void>*>(e.user.data2));

			thread_signal(sig, *data);
		}
		break;

		default:
			sig_allegro_event(e);
	}
}

void Engine_Drawing_Thread::resize_display()
{
	int real_width = al_get_display_width(m_display.get());
	int real_height = al_get_display_height(m_display.get());

	sig_display_update(real_width, real_height);
}

void Engine_Drawing_Thread::thread_die()
{
	m_running = false;
}

void Engine_Drawing_Thread::thread_draw(s_ptr<Draw_Buffer> draw_buf)
{
	auto draw_imgui = [this]()
	{
		auto lock = lock_imgui();
		ImGui::Render();
		auto draw_data = ImGui::GetDrawData();

		if (draw_data)
		{
			ImGui_ImplA5_RenderDrawLists(draw_data);
		}
	};

	int d = std::min(32, m_frames_drawn);
	auto draw_times_sum = std::accumulate(std::begin(m_draw_times), std::begin(m_draw_times) + d, 0.0);
	double avg_draw_time = d == 0 ? m_frame_time : (draw_times_sum / d);

	double work_start, work_end, work_time;

	work_start = al_get_time();
	{
		std::unique_lock<std::mutex> lock(m_pre_frame_jobs_mutex);

		while (m_pre_frame_jobs.size() > 0)
		{
			Engine::Job& job = m_pre_frame_jobs.back();
			job.fn();
			m_pre_frame_jobs.pop_back();
		}
	}
	work_end = al_get_time();

	work_time = work_end - work_start;

	double jobs_start, jobs_end, jobs_time;

	jobs_start = al_get_time();
	{
		double post_frame_time = m_frame_time - (avg_draw_time * 2);

		if (post_frame_time < m_frame_time / 8)
			post_frame_time = m_frame_time / 8;
		else if (post_frame_time > m_frame_time / 2)
			post_frame_time = m_frame_time / 2;

		// try_lock_for is unsupported, I guess...
		std::unique_lock<std::mutex> lock(m_post_frame_jobs_mutex, std::try_to_lock);

		if (lock)
		{
			while (m_post_frame_jobs.size() > 0 && post_frame_time > al_get_time() - jobs_start)
			{
				Engine::Job& job = m_post_frame_jobs.back();
				job.fn();
				m_post_frame_jobs.pop_back();
			}
		}
	}
	jobs_end = al_get_time();

	jobs_time = jobs_end - jobs_start;

	if (draw_buf)
	{
		double draw_start, draw_end, draw_time;

		al_set_target_backbuffer(m_display.get());

		draw_start = al_get_time();
		{
			Draw_Render_Data render_data{draw_imgui};
			draw_render(render_data, *draw_buf);
			al_flip_display();
		}
		draw_end = al_get_time();

		draw_time = (draw_end - draw_start) + work_time;

		cio::out << "Frame took " << (draw_time * 1000.0) << " ms to draw" << cio::endl;

		m_draw_times[m_draw_times_idx++] = draw_time;
		m_frames_drawn++;

		if (m_draw_times_idx >= 32)
			m_draw_times_idx = 0;

		m_first_draw = false;
	}
}

void Engine_Drawing_Thread::thread_signal(intptr_t sig, s_ptr<void> data)
{
	switch (sig)
	{
		case signal_die:
			thread_die();
		break;

		case signal_draw:
			m_draw_buf = std::static_pointer_cast<Draw_Buffer>(data);
		break;
	}
}

void Engine_Drawing_Thread::signal(intptr_t sig, s_ptr<void> data)
{
	ALLEGRO_EVENT event;

	event.user = {};
	event.user.type = EOMAP_ENGINE_SIGNAL_TYPE;
	event.user.data1 = sig;
	event.user.data2 = reinterpret_cast<intptr_t>(new s_ptr<void>(data));

	al_emit_user_event(&m_event_source, &event, nullptr);
}

Engine_Drawing_Thread::Engine_Drawing_Thread(Engine& engine, sig<void(s_ptr<Draw_Buffer>)>& draw_signal)
	: m_engine(engine)
	, m_thread(std::bind(&Engine_Drawing_Thread::thread_main, this))
{
	m_event_queue = alsmart::create_event_queue_unique();
	using namespace std::placeholders;
	draw_signal.connect(std::bind(&Engine_Drawing_Thread::draw, this, _1));

	al_init_user_event_source(&m_event_source);
	al_register_event_source(m_event_queue.get(), &m_event_source);
}

Engine_Drawing_Thread::~Engine_Drawing_Thread()
{
	die();
	m_thread.join();
	al_unregister_event_source(m_event_queue.get(), &m_event_source);
	al_destroy_user_event_source(&m_event_source);

	ImGui_ImplA5_Shutdown();
}

void Engine_Drawing_Thread::emplace_pre_frame_job(Engine::Job&& job)
{
	std::unique_lock<std::mutex> l(m_pre_frame_jobs_mutex);
	m_pre_frame_jobs.push_back(job);
}

void Engine_Drawing_Thread::emplace_post_frame_job(Engine::Job&& job)
{
	std::unique_lock<std::mutex> l(m_post_frame_jobs_mutex);
	m_post_frame_jobs.push_back(job);
}

void Engine_Drawing_Thread::die()
{
	signal(signal_die, nullptr);
}

void Engine_Drawing_Thread::draw(s_ptr<Draw_Buffer> draw_buf)
{
	signal(signal_draw, draw_buf);
}

std::unique_lock<std::mutex> Engine_Drawing_Thread::lock_imgui()
{
	return std::unique_lock<std::mutex>(m_imgui_mutex);
}

void Engine_Drawing_Thread::wait_for_imgui()
{
	std::unique_lock<std::mutex> lock(m_display_ready_mutex);
	m_display_ready_cond.wait(lock, [this]() { return m_display_ready; });
}
