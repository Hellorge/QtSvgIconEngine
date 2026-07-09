# Documentation image generator. Not installed; built only on request.
add_executable(make_docs make_docs.cpp)
target_link_libraries(make_docs PRIVATE SvgIconEngine::SvgIconEngine Qt6::Widgets)
