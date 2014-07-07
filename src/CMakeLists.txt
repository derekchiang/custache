aux_source_directory(. C_SOURCES)

add_library(custache SHARED ${C_SOURCES})

add_library(custache_static STATIC ${C_SOURCES})
set_target_properties(custache_static PROPERTIES OUTPUT_NAME custache)

install(TARGETS custache DESTINATION lib)
install(TARGETS custache_static DESTINATION lib)
