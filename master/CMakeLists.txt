
set(SOURCES "master.cpp" "master.h" "main.cpp")
add_executable(master ${SOURCES})
target_link_libraries(master soem)
set_property(TARGET master PROPERTY C_STANDARD 11)
install(TARGETS master DESTINATION bin)