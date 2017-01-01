#ifndef EOMAP_ATLAS_HPP
#define EOMAP_ATLAS_HPP

#include "common.hpp"
#include "common_alsmart.hpp"

#include <deque>
#include <map>
#include <vector>

class Engine;

class Atlas
{
	private:
		static constexpr int x_res = 32;
		static constexpr int y_res = 32;

		struct PageEntry
		{
			int x, y, w, h;
		};

		struct Entry
		{
			int page;
			PageEntry pe;
		};

		struct Page
		{
			bool fresh = true;
			int width, height;
			std::vector<int> col_heights;
			s_bmp bmp;

			Page(int width, int height)
				: width(width)
				, height(height)
			{
				bmp = alsmart::create_bitmap_shared(width, height);
				alsmart::unique_draw_target tgt(bmp.get());
				al_clear_to_color(al_map_rgba(0,0,0,0));
				col_heights.resize((width + x_res - 1) / x_res);
			}

			opt<PageEntry> add(int width, int height);
		};

		int m_page_w;
		int m_page_h;

		std::vector<Page> m_pages;
		std::map<int, pair<Entry, s_bmp>> m_entries;

		void new_page();
		Page& last_page();

	public:
		Atlas(int page_w, int page_h);
		~Atlas();

		s_bmp add(int id, int width, int height);
		s_bmp get(int id);

		s_bmp get_page_bmp(int id)
		{
			return m_pages[id].bmp;
		}

		std::vector<int> get_entry_list();

		void upload(Engine& engine);
};

#endif // EOMAP_ATLAS_HPP
