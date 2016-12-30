TEMPLATE = app

CONFIG += c++14 \
	thread \
	object_parallel_to_source \
	link_pkgconfig

CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    src/pub_structs.cpp \
    src/eo_stream.cpp \
    src/eo_data.cpp \
    src/cio/cio.cpp \
    src/draw_buffer.cpp \
    src/draw_renderer.cpp \
    src/gfx_manager.cpp \
    src/pe_reader.cpp \
    src/dib_reader.cpp \
    src/engine.cpp \
    src/engine_drawing_thread.cpp \
    src/main.cpp \
    src/third-party/imgui/imgui_demo.cpp \
    src/third-party/imgui/imgui_draw.cpp \
    src/third-party/imgui/imgui.cpp \
    src/third-party/imgui/imgui_impl_a5.cpp \
    src/atlas.cpp \
    src/gui.cpp \
    src/map_renderer.cpp \
    src/dialog/newmapdialog.cpp \
    src/dialog/aboutdialog.cpp

PKGCONFIG += allegro-5 \
	allegro_image-5 \
	allegro_dialog-5 \
	allegro_primitives-5

HEADERS += \
    src/common.hpp \
    src/eo_types.hpp \
    src/eo_stream.hpp \
    src/pub_structs.hpp \
    src/pub_enums.hpp \
    src/eo_data.hpp \
    src/cio/cio.hpp \
    src/draw_buffer.hpp \
    src/draw_renderer.hpp \
    src/gfx_manager.hpp \
    src/third-party/variant.h \
    src/alsmart/alsmart.hpp \
    src/common_alsmart.hpp \
    src/pe_reader.hpp \
    src/dib_reader.hpp \
    src/int_pack.hpp \
    src/engine.hpp \
    src/engine_drawing_thread.hpp \
    src/common_signal.hpp \
    src/third-party/imgui/imconfig.h \
    src/third-party/imgui/imgui_internal.h \
    src/third-party/imgui/imgui.h \
    src/third-party/imgui/stb_rect_pack.h \
    src/third-party/imgui/stb_textedit.h \
    src/third-party/imgui/stb_truetype.h \
    src/third-party/imgui/imgui_impl_a5.h \
    src/alsmart/native_dialog.hpp \
    src/atlas.hpp \
    src/gui.hpp \
    src/map_renderer.hpp \
    src/dialog/newmapdialog.hpp \
    src/dialog/aboutdialog.hpp
