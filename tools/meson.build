
tool_names = [
  'babl_fish_path_fitness',
  'babl-lut-verify',
  'babl-benchmark',
  'babl-html-dump',
  'babl-icc-dump',
  'babl-icc-rewrite',
  'babl-verify',
  'conversions',
  'formats',
  'introspect',
  'trc-validator',
]

foreach tool_name : tool_names
  tool = executable(tool_name,
    tool_name + '.c',
    include_directories: [rootInclude, bablInclude],
    link_with: babl,
    dependencies: [math, thread, lcms, log],
    install: false,
  )

  if tool_name == 'babl-html-dump'
    babl_html_dump = tool
  endif
endforeach
