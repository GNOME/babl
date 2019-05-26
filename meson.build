project('babl', 'c',
  license: 'LGPL3+',
  version: '0.1.65',
  meson_version: '>=0.47.0',
  default_options: [
    'libdir=lib',
    'sysconfdir=/etc',
    'localstatedir=/var',
    'sharedstatedir=/var/lib'
  ],
  # https://gitlab.gnome.org/GNOME/babl/issues/
)

# Making releases on the stable branch:
#   BABL_MICRO_VERSION += 1;
#   BABL_INTERFACE_AGE += 1;
#   BABL_BINARY_AGE += 1;
# if any functions have been added,
#    set BABL_INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
#    set BABL_BINARY_AGE _and_ BABL_INTERFACE_AGE to 0.


conf = configuration_data()

pkgconfig = import('pkgconfig')
i18n      = import('i18n')
gnome     = import('gnome')

cc        = meson.get_compiler('c')
prefix    = get_option('prefix')
buildtype = get_option('buildtype')

babl_prefix = get_option('prefix')
babl_libdir = join_paths(babl_prefix, get_option('libdir'))

################################################################################
# Projects infos

version = meson.project_version()
array_version = version.split('.')
major_version = array_version[0].to_int()
minor_version = array_version[1].to_int()
micro_version = array_version[2].to_int()
interface_age = 1

binary_age = 100 * minor_version + micro_version

lt_current = binary_age - interface_age

api_version = '@0@.@1@'.format(major_version, minor_version)
lib_version = '@0@:@1@:@2@'.format(lt_current, interface_age, lt_current)
so_version  = '@0@.@1@.@2@'.format(0, lt_current, interface_age)
lib_name    = meson.project_name() + '-' + api_version

stability_version_number = (major_version != 0 ? minor_version : micro_version)
stable = (stability_version_number % 2 == 0)

conf.set10('BABL_UNSTABLE', not stable, Description:
  'Define to 1 if this is an unstable version of BABL.')

conf.set       ('BABL_MAJOR_VERSION',    '@0@'.format(major_version))
conf.set       ('BABL_MINOR_VERSION',    '@0@'.format(minor_version))
conf.set       ('BABL_MICRO_VERSION',    '@0@'.format(micro_version))
conf.set_quoted('BABL_INTERFACE_AGE',    '@0@'.format(interface_age))
conf.set_quoted('BABL_BINARY_AGE',       '@0@'.format(binary_age))
conf.set_quoted('BABL_VERSION',          '@0@'.format(version))
conf.set_quoted('BABL_REAL_VERSION',     '@0@'.format(version))
conf.set_quoted('BABL_API_VERSION',      '@0@'.format(api_version))
conf.set_quoted('BABL_RELEASE',          '@0@'.format(api_version))
conf.set_quoted('BABL_LIBRARY_VERSION',  '@0@'.format(lib_version))
conf.set_quoted('BABL_CURRENT_MINUS_AGE','@0@'.format(0))
conf.set_quoted('BABL_LIBRARY',          '@0@'.format(lib_name))

################################################################################
# Host system environment

platform_android = false
platform_osx = false
platform_win32 = false

host_cpu = host_machine.cpu()
message('Architecture: ' + host_cpu)

if   host_cpu.startswith('i') and host_cpu.endswith('86')
  have_x86 = true
  conf.set10('ARCH_X86', true, description:
    'Define to 1 if you are compiling for ix86.')
elif host_cpu == 'x86_64'
  have_x86 = true
  conf.set10('ARCH_X86', true, description:
    'Define to 1 if you are compiling for ix86.')
  conf.set10('ARCH_X86_64', true, description:
    'Define to 1 if you are compiling for amd64.')
elif host_cpu == 'ppc' or host_cpu == 'powerpc'
  have_ppc = true
  conf.set10('ARCH_PPC', true, description:
    'Define to 1 if you are compiling for PowerPC.')
