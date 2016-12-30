#include "draw_buffer.hpp"

#include "cio/cio.hpp"

Draw_Buffer::Draw_Buffer()
{
	m_cmd_buffer.reserve(1024);
}

Draw_Buffer::~Draw_Buffer()
{ }

void Draw_Buffer::add(const Draw_Command& cmd)
{
	m_cmd_buffer.push_back(cmd);
}
