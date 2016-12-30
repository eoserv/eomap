#ifndef EOMAP_ABOUTDIALOG_HPP
#define EOMAP_ABOUTDIALOG_HPP

class Engine;

class AboutDialog
{
	private:
		Engine& m_engine;

	public:
		AboutDialog(Engine& engine);

		bool do_gui();
};

#endif // EOMAP_ABOUTDIALOG_HPP
