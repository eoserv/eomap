
find_path(ASIO_INCLUDE_DIR
	NAMES asio.hpp
)

if(ASIO_INCLUDE_DIR)
	file(STRINGS "${ASIO_INCLUDE_DIR}/asio/version.hpp" _asio_VERSION_HPP_CONTENTS REGEX "^[ \t]*#[ \t]*define[ \t]+ASIO_VERSION[ \t]")
	if(${_asio_VERSION_HPP_CONTENTS} MATCHES "^[ \t]*#[ \t]*define[ \t]+ASIO_VERSION[ \t]+([0-9]+)")
		math(EXPR _asio_MAJOR_VERSION "${CMAKE_MATCH_1} / 100000")
		math(EXPR _asio_MINOR_VERSION "${CMAKE_MATCH_1} / 100 % 1000")
		math(EXPR _asio_PATCH_VERSION "${CMAKE_MATCH_1} % 100")
		set(ASIO_VERSION_STRING "${_asio_MAJOR_VERSION}.${_asio_MINOR_VERSION}.${_asio_PATCH_VERSION}")
	endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
	Asio FOUND_VAR ASIO_FOUND
	REQUIRED_VARS ASIO_INCLUDE_DIR
	VERSION_VAR ASIO_VERSION_STRING
)

mark_as_advanced(
	ASIO_INCLUDE_DIR
)
