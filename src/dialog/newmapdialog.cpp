#include "newmapdialog.hpp"

#include "../engine.hpp"

#include "../third-party/imgui/imgui.h"

NewMapDialog::NewMapDialog(Engine& engine)
	: m_engine(engine)
{

}

bool NewMapDialog::do_gui()
{
	bool mutated = false;
	bool open = true;

	if (ImGui::Begin("New Map", &open))
	{
		bool is_ok = true;
		ImGui::InputText("Name", m_name, 24);
		ImGui::InputInt("Width", &m_width, 0);
		ImGui::InputInt("Height", &m_height, 0);

		if (m_width  <   0) { m_width  = 0;   mutated = true; }
		if (m_height <   0) { m_height = 0;   mutated = true; }
		if (m_width  > 254) { m_width  = 254; mutated = true; }
		if (m_height > 254) { m_height = 254; mutated = true; }

		if (m_width == 0 || m_height == 0
		 || (m_width > 252 && m_width != 254)
		 || (m_height > 252 && m_height != 254))
		{
			ImGui::TextColored(ImVec4(1.f, 0.5f, 0.5f, 1.f), "Error: Invalid map size!");
			is_ok = false;
		}
		else
		{
			ImGui::Text("");
		}

		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(80.f, 0.f)))
		{
			m_engine.do_new();
			open = false;
		}

		ImGui::SameLine(0.0, 20.0);

		if (ImGui::Button("Cancel", ImVec2(80.f, 0.f)))
		{
			open = false;
		}
	}
	ImGui::End();

	if (!open)
	{
		m_engine.m_new_map_dialog = nullopt;
		mutated = true;
	}

	return mutated;
}
