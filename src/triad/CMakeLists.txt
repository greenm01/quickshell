qt_add_library(quickshell-triad STATIC
	connection.cpp
	output.cpp
	qml.cpp
	window.cpp
	workspace.cpp
)

qt_add_qml_module(quickshell-triad
	URI Quickshell.Triad
	VERSION 0.1
	DEPENDENCIES QtQml
)

qs_add_module_deps_light(quickshell-triad Quickshell)

install_qml_module(quickshell-triad)

target_link_libraries(quickshell-triad PRIVATE Qt::Quick Qt::Network quickshell-core)

qs_module_pch(quickshell-triad SET large)

target_link_libraries(quickshell PRIVATE quickshell-triadplugin)

if (BUILD_TESTING)
	add_subdirectory(test)
endif()