elif host_cpu == 'ppc64' or host_cpu == 'powerpc64'
  have_ppc = true
  conf.set10('ARCH_PPC', true, description:
    'Define to 1 if you are compiling for PowerPC.')
  conf.set10('ARCH_PPC64', true, description:
    'Define to 1 if you are compiling for PowerPC64.')
else
  error('Unknown host architecture')
endif

host_os = host_machine.system()
message('Host os: ' + host_os)

platform_win32 = (host_os.startswith('mingw') or
                  host_os.startswith('cygwin') or
                  host_os.startswith('windows'))

platform_osx = host_os.startswith('darwin')
if platform_osx
  if cc.get_id() != 'clang'
    error('You should use Clang/Clang++ on OSX.')
  endif
endif

platform_android = host_os.contains('android')

path_sep = ( platform_win32 ? ';'     : ':' )
dirs_sep = ( platform_win32 ? '\\\\'  : '/' )
lib_ext  = ( platform_win32 ? '.dll'  : '.so' )

conf.set('BABL_PATH_SEPARATOR', '\'' + path_sep + '\'', description:
  'separator between paths in BABL_PATH')
conf.set_quoted('BABL_DIR_SEPARATOR',  dirs_sep, description:
  'separator between directories in BABL_PATH')
conf.set_quoted('SHREXT', lib_ext, description:
  'File extension for shared libraries')

# assume *nix if not android/osx/win32
platform_unix = not (
  platform_android or
  platform_osx or
  platform_win32
)

# Build system environment
build_os = build_machine.system()
message('Build os: ' + build_os)

build_platform_win32 = (build_os.startswith('mingw') or
                        build_os.startswith('cygwin') or
                        build_os.startswith('windows'))

# Only run cross compile objects if we have exe wrapper
cc_can_run = not meson.is_cross_build() or meson.has_exe_wrapper()

################################################################################
# Compiler arguments

common_c_flags = []
if cc.get_id().startswith('clang') or cc.get_id().startswith('gcc')
  common_c_flags += cc.get_supported_arguments(['-Ofast'])
endif

extra_warnings_list = [
  '-Wdeclaration-after-statement',
  '-Winit-self',
  '-Wmissing-declarations',
  '-Wmissing-prototypes',
  '-Wold-style-definition',
  '-Wpointer-arith',
]
common_c_flags += cc.get_supported_arguments(extra_warnings_list)

add_project_arguments(common_c_flags, language: 'c')

################################################################################
# Check for compiler CPU extensions

# mmx assembly
if cc.has_argument('-mmmx') and get_option('enable-mmx')
  if cc.compiles('asm ("movq 0, %mm0");')
    message('mmx assembly available')
    add_project_arguments('-mmmx', language: 'c')
    conf.set('USE_MMX', 1, description:
      'Define to 1 if MMX assembly is available.')

    # sse assembly
    if cc.has_argument('-msse') and get_option('enable-sse') 
      if cc.compiles('asm ("movntps %xmm0, 0");')
        add_project_arguments('-msse', language: 'c')
        message('sse assembly available')
        conf.set('USE_SSE', 1, description:
          'Define to 1 if SSE assembly is available.')
        sse_args = ['-mfpmath=sse']
        if platform_win32
          sse_args += '-mstackrealign'
        endif

        foreach sse_arg : sse_args
          if cc.has_argument(sse_arg)
            add_project_arguments(sse_arg, language: 'c')
          endif
        endforeach

        # sse2 assembly
        if cc.has_argument('-msse2') and get_option('enable-sse2')
          if cc.compiles('asm ("punpckhwd %xmm0,%xmm1");')
            message('sse2 assembly available')
            sse2_cflags = '-msse2'
            conf.set('USE_SSE2', 1, description:
              'Define to 1 if sse2 assembly is available.')

            # sse4.1 assembly
            if cc.has_argument('-msse4.1') and get_option('enable-sse4_1')
              if cc.compiles('asm ("pmovzxbd %xmm0,%xmm1");')
                message('sse4.1 assembly available')
                sse4_1_cflags = '-msse4.1'
                conf.set('USE_SSE4_1', 1, description:
                  'Define to 1 if sse4.1 assembly is available.')
              endif
            endif
          endif
        endif
      endif
      if cc.has_argument('-mf16c') and get_option('enable-f16c')
        if cc.compiles(
          'asm ("#include <immintrin.h>],' +
          '[__m128 val = _mm_cvtph_ps ((__m128i)_mm_setzero_ps());' +
          ' __m128i val2 = _mm_insert_epi64((__m128i)_mm_setzero_ps(),0,0);");'
          )
          message('Can compile half-floating point code (f16c)')
          f16c_cflags = '-mf16c'
          conf.set('USE_F16C', 1, description:
            'Define to 1 if f16c intrinsics are available.')
        endif
      endif
    endif
  endif
