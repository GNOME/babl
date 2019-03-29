
graphic_files = [
  'babl-16x16.svg',
  'babl-48x48.svg',
  'babl-a4poster.svg',
]

graphic_files_install = []

# Copy index.html to build
graphic_files_install += configure_file(
  input:  'index.html',
  output: '@PLAINNAME@',
  copy: true
)

foreach file : graphic_files
  # Copy svg files to build
  graphic_files_install += configure_file(
    input:  file,
    output: '@PLAINNAME@',
    copy: true
  )
endforeach

if rsvg_convert_bin.found()
  foreach file : graphic_files
    sizeinfo = (
      file.contains('a4poster')
      ? [ '-w', '256', ]
      : []
    )

    graphic_files_install += custom_target(
      file,
      input: file,
      output: '@BASENAME@.png',
      command: [
        rsvg_convert_bin,
        sizeinfo,
        '-o', '@OUTPUT@',
        '@INPUT@',
      ],
      install: false,
      build_by_default: true,
    )
  endforeach
endif
