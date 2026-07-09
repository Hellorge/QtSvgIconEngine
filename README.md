# QtSvgIconEngine

Tinted, state-aware, animated SVG icons for Qt — as widgets, or as a real `QIcon`.

Qt's stock SVG icons are static bitmaps once rendered: you cannot recolour them from the
palette, give a hover state its own artwork, or animate between states. This library does
all three, and exposes the result both as a `QWidget` (animatable) and as a `QIconEngine`
(usable anywhere Qt takes a `QIcon` — `QAction`, `QPushButton::setIcon`, item views).

| Hover: artwork swaps, colour interpolates | Press: squash + darken | Theme: icons follow `QPalette::Text` |
|:---:|:---:|:---:|
| ![hover](docs/hover.gif) | ![press](docs/press.gif) | ![theme](docs/theme.gif) |

![draw](docs/draw.gif)

*`stroke_progress` writes the icon on, the way `stroke-dashoffset` does on the web.*

The clips are recorded from synthesized mouse events on a real `SvgIconButton`, not by
calling `setState()` — so they show the whole chain, cursor included. The same events drive
`tests/tst_interaction.cpp`.

![showcase](docs/showcase.png)

---

## Quick start

```cpp
#include <SvgIconEngine/SvgIconEngine.h>
#include <SvgIconEngine/SvgIconButton.h>

// The icon bank: a filesystem directory or a qrc prefix.
SvgIconEngine icons(":/icons");

// A QIcon — works with any Qt widget.
QPushButton *save = new QPushButton(icons.icon("regular/save"), "Save");

// A widget — animates between states.
QVariantMap opts;
opts["size"]        = QSize(24, 24);
opts["hover_icon"]  = "solid/heart";      // fills in on hover
opts["hover_color"] = QColor("#f43f5e");
auto *like = new SvgIconButton(icons.getIcon("regular/heart", opts), "Like");
```

No colour was specified, so both icons inherit `QPalette::Text` and re-tint themselves when
the application theme changes.

## Addressing icons

Icons are addressed by path, the way an `<img src>` is. The `.svg` suffix is optional.

```cpp
icons.icon("regular/heart");        // <root>/regular/heart.svg
icons.icon("regular/heart.svg");    // the same
icons.icon(":/other/logo.svg");     // qrc, ignoring the bank
icons.icon("/usr/share/x.svg");     // filesystem, ignoring the bank
```

Inside a per-state option the reference is **relative**, again like HTML: a bare name is a
sibling of the base icon, a name with a slash is resolved against the bank root.

```cpp
opts["hover_icon"] = "heart";        // the heart next to me  -> regular/heart.svg
opts["hover_icon"] = "solid/heart";  // across the bank       -> solid/heart.svg
```

## States

`Normal`, `Hovered`, `Pressed`, `Selected`, `Disabled`. Every visual option takes a
per-state override by prefix. Anything you don't override is *derived* from the Normal
value, so a theme change carries through automatically.

```cpp
QVariantMap o;
o["color"]           = QColor("#94a3b8");   // omit to inherit QPalette::Text
o["hover_color"]     = QColor("#38bdf8");
o["pressed_scale"]   = 0.85;                // tactile squash
o["selected_icon"]   = "solid/star";        // a different SVG when toggled on
o["transition_ms"]   = 180;                 // 0 = instant
```

`SvgIconButton` drives the state for you (`disabled > pressed > hovered > checked`), or you
can drive it directly with `icon->setState(SvgIcon::Hovered)`.

### Options

