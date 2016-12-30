#ifndef EOMAP_NEWMAPDIALOG_HPP
#define EOMAP_NEWMAPDIALOG_HPP

class Engine;

class NewMapDialog
{
	private:
		Engine& m_engine;

	public:
		char m_name[24] = {};
		int m_width = 0;
		int m_height = 0;

		NewMapDialog(Engine& engine);

		bool do_gui();
};

#endif // EOMAP_NEWMAPDIALOG_HPP
