
graphic_files = [
  'babl-16x16',
  'babl-48x48',
  'babl-a4poster',
]

graphic_files_install = [
  files('index.html'),
]

if rsvg_convert_bin.found()
  foreach file : graphic_files
    sizeinfo = (
      file.contains('a4poster')
      ? [ '-w', '256', ]
      : []
    )

    graphic_files_install += custom_target(file,
      input : [ file + '.svg' ],
      output: [ file + '.png' ],
      command: [
        rsvg_convert_bin,
        sizeinfo,
        '-o', '@OUTPUT@',
        '@INPUT@',
      ],
      install: false,
    )
  endforeach
endif