| Key | Type | Default |
|---|---|---|
| `size` | `QSize` | the SVG's own `defaultSize()` |
| `color` | `QColor` | **inherits `QPalette::Text`** |
| `background` | `QColor` | transparent |
| `opacity` | `qreal` | `1.0` |
| `scale` | `qreal` | `1.0` |
| `border_color` | `QColor` | inherits `QPalette::Text` |
| `border_width` | `qreal` | `0.0` |
| `default_colors` | `bool` | `false` — keep the file's own colours, no tinting |
| `transition_ms` | `int` | `150` |
| `stroke_progress` | `qreal` | `1.0` — `0` undrawn, `1` fully drawn ([stroke effects](#stroke-effects)) |
| `dash_pattern` | `qreal` | `0.0` — dash length in SVG user units; `0` disables |
| `dash_offset` | `qreal` | `0.0` — animate for marching ants |

Prefix any of the visual keys with `hover_`, `pressed_`, `selected_` or `disabled_` to
override it for that state. `hover_icon`, `pressed_icon`, `selected_icon` and
`disabled_icon` take a **path** and give the state its own artwork.

When a state has no explicit override, it is derived. These knobs tune the derivation:

| Key | Default | Effect |
|---|---|---|
| `hover_lighten` | `130` | `QColor::lighter()` factor for `Hovered` |
| `pressed_darken` | `115` | `QColor::darker()` factor for `Pressed` |
| `disabled_opacity_factor` | `0.5` | opacity multiplier for `Disabled` |
| `selected_wash_alpha` | `60` | alpha of the `QPalette::Highlight` wash behind `Selected` |

## Stroke effects

Beyond colour and geometry, the stroke itself is animatable. `stroke_progress` goes from
`0` (undrawn) to `1` (fully drawn).

```cpp
#include <QPropertyAnimation>

QVariantMap o;
o["size"]            = QSize(48, 48);
o["stroke_progress"] = 0.0;                  // start undrawn
SvgIcon *icon = icons.getIcon("regular/star", o);

auto *draw = new QPropertyAnimation(icon, "stroke_progress", icon);
draw->setDuration(800);
draw->setStartValue(0.0);
draw->setEndValue(1.0);
draw->setEasingCurve(QEasingCurve::InOutCubic);
draw->start();                               // the star writes itself on
```

It is a `Q_PROPERTY`, so it also works through `animateTo()` and takes per-state overrides
like every other key — here the icon sits half-drawn and completes when the pointer arrives:

```cpp
QVariantMap o;
o["stroke_progress"]       = 0.35;           // partially drawn at rest
o["hover_stroke_progress"] = 1.0;            // completes on hover
o["transition_ms"]         = 400;            // over 400ms
auto *btn = new SvgIconButton(icons.getIcon("regular/heart", o), "Like");
```

Marching ants come from the same primitive — a fixed dash, an animated offset:

```cpp
QVariantMap o;
o["size"]         = QSize(48, 48);
o["dash_pattern"] = 36.0;                    // dash length, in SVG user units
SvgIcon *icon = icons.getIcon("regular/cloud", o);

auto *ants = new QPropertyAnimation(icon, "dash_offset", icon);
ants->setDuration(1400);
ants->setStartValue(0.0);
ants->setEndValue(72.0);                     // one full dash + gap period
ants->setLoopCount(-1);
ants->start();
```

Both `stroke_progress` and `dash_offset` are valid keys for `animateTo()`, alongside
`color`, `background`, `opacity`, `scale`, `border_color`, `border_width` and `size`.

**How it works, and what that costs.** There is no SVG path parser here. `stroke-dasharray`
and `stroke-dashoffset` inherit, so the library injects them once on the root `<svg>`
element and lets Qt do the rest. An icon's path length — the dash that exactly hides it —
is recovered by *bisecting on rendered coverage*: the smallest dash that draws nothing is
the longest subpath. That measurement costs ~6 ms and is memoised per file.

Each animated frame re-renders the SVG with a new offset: **~167 µs**, about 1% of a 60 fps
budget for one icon. Fine for a handful; don't animate a hundred at once. Icons that use no
stroke effect never pay any of this.

**Limitation:** a *filled* icon has no stroke to draw. `solid/heart` reports a stroke length
of `0` and ignores the effect (with a warning) rather than rendering blank. Query it with
`icons.strokeLength("regular/star")` or `icon->strokeLength()`.

## Widget or QIcon?

```cpp
SvgIcon *w = icons.getIcon("regular/heart", opts);   // a QWidget
QIcon    i = icons.icon("regular/heart", opts);      // a QIcon
```

`QIcon::Mode` maps onto the state model:

| `QIcon::Mode` | `SvgIcon::State` |
|---|---|
| `Normal` | `Normal` |
| `Active` | `Hovered` |
| `Selected` | `Selected` |
| `Disabled` | `Disabled` |

`QIcon` has **no pressed mode and cannot animate** — it is a pixmap factory, not a live
object. Use the widget when you need either. Everything else (tinting, per-state artwork,
palette inheritance, sprites, HiDPI) works identically through both.

## Sprites

Render one element of a sprite sheet, by `id`. The element is tinted, animated and
state-aware exactly like a whole file.

```cpp
SvgIcon *w = icons.getIconFromSprite("sheets/toolbar.svg", "cut");
QIcon    i = icons.iconFromSprite("sheets/toolbar.svg", "paste");
```

## Caching

Parsing an SVG is by far the expensive part, so renderers are held in an LRU cache and
shared between every icon that uses the same file. Measured on this machine:

| | cost |
|---|---|
| construct a `QSvgRenderer` (parse the file) | **21.3 µs** |
| fetch the cached one | **3 ns** |

That is roughly a 7000× difference per lookup, which is why the cache exists rather than
being ceremony. A toolbar rebuilt on every theme change would otherwise re-parse every icon.

```cpp
icons.setCacheLimit(200);   // renderers to keep; 0 disables caching
icons.clearCache();
```

Eviction is safe: the cache holds a *reference*, so a renderer still in use by a live icon
stays alive until the last icon using it goes away.

## HiDPI

Both paths rasterise at the screen's `devicePixelRatio` and re-rasterise when the window
moves to a display with a different scale factor. The test suite runs every scale-sensitive
case at `QT_SCALE_FACTOR=2` as well as `1`.

## Building

Requires Qt 6 (`Widgets`, `SvgWidgets`) and CMake ≥ 3.16.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build      # 13 suites, incl. HiDPI re-runs
./build/test/SvgIconEngineTest   # the showcase above
```

### Using it

With an installed package:

```sh
cmake --install build --prefix /usr/local
```

```cmake
find_package(SvgIconEngine 0.2 REQUIRED)
target_link_libraries(myapp PRIVATE SvgIconEngine::SvgIconEngine)
```

Or vendored, as a subdirectory:

```cmake
add_subdirectory(third_party/QtSvgIconEngine)
target_link_libraries(myapp PRIVATE SvgIconEngine::SvgIconEngine)
```

`BUILD_SHARED_LIBS=ON` builds a shared library. `SVGICONENGINE_BUILD_TESTS` and
`SVGICONENGINE_BUILD_DEMO` default to `ON` only when this is the top-level project.

### Regenerating the images above

```sh
cmake -S . -B build -DSVGICONENGINE_BUILD_TOOLS=ON
cmake --build build --target make_docs SvgIconEngineTest
./tools/make_docs.sh build     # needs ffmpeg
```

## Notes

The demo uses the beautiful open-source [ionicons](https://ionic.io/ionicons).

Merge requests are welcome; I'll only work on or add functionality I deem required while
working on other projects of mine.

## License

GNU Lesser General Public License. See [LICENSE](./LICENSE).
