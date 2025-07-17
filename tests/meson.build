
test_names = [
  'babl_class_name',
  'cairo_cmyk_hack',
  'cairo-RGB24',
  'cmyk',
  'chromaticities',
  'conversions',
  'extract',
  'floatclamp',
  'float-to-8bit',
  'format_with_space',
  'grayscale_to_rgb',
  'hsl',
  'hsva',
  'models',
  'n_components',
  'n_components_cast',
  'nop',
  'palette',
  'rgb_to_bgr',
  'rgb_to_ycbcr',
  'sanity',
  'srgb_to_lab_u8',
  'transparent',
  'alpha_symmetric_transform',
  'types',
  'xyz_to_lab'
]
if platform_unix
  test_names += [
    'concurrency-stress-test',
    'palette-concurrency-stress-test',
    'trcs',
  ]
endif

test_env = environment()
test_env.set('BABL_PATH', babl_extensions_build_dir)

foreach test_name : test_names
  test = executable(test_name,
    test_name + '.c',
    include_directories: [rootInclude, bablInclude],
    link_with: babl,
    dependencies: [thread, lcms, log],
    export_dynamic: true,
    install: false,
  )

  test(test_name,
    test,
    env: test_env,
    workdir: meson.current_build_dir(),
  )
endforeach
