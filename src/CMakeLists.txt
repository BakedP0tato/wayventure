file(GLOB MY_FILES res/*.txt res/*.png res/help/*.txt)
file(COPY ${MY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(wayventure 
	main.cc
	classes.cc
	main_dungeon.cc
	inventory_mode.cc
	draw.cc
	generate.cc
	town.cc
	help_mode.cc
	inventory_modifier_mode.cc
	threads.cc
	town_additional.cc
)

target_link_libraries(wayventure -ltcod -lSDL2 -lpthread -lz -ldl)