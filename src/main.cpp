#include "alsmart/alsmart.hpp"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_primitives.h>

#include "engine.hpp"

int main(int argc, char *argv[])
{
	al_init();
	al_init_image_addon();
	al_init_native_dialog_addon();
	al_init_primitives_addon();

	al_install_keyboard();
	al_install_mouse();

	(void)argc;
	(void)argv;

	Engine engine;
	engine.run();

	return 0;
}
