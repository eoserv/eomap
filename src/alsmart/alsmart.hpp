#ifndef ALSMART_ALSMART_HPP
#define ALSMART_ALSMART_HPP

#include <allegro5/allegro.h>
#include <memory>

namespace alsmart
{
	typedef char ALSMART_CSTR;

	#define ALSMART_DEFINE_DESTROY_CLASS(T, name, Fn) \
		struct destroy_##T \
		{ \
			void operator()(T* name) const \
			{ \
				Fn(name); \
			} \
		}; \
		typedef std::unique_ptr<T, destroy_##T> unique_##name; \
		typedef std::shared_ptr<T> shared_##name; \
		typedef std::weak_ptr<T> weak_##name;

	#define ALSMART_DEFINE_CONSTRUCTOR(T, name) \
		template <class ...Args> std::unique_ptr<T, destroy_##T> name##_unique(const Args& ...args) \
		{ \
			return std::unique_ptr<T, destroy_##T>(al_##name(args...)); \
		} \
		template <class ...Args> std::shared_ptr<T> name##_shared(const Args& ...args) \
		{ \
			return std::shared_ptr<T>(al_##name(args...), destroy_##T()); \
		}

	// ---

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_BITMAP, bitmap, al_destroy_bitmap)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_CONFIG, config, al_destroy_config)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_DISPLAY, display, al_destroy_display)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_EVENT_QUEUE, event_queue, al_destroy_event_queue)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_FILE, file, al_fclose)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_FS_ENTRY, fs_entry, al_destroy_fs_entry)

#if 0
	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_HAPTIC, haptic, al_release_haptic)
#endif

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_JOYSTICK, joystick, al_release_joystick)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_PATH, path, al_destroy_path)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_SHADER, shader, al_destroy_shader)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_THREAD, thread, al_destroy_thread)
	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_MUTEX, mutex, al_destroy_mutex)
	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_COND, cond, al_destroy_cond)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_TIMER, timer, al_destroy_timer)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_USTR, ustr, al_ustr_free)
	ALSMART_DEFINE_DESTROY_CLASS(ALSMART_CSTR, cstr, al_free)

	// ---

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_BITMAP, create_bitmap)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_BITMAP, create_sub_bitmap)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_BITMAP, clone_bitmap)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_BITMAP, load_bitmap)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_BITMAP, load_bitmap_flags)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_BITMAP, load_bitmap_f)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_BITMAP, load_bitmap_flags_f)

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_CONFIG, create_config)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_CONFIG, load_config_file)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_CONFIG, load_config_file_f)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_CONFIG, merge_config)

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_DISPLAY, create_display)

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_EVENT_QUEUE, create_event_queue)

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FILE, fopen)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FILE, fopen_interface)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FILE, fopen_slice)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FILE, fopen_fd)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FILE, make_temp_file)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FILE, create_file_handle)

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FS_ENTRY, create_fs_entry)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FS_ENTRY, read_directory)

#if 0
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_HAPTIC, get_haptic_from_mouse)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_HAPTIC, get_haptic_from_keyboard)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_HAPTIC, get_haptic_from_display)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_HAPTIC, get_haptic_from_joystick)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_HAPTIC, get_haptic_from_touch_input)
#endif

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_JOYSTICK, get_joystick)

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_PATH, create_path)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_PATH, create_path_for_directory)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_PATH, clone_path)
	
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_SHADER, create_shader)
	
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_THREAD, create_thread)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_MUTEX, create_mutex)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_MUTEX, create_mutex_recursive)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_COND, create_cond)
	
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_TIMER, create_timer)

	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_USTR, ustr_new)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_USTR, ustr_new_from_buffer)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_USTR, ustr_newf)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_USTR, ustr_dup)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_USTR, ustr_dup_substr)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_USTR, ustr_new_from_utf16)

	ALSMART_DEFINE_CONSTRUCTOR(ALSMART_CSTR, cstr_dup)

	class unique_bitmap_flags
	{
		private:
			int m_flags;

		public:
			unique_bitmap_flags(int flags, bool override = false)
			{
				m_flags = al_get_new_bitmap_flags();
				al_set_new_bitmap_flags(override ? flags : (m_flags | flags));
			}

			~unique_bitmap_flags()
			{
				al_set_new_bitmap_flags(m_flags);
			}

			unique_bitmap_flags(const unique_bitmap_flags&) = delete;
			unique_bitmap_flags& operator=(const unique_bitmap_flags) = delete;
	};

	class unique_draw_target
	{
		private:
			ALLEGRO_BITMAP* m_target;

		public:
			unique_draw_target(ALLEGRO_BITMAP* target)
			{
				m_target = al_get_target_bitmap();
				al_set_target_bitmap(target);
			}

			~unique_draw_target()
			{
				al_set_target_bitmap(m_target);
			}

			unique_draw_target(const unique_draw_target&) = delete;
			unique_draw_target& operator=(const unique_draw_target) = delete;
	};

	class unique_draw_hold
	{
		public:
			unique_draw_hold()
			{
				al_hold_bitmap_drawing(true);
			}

			~unique_draw_hold()
			{
				al_hold_bitmap_drawing(false);
			}

			unique_draw_hold(const unique_draw_target&) = delete;
			unique_draw_hold& operator=(const unique_draw_target) = delete;
	};

	class unique_draw_clipper
	{
		private:
			int m_x, m_y, m_w, m_h;

		public:
			unique_draw_clipper(int x, int y, int w, int h)
			{
				al_get_clipping_rectangle(&m_x, &m_y, &m_w, &m_h);
				al_set_clipping_rectangle(x, y, w, h);
			}

			~unique_draw_clipper()
			{
				al_set_clipping_rectangle(m_x, m_y, m_w, m_h);
			}

			unique_draw_clipper(const unique_draw_clipper&) = delete;
			unique_draw_clipper& operator=(const unique_draw_clipper) = delete;
	};
}


#endif // ALSMART_ALSMART_HPP
