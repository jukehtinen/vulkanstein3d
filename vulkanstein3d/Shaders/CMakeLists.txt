cmake_minimum_required (VERSION 3.8)

# Compile shaders to spir-v
add_custom_target(shaders-target ALL)

find_program(GLSLC glslc)
file(GLOB frags ${CMAKE_CURRENT_SOURCE_DIR}/*.frag)
file(GLOB verts ${CMAKE_CURRENT_SOURCE_DIR}/*.vert)
list(APPEND shaders ${frags})
list(APPEND shaders ${verts})

foreach(shader ${shaders})
    get_filename_component(filename ${shader} NAME)
    set(current-output-path ${CMAKE_CURRENT_BINARY_DIR}/${filename}.spv)
    
    add_custom_command(
		OUTPUT ${current-output-path}
		COMMAND ${GLSLC} -O0 -o ${current-output-path} ${shader}
		DEPENDS ${shader}
		IMPLICIT_DEPENDS CXX ${shader}
		VERBATIM)

    set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
    set_property(TARGET shaders-target APPEND PROPERTY SOURCES ${current-output-path})
endforeach(shader)

# Generate reflection json
add_custom_target(shaders-json-target ALL DEPENDS shaders-target)
find_program(SPRIVCROSS spirv-cross)
file(GLOB bins ${CMAKE_CURRENT_BINARY_DIR}/*.spv)

foreach(spv ${bins})
	get_filename_component(filename ${spv} NAME)
	set(current-output-path-json ${CMAKE_CURRENT_BINARY_DIR}/${filename}.json)
	add_custom_command(
		OUTPUT  ${current-output-path-json}
		COMMAND ${SPRIVCROSS} ${filename} --reflect --output ${current-output-path-json}
		DEPENDS ${shader}
		IMPLICIT_DEPENDS CXX ${spv}
		VERBATIM)
	set_source_files_properties(${current-output-path-json} PROPERTIES GENERATED TRUE)
    set_property(TARGET shaders-json-target APPEND PROPERTY SOURCES ${current-output-path-json})
endforeach(spv)