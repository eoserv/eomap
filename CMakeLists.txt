cmake_minimum_required(VERSION 3.0)
project(eomap-ng)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------

add_executable(eomap-ng
	src/atlas.cpp
	src/dialog/aboutdialog.cpp
	src/dialog/newmapdialog.cpp
	src/dib_reader.cpp
	src/draw_buffer.cpp
	src/draw_renderer.cpp
	src/engine.cpp
	src/engine_drawing_thread.cpp
	src/eo_data.cpp
	src/eo_stream.cpp
	src/gfx_manager.cpp
	src/gui.cpp
	src/main.cpp
	src/map_renderer.cpp
	src/palette_renderer.cpp
	src/pe_reader.cpp
	src/pub_structs.cpp
	src/third-party/imgui/imgui.cpp
	src/third-party/imgui/imgui_demo.cpp
	src/third-party/imgui/imgui_draw.cpp
	src/third-party/imgui/imgui_impl_a5.cpp
	src/util/cio/cio.cpp

	src/atlas.hpp
	src/common.hpp
	src/common_alsmart.hpp
	src/common_signal.hpp
	src/dialog/aboutdialog.hpp
	src/dialog/newmapdialog.hpp
	src/dib_reader.hpp
	src/draw_buffer.hpp
	src/draw_renderer.hpp
	src/engine.hpp
	src/engine_drawing_thread.hpp
	src/eo_data.hpp
	src/eo_stream.hpp
	src/eo_types.hpp
	src/gfx_manager.hpp
	src/gui.hpp
	src/map_renderer.hpp
	src/palette_renderer.hpp
	src/pe_reader.hpp
	src/pub_enums.hpp
	src/pub_structs.hpp
	src/third-party/imgui/imconfig.h
	src/third-party/imgui/imgui.h
	src/third-party/imgui/imgui_impl_a5.h
	src/third-party/imgui/imgui_internal.h
	src/third-party/imgui/stb_rect_pack.h
	src/third-party/imgui/stb_textedit.h
	src/third-party/imgui/stb_truetype.h
	src/third-party/variant.h
	src/util/alsmart/alsmart.hpp
	src/util/alsmart/native_dialog.hpp
	src/util/cio/cio.hpp
	src/util/int_pack.hpp
	src/util/signal.hpp
)

install(TARGETS eomap-ng
	RUNTIME DESTINATION bin
)

# -----------

# pkg-config not generally available on Windows
#find_package(PkgConfig)
#pkg_check_modules(ALLEGRO5 REQUIRED allegro-5)
#pkg_check_modules(ALLEGRO5_DIALOG REQUIRED allegro_dialog-5)
#pkg_check_modules(ALLEGRO5_IMAGE REQUIRED allegro_image-5)
#pkg_check_modules(ALLEGRO5_PRIMITIVES REQUIRED allegro_primitives-5)

find_package(Threads REQUIRED)
find_package(Allegro5 REQUIRED)
find_package(Asio 1.10 EXACT REQUIRED)

target_include_directories(eomap-ng SYSTEM PRIVATE ${ALLEGRO5_INCLUDE_DIRS} ${ASIO_INCLUDE_DIR})
target_link_libraries(eomap-ng PRIVATE Threads::Threads ${ALLEGRO5_LIBRARY} ${ALLEGRO5_DIALOG_LIBRARY} ${ALLEGRO5_IMAGE_LIBRARY} ${ALLEGRO5_PRIMITIVES_LIBRARY})
target_compile_definitions(eomap-ng PRIVATE ASIO_STANDALONE)
