Demo uses the beautiful open source [ionicons](https://ionic.io/ionicons)

Merge requests are welcome, I'll only work on or add functionality I deem required while working on other projects of mine.

> [!NOTE]
> The `CMakeLists.txt` bundles all `.svg` files present in provided directory in a resource file `resources.qrc` located inside your build folder.

### Usage
```
SvgIconEngine iconEngine(<path/to/svg/files/directory>, <options>);
SvgIcon icon = iconEngine.getIcon(<style>, <icon_name>, <options>);
```

### Arguments
1. `SvgIconEngine::SvgIconEngine` SvgIconEngine constructor
	  - `path/to/svg/files/directory` | QString | Required

    	Folders path where icons are located,

    - `options` | QVariantMap

    	Set default options for all new icons

2. `SvgIconEngine::getIcon` get an icon
	  - `style` | QString | Required

    	Folders path between `<path/to/svg/files/directory>` and svg file,

    - `icon_name` | QString | Required

    	Svg filename without extension

    - `options` | QVariantMap

   		Set options for required icon

### `<options>`
|    QVariant    |  Value Type  | Default Value |    Ex. Value    | Use |
| -------------- | ------------ | ------------- | --------------- | --- |
| color          |    QColor    | QApplication::palette().text().color() | QColor(Qt::red) | Color used to fill icon |
| default_colors |     bool     | false | true/false | If icon colors remain as in svg file |

### Example
An svg file at `/home/user/pictures/svgs/regular/calendar.svg`

```
path/to/svg/files/directory = "/home/user/pictures/svgs"
style = "regular"
icon_name = "calendar"
```
