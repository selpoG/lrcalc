macro(fix_default_cxx_flags)
	if(MSVC)
		string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.14)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /experimental:external /external:W0")
		endif()
		if(NOT BUILD_SHARED_LIBS)
			string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
			string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
		endif()
	endif()
endmacro()

macro(set_flags)
	if(MSVC)
		set(cxx_flags /W4 /wd4068 /permissive- /MP /Zc:rvalueCast /sdl)
		set(cxx_flags ${cxx_flags} $<$<CONFIG:Release>:/Gy> $<$<CONFIG:Release>:/Oi>)
	else()
		set(cxx_flags -Wall -Wextra -Wpedantic -Wno-padded -pthread -fPIC)
		set(cxx_flags ${cxx_flags} $<$<CONFIG:Debug>:-O0> $<$<CONFIG:Debug>:-g3>)
		if(CMAKE_COMPILER_IS_GNUCXX)
			set(cxx_flags ${cxx_flags} -Weffc++ -Wno-noexcept -Walloca -Wcast-align -Wcast-qual -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wdate-time -Wdisabled-optimization -Wdouble-promotion -Wduplicated-cond -Wfloat-conversion -Wfloat-equal -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wmultichar -Wmultiple-inheritance -Wnull-dereference -Wold-style-cast -Woverlength-strings -Woverloaded-virtual -Wpacked -Wredundant-decls -Wrestrict -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wsuggest-override -Wswitch-default -Wswitch-enum -Wuninitialized -Wunused-macros -Wunused-parameter -Wuseless-cast -Wvariadic-macros -Wvector-operation-performance -Wvirtual-inheritance)
		else()
			set(cxx_flags ${cxx_flags} -stdlib=libc++)
			if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 9.0.0)
				set(cxx_flags ${cxx_flags} -Wno-ctad-maybe-unsupported)
			endif()
			set(cxx_flags ${cxx_flags} -Weverything -Wno-zero-as-null-pointer-constant -Wno-c++98-compat -Wno-shadow-field-in-constructor -Wno-c++98-compat-pedantic -Wno-global-constructors -Wno-exit-time-destructors -Wno-covered-switch-default)
		endif()
	endif()
endmacro()

function(set_default_property target dir)
	set_target_properties(
		${target} PROPERTIES
		COMPILE_OPTIONS "${cxx_flags}"
		CXX_EXTENSIONS OFF
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON
	)
endfunction()