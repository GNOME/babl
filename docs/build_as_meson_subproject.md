
Including babl as a meson subproject in your project:

You create a babl.wrap file inside a folder named 'subprojects'
which contains a way to download the babl source.
See https://mesonbuild.com/Wrap-dependency-system-manual.html.
An example will be:
```
[wrap-git]
url = https://gitlab.gnome.org/GNOME/babl
revision = master
depth = 1
```

Next, include in your meson.build file something like this:
```
babl = dependency('babl', fallback: ['babl', 'libbabl_dep'])
```

If babl is installed in your system, meson will use that one,
otherwise it will download and build babl.


