#include "atlas.hpp"

#include "engine.hpp"

#include "util/cio/cio.hpp"

#include <numeric>

opt<Atlas::PageEntry> Atlas::Page::add(int width, int height)
{
	if (width > this->width || height > this->height)
		return nullopt;

	int page_width = (this->width + x_res - 1) / x_res;
	int page_height = (this->height + x_res - 1) / y_res;

	int block_width = (width + x_res - 1) / x_res;
	int block_height = (height + y_res - 1) / y_res;

	int start_y = std::accumulate(col_heights.begin(), col_heights.end(),
	                              page_height, [](int x, int y) { return std::min(x, y); });

	for (int y = start_y; y < page_height - block_height; ++y)
	{
		for (int x = 0; x < page_width - block_width; ++x)
		{
			int max_y = std::accumulate(col_heights.begin() + x,
			                            col_heights.begin() + x + block_width,
			                            0, [](int x, int y) { return std::max(x, y); });

			if (max_y < y)
			{
				for (auto it  = col_heights.begin() + x;
				          it != col_heights.begin() + x + block_width;
				        ++it)
				{
					*it = max_y + block_height;
				}

				fresh = false;

				return PageEntry{x * x_res, y * y_res, width, height};
			}
		}
	}

	return nullopt;
}

void Atlas::new_page()
{
	m_pages.emplace_back(m_page_w, m_page_h);
}

Atlas::Page& Atlas::last_page()
{
	return m_pages.back();
}

Atlas::Atlas(int page_w, int page_h)
	: m_page_w(page_w)
	, m_page_h(page_h)
{
	new_page();
}

Atlas::~Atlas()
{ }

s_bmp Atlas::add(int id, int width, int height)
{
	try_page:
	opt<PageEntry> entry = last_page().add(width, height);
	if (!entry)
	{
		if (last_page().fresh)
			return nullptr;

		new_page();
		goto try_page;
	}

	auto&& entry_lock = entry.value();
	int page_num = int(m_pages.size());

	auto image = alsmart::create_sub_bitmap_shared(
		last_page().bmp.get(), entry_lock.x, entry_lock.y,
		entry_lock.w, entry_lock.h
	);

	m_entries.insert({id, {{page_num, entry_lock}, image}});

	return image;
}

s_bmp Atlas::get(int id)
{
	auto it = m_entries.find(id);

	if (it == m_entries.end())
		return nullptr;

	return it->second.second;
}

std::vector<int> Atlas::get_entry_list()
{
	std::vector<int> result;

	for (auto&& entry : m_entries)
		result.push_back(entry.first);

	return result;
}

void Atlas::upload(Engine& engine)
{
	cio::out << "Uploading " << m_pages.size() << " page(s)." << cio::endl;
	for (auto& page : m_pages)
		engine.queue_upload(page.bmp);
}
