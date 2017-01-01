#ifndef ALSMART_NATIVE_DIALOG_HPP
#define ALSMART_NATIVE_DIALOG_HPP

#include "alsmart.hpp"

#include <allegro5/allegro_native_dialog.h>

namespace alsmart
{
	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_FILECHOOSER, filechooser, al_destroy_native_file_dialog)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_FILECHOOSER, create_native_file_dialog)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_TEXTLOG, textlog, al_close_native_text_log)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_TEXTLOG, open_native_text_log)

	ALSMART_DEFINE_DESTROY_CLASS(ALLEGRO_MENU, menu, al_destroy_menu)
	ALSMART_DEFINE_CONSTRUCTOR(ALLEGRO_MENU, create_menu)
}

#endif // ALSMART_NATIVE_DIALOG_HPP
