add_executable(texed
	texed.c
	platform.c
)

include_directories(.)
target_link_libraries(texed raylib cwpack)

link_system_libs(texed)
