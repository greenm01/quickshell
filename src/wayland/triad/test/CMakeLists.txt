function (qs_test name)
	add_executable(${name} ${ARGN})
	target_link_libraries(${name} PRIVATE Qt::Quick Qt::Network Qt::Test quickshell-triad quickshell-core quickshell-window quickshell-ui quickshell-io)
	add_test(NAME ${name} WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" COMMAND $<TARGET_FILE:${name}>)
endfunction()

qs_test(triad-ipc triad_ipc.cpp)

qs_test(triad-qml triad_qml.cpp)
target_link_libraries(
	triad-qml
	PRIVATE
	"$<LINK_GROUP:RESCAN,quickshell-triadplugin,quickshell-coreplugin,quickshell-triad,quickshell-core,quickshell-window,quickshell-ui,quickshell-io>"
)
target_compile_definitions(triad-qml PRIVATE QS_TEST_QML_IMPORT_PATH="${CMAKE_BINARY_DIR}/qml_modules")

qs_test(triad-live triad_live.cpp)
target_link_libraries(
	triad-live
	PRIVATE
	"$<LINK_GROUP:RESCAN,quickshell-triadplugin,quickshell-coreplugin,quickshell-triad,quickshell-core,quickshell-window,quickshell-ui,quickshell-io>"
)
target_compile_definitions(triad-live PRIVATE QS_TEST_QML_IMPORT_PATH="${CMAKE_BINARY_DIR}/qml_modules")
