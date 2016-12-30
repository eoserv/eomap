#ifndef EOMAP_GUI_HPP
#define EOMAP_GUI_HPP

class Draw_Buffer;
class Engine;

class GUI
{
	private:
		Engine& m_engine;

	public:
		GUI(Engine& engine);
		~GUI();

		void resize(int w, int h);

		void draw(Draw_Buffer& draw, int x, int y);
};

#endif // EOMAP_GUI_HPP
