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
| QVariant       | Value Type | Default Value                          | Ex. Value         | Use |
| -------------- | ---------- | -------------------------------------- | ----------------- | --- |
| color          | QColor     | QApplication::palette().text().color() | QColor(Qt::red)   | Color used to fill icon |
| size           | QSize      | QSvgRenderer.defaultSize()             | QSize(64, 64)     | Icon size |
| default_colors | bool       | false                                  | true/false        | If icon colors remain as in svg file |
| background     | QColor     | Qt::transparent                        | QColor(Qt::red)   | Background fill for icons |
| opacity        | int        | 1                                      | QColor(Qt::red)   | Icon opacity |
| border_color   | QColor     | QApplication::palette().text().color() | QColor(Qt::black) | Color for border around icon |
| border_width   | int	      | 0                                      | 10                | Border width |

### Example
An svg file at `/home/user/pictures/svgs/regular/calendar.svg`

```
path/to/svg/files/directory = "/home/user/pictures/svgs"
style = "regular"
icon_name = "calendar"
```


## License

This project is licensed under the terms of the GNU Lesser General Public License. See the [LICENSE](./LICENSE) file for details.
