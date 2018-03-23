cmake_minimum_required(VERSION 3.8)
project(softbank_autologin)

set(CMAKE_CXX_STANDARD 14)

find_package(CURL REQUIRED)

include_directories(
	${CURL_INCLUDE_DIRS}
)

set(SOURCES
	main.cpp
	account.hpp
	account.cpp
	ssid.hpp
	utils.hpp
)

if (WIN32)
	list(APPEND SOURCES
		ssid_win32.cpp
	)
else ()
	list(APPEND SOURCES
		ssid_noimpl.cpp
	)
endif ()

add_executable(softbank_autologin ${SOURCES})

target_link_libraries(softbank_autologin
	${CURL_LIBRARY}
)

if (WIN32)
	target_link_libraries(softbank_autologin
		wlanapi
		ole32
	)
endif ()