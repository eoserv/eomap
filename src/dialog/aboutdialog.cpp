#include "aboutdialog.hpp"

#include "../engine.hpp"
#include "../third-party/imgui/imgui.h"

AboutDialog::AboutDialog(Engine& engine)
	: m_engine(engine)
{

}

bool AboutDialog::do_gui()
{
	bool mutated = false;
	bool open = true;

	auto flags = ImGuiWindowFlags_NoResize
	           | ImGuiWindowFlags_NoTitleBar
	           | ImGuiWindowFlags_NoSavedSettings;

	ImGui::SetNextWindowPosCenter();
	ImGui::OpenPopup("About");

	if (ImGui::BeginPopupModal("About", &open, flags))
	{
		//ImGui::Columns(2, "AboutTop", false);
		// TODO: Support images on the GUI
		//ImGui::NextColumn();
		ImGui::Text("EOMap-ng - http://eoserv.net/");
		ImGui::Text("Version 1.0-dev (" __DATE__ ")");
		//ImGui::Columns();
		ImGui::Separator();
		ImGui::Text("Made by Sausage\n<sausage@tehsausage.com>");
		ImGui::Separator();

		ImGui::SetNextWindowPosCenter();
		ImGui::Spacing();
		ImGui::Columns(3, nullptr, false);
		ImGui::NextColumn();

		if (ImGui::Button("Cool!"))
			open = false;

		ImGui::Columns(1);
		ImGui::EndPopup();
	}

	if (!open)
	{
		m_engine.m_about_dialog = nullopt;
		mutated = true;
	}

	return mutated;
}
