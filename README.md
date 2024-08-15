Demo uses the beautiful open source [ionicons](https://ionic.io/ionicons)

Merge requests are welcome, I'll only work on or add functionality I deem required while working on other projects of mine.

> [!NOTE]
> The `CMakeLists.txt` bundles all `.svg` files present in provided directory in a resource file `resources.qrc` located inside your build folder.

### Usage
```
SvgIconEngine iconEngine(<path/to/svg/files/directory>, <options>);

SvgIcon icon = iconEngine.getIcon(<style>, <icon_name>, <options>);
// to get an icon

iconEngine.setDefaults(<options>);
// set new default options

iconEngine.setCachePolicy(CachePolicy::<policy>);
// {ALL, NONE, LRU}
// ALL: cache all, NONE: no caching, LRU: 100 QSvgRenderer instances will be cache at once

iconEngine.clearCache();
// clear QSvgRenderer cache, without cache QSvgRenderer instances are generated from svg paths for each time an icon is requested.
```

### Arguments
1. `SvgIconEngine::SvgIconEngine` SvgIconEngine constructor
	  - `path/to/svg/files/directory` | QString | Required

    	Folders path where icons are located,

    - `options` | QVariantMap

    	Set default options for all new icons

    - `policy` | CachePolicy

    	Set cache policy for SvgIconEngine, possible values are: `ALL`, `NONE`, `LRU`

2. `SvgIconEngine::getIcon` get an icon
	  - `style` | QString | Required

    	Folders path between `<path/to/svg/files/directory>` and svg file,

    - `icon_name` | QString | Required

    	Svg filename without extension

    - `options` | QVariantMap

   		Set options for required icon

### `<options>`
| QVariant       | Value Type | Default Value                          | Ex. Value         | Animates? | Use |
| -------------- | ---------- | -------------------------------------- | ----------------- | --------- | --- |
| color          | QColor     | QApplication::palette().text().color() | QColor(Qt::red)   | Yes       | Color used to fill icon |
| background     | QColor     | Qt::transparent                        | QColor(Qt::red)   | Yes       | Background fill for icons |
| size           | QSize      | QSvgRenderer.defaultSize()             | QSize(64, 64)     | Yes       | Icon size |
| scale          | qreal      | 1                                      | 2                 | Yes       | Image size inside icon box |
| opacity        | qreal      | 1                                      | .5                | Yes       | Icon opacity |
| border_color   | QColor     | QApplication::palette().text().color() | QColor(Qt::black) | Will      | Color for border around icon |
| border_width   | qreal      | 0                                      | 10                | Will      | Border width |
| default_colors | bool       | false                                  | true/false        | No        | If icon colors remain as in svg file |

### Example
An svg file at `/home/user/pictures/svgs/regular/calendar.svg`

```
path/to/svg/files/directory = "/home/user/pictures/svgs"
style = "regular"
icon_name = "calendar"
```


## License

This project is licensed under the terms of the GNU Lesser General Public License. See the [LICENSE](./LICENSE) file for details.
