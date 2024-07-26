Demo uses the beautiful open source [ionicons](https://ionic.io/ionicons)

Merge requests are welcome, I'll only work on or add functionality I deem required while working on other projects of mine.

> [!NOTE]
> The `CMakeLists.txt` bundles all `.svg` files present in provided directory in a resource file `resources.qrc` located inside your build folder.

### Usage
```
SvgIconEngine iconEngine(<path/to/svg/files/directory>);
SvgIcon icon = iconEngine.getIcon(<style>, <icon_name>, <options>);
```

### Arguments

1. `style` | QString

    Folders path between `<path/to/svg/files/directory>` and svg file,

3. `icon_name` | QString

   Svg filename without extension

5. `options` | QVariantMap

    | QVariant  |  Value Type  | Default Value |    Ex. Value    | Use |
    | ------------------ | ------ | ------------- | --------------- | --- |
    | color              | QColor | QApplication::palette().text().color() | QColor(Qt::red) | Color used to fill icon |
    | default_colors     |  bool  | false | true/false | If icon colors remain as in svg file |

### Example
An svg file at `/home/user/pictures/svgs/regular/calendar.svg`

```
path/to/svg/files/directory = "/home/user/pictures/svgs"
style = "regular"
icon_name = "calendar"
```
