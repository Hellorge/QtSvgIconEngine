# Every test is a plain executable returning the number of failed assertions, so
# no test framework is needed. Each takes the icon root as argv[1].
#
# Scale-sensitive suites are re-run at devicePixelRatio 2: rendering faults at 2x
# do not show at 1x.

set(SVG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../test/svgs)

set(TESTS
    tst_icon                # lifetime, palette, per-state options, animation
    tst_qicon               # QIconEngine: modes, per-state artwork, HiDPI
    tst_paths               # <img src>-style path resolution, sprites
    tst_theme               # palette-follow, stylesheet regeneration
    tst_button_layout       # sizeHint never clips its content
    tst_button_background   # the icon must not paint a window-brush card
    tst_hidpi               # widget rasterises at the screen's dpr
    tst_interaction         # synthesized mouse events actually reach the icon
    tst_stroke              # stroke-dashoffset draw-on, marching ants
    tst_readme              # every README code block compiles and behaves
)

foreach(t ${TESTS})
    add_executable(${t} ${t}.cpp)
    target_link_libraries(${t} PRIVATE SvgIconEngine::SvgIconEngine Qt6::Widgets)

    add_test(NAME ${t} COMMAND ${t} ${SVG_ROOT})
    set_tests_properties(${t} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endforeach()

# Re-run the scale-sensitive suites on a 2x display.
foreach(t tst_qicon tst_hidpi tst_paths)
    add_test(NAME ${t}_hidpi COMMAND ${t} ${SVG_ROOT})
    set_tests_properties(${t}_hidpi PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_SCALE_FACTOR=2")
endforeach()