endif

################################################################################
# Check environment

# Check headers
check_headers = [
]
# Don't check for dynamic load on windows
if not platform_win32
  check_headers += [
    ['HAVE_DLFCN_H', 'dlfcn.h'],
    ['HAVE_DL_H', 'dl.h'],
  ]
endif
foreach header: check_headers
  if cc.has_header(header[1])
    conf.set(header[0], 1, description: 
      'Define to 1 if the <@0@> header is available'.format(header[1]))
  endif
endforeach


# Check functions
# general
check_functions = [
  ['HAVE_GETTIMEOFDAY', 'gettimeofday', '<sys/time.h>'],
  ['HAVE_RINT',         'rint'        , '<math.h>'],  
  ['HAVE_SRANDOM',      'srandom'     , '<stdlib.h>'],
]
foreach func: check_functions
  if cc.has_function(func[1], prefix: '#include ' + func[2])
    conf.set(func[0], 1, description: 
      'Define to 1 if the @0@() function is available'.format(func[1]))
  endif
endforeach


# Check for uncommon features

# babl_fish_reference(), create_name() would like this
if cc.compiles('int main() { static __thread char buf[1024]; }')
  conf.set('HAVE_TLS', 1, description:
    'Define to 1 if compiler supports __thread')
endif


################################################################################
# Dependencies

# LCMS
if get_option('with-lcms')
  lcms = dependency('lcms2', required: true, native: false)
  if lcms.found()
    conf.set('HAVE_LCMS', 1, description: 
      'Define to 1 if liblcms2 is available')
  endif
else
  lcms = declare_dependency()
endif

math = cc.find_library('m',   required: false)
thread = dependency('threads',  required: false)
if platform_android
  log  = cc.find_library('log', required: false)
else
  log = []
endif
if platform_win32 
  dl = []
else
  dl = cc.find_library('dl',  required: false)
endif


################################################################################
# Build utilities

env_bin = find_program('env', required: false, native: true)
git_bin = find_program('git', required: false, native: true)
rsvg_convert_bin = find_program('rsvg-convert', required: false,
  native: true)
test_bin = find_program('test', required: false, native: true)
w3m_bin = find_program('w3m', required: false, native: true)

################################################################################
# Build flags

build_docs = false

if get_option('with-docs')
  if cc_can_run and env_bin.found() and not platform_win32
    build_docs = true
  else
    warning('Unable to generate docs in this environment')
  endif
endif


################################################################################
# Configuration files

# config.h
configure_file(
  output: 'config.h',
  configuration: conf
)

# pkg-config file
pkgconfig.generate(filebase: 'babl',
  name: 'babl',
  description: 'Dynamic, any to any, pixel format conversion library',
  version: meson.project_version(),
  libraries: [
    '-L${libdir}',
    '-l' + lib_name,
  ],
  libraries_private: [
    '-lm', '-lpthread',
  ],
  subdirs: [
    lib_name,
  ],
)

################################################################################
# Subdirs

rootInclude = include_directories('.')

subdir('babl')
subdir('extensions')
subdir('tests')
subdir('tools')
if build_docs
  subdir('docs')
endif

if w3m_bin.found() and build_docs
  custom_target('README',
    input: index_html,
    output: 'README',
    command: [
      w3m_bin,
      '-cols', '72',
      '-dump',
      '@INPUT@',
    ],
    capture: true,
  )
endif
