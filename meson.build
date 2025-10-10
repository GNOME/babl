project('babl', 'c',
  license: 'LGPL3+',
  version: '0.1.117',
  meson_version: '>=0.60.0',
  default_options: [
    'buildtype=debugoptimized'
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
gnome     = import('gnome')
python    = import('python').find_installation()

cc        = meson.get_compiler('c')
prefix    = get_option('prefix')
buildtype = get_option('buildtype')

babl_prefix = get_option('prefix')
babl_libdir = babl_prefix / get_option('libdir')

project_build_root = meson.current_build_dir()
project_source_root = meson.current_source_dir()

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

conf.set10('BABL_UNSTABLE', not stable, description:
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

host_cpu_family = host_machine.cpu_family()
if   host_cpu_family == 'x86'
  have_x86 = true
  conf.set10('ARCH_X86',    true)
elif host_cpu_family == 'x86_64'
  have_x86 = true
  conf.set10('ARCH_X86',    true)
  conf.set10('ARCH_X86_64', true)
elif host_cpu_family == 'ppc'
  have_ppc = true
  conf.set10('ARCH_PPC',    true)
elif host_cpu_family == 'ppc64'
  have_ppc = true
  conf.set10('ARCH_PPC',    true)
  conf.set10('ARCH_PPC64',  true)
elif host_cpu_family == 'arm'
  have_arm = true
  conf.set10('ARCH_ARM',  true)
elif host_cpu_family == 'aarch64'
  have_aarch64 = true
  conf.set10('ARCH_AARCH64',  true)
endif


host_os = host_machine.system()
message('Host os: ' + host_os)

platform_win32 = (host_os.startswith('mingw') or
                  host_os.startswith('cygwin') or
                  host_os.startswith('windows'))

platform_osx = host_os.startswith('darwin')

platform_android = host_os.contains('android')

path_sep = ( platform_win32 ? ';'     : ':' )
dirs_sep = ( platform_win32 ? '\\\\'  : '/' )
if platform_win32
  lib_ext = '.dll'
elif platform_osx
  lib_ext = '.dylib'
else
  lib_ext = '.so'
endif

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

# Only try to run compiled programs if native compile or cross-compile
# and have exe wrapper. If we don't need a wrapper (e.g. 32 bit build in
# 64-bit environment) then set proprty has_exe_wrapper=true in cross 
# file
can_run_host_binaries = meson.can_run_host_binaries()


################################################################################
# Compiler arguments

common_c_flags = []
common_l_flags = []

common_c_flags += cc.get_supported_arguments(
  ['-fno-unsafe-math-optimizations','-ftree-vectorize']
)


supress_warnings_list = [
  '-Wno-implicit-function-declaration',
]
common_c_flags += cc.get_supported_arguments(supress_warnings_list)

extra_warnings_list = [
  '-Wdeclaration-after-statement',
  '-Winit-self',
  '-Wmissing-declarations',
  '-Wmissing-prototypes',
  '-Wold-style-definition',
  '-Wpointer-arith',
]
common_c_flags += cc.get_supported_arguments(extra_warnings_list)


# DEBUG SYMBOLS
debugging_format = 'Disabled'
if buildtype == 'debug' or buildtype == 'debugoptimized'
  if not platform_win32
    # DWARF symbols for GCC and LLDB on Unix
    debugging_format = 'Native'
  elif cc.get_argument_syntax() == 'msvc'
    # CodeView symbols for DIA or DbgHelp debuggers and LLDB on Windows
    debugging_format = 'Native'
  elif platform_win32 and get_option('win-debugging') == 'native' and cc.has_argument('-gcodeview') and cc.has_link_argument('-Wl,--pdb=') and cc.get_id() == 'clang'
    # CodeView symbols for DIA or DbgHelp debuggers and LLDB on Windows
    debugging_format = 'Native'
    common_c_flags += '-gcodeview'
    common_l_flags += '-Wl,--pdb='
  else
    # DWARF symbols for GCC and LLDB on Windows (fallback)
    debugging_format = 'DWARF'
    if cc.get_id() == 'clang'
      # Optimize DWARF symbols to Dr. Mingw
      # https://github.com/jrfonseca/drmingw/issues/42
      common_c_flags += '-gdwarf-aranges'
    endif
  endif
endif


if platform_win32 and cc.get_id() == 'clang'
  # Workaround to get colored output
  # https://github.com/msys2/MINGW-packages/issues/2988
  common_c_flags += '-fansi-escape-codes'
endif

if cc.get_argument_syntax() == 'msvc'
  #Needed otherwise MSVC get angry with the headers provided by UCRT
  common_c_flags += ['/D_USE_MATH_DEFINES']
endif

add_project_arguments(common_c_flags, language: 'c')

# Linker arguments
if platform_win32 and cc.has_link_argument('-Wl,--no-undefined')
  no_undefined = '-Wl,--no-undefined'
else
  no_undefined = []
endif

if host_cpu_family == 'x86_64'
  x86_64_v2_flags = cc.get_supported_arguments(['-march=x86-64','-msse2', '-msse2','-msse4.1','-msse4.2','-mpopcnt','-mssse3'])
  x86_64_v3_flags = x86_64_v2_flags + cc.get_supported_arguments(['-mavx','-mavx2','-mf16c','-mfma','-mmovbe', '-mbmi', '-mbmi2'])

  x86_64_v2_flags += '-DX86_64_V2'
  x86_64_v3_flags += '-DX86_64_V3'

elif host_cpu_family == 'arm'
  arm_neon_flags = cc.get_supported_arguments(['-mfpu=neon-vfpv3', '-mfpu=neon-vfpv4'])
  arm_neon_flags += '-DARM_NEON'
elif host_cpu_family == 'aarch64'
  common_c_flags += cc.get_supported_arguments(['-mfpu=neon-fp-armv8', '-ftree-vectorize'])
endif

add_project_link_arguments(common_l_flags, language: 'c')

################################################################################
# Check for compiler CPU extensions

# intialize these to nothing, so meson doesn't complain on non-x86

have_mmx    = false
have_sse    = false
have_sse2   = false
have_sse4_1 = false
have_avx2   = false
have_f16c   = false

sse2_cflags   = []
f16c_cflags   = []
sse4_1_cflags = []
avx2_cflags   = []

# mmx assembly
if get_option('enable-mmx') and cc.has_argument('-mmmx')
  if cc.compiles('asm ("movq 0, %mm0");')
    message('mmx assembly available')
    add_project_arguments('-mmmx', language: 'c')
    conf.set('USE_MMX', 1, description:
      'Define to 1 if MMX assembly is available.')
    have_mmx = true

    # sse assembly
    if get_option('enable-sse') and cc.has_argument('-msse')
      if cc.compiles('asm ("movntps %xmm0, 0");')
        add_project_arguments('-msse', language: 'c')
        message('sse assembly available')
        conf.set('USE_SSE', 1, description:
          'Define to 1 if SSE assembly is available.')
        have_sse = true
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
        if get_option('enable-sse2') and cc.has_argument('-msse2')
          if cc.compiles('asm ("punpckhwd %xmm0,%xmm1");')
            message('sse2 assembly available')
            sse2_cflags = '-msse2'
            conf.set('USE_SSE2', 1, description:
              'Define to 1 if sse2 assembly is available.')
            have_sse2 = true

            # sse4.1 assembly
            if get_option('enable-sse4_1') and cc.has_argument('-msse4.1')
              if cc.compiles('asm ("pmovzxbd %xmm0,%xmm1");')
                message('sse4.1 assembly available')
                sse4_1_cflags = '-msse4.1'
                conf.set('USE_SSE4_1', 1, description:
                  'Define to 1 if sse4.1 assembly is available.')
                have_sse4_1 = true
              endif

              # avx2 assembly
              if get_option('enable-avx2') and cc.has_argument('-mavx2')
                if cc.compiles('asm ("vpgatherdd %ymm0,(%eax,%ymm1,4),%ymm2");')
                  message('avx2 assembly available')
                  avx2_cflags = '-mavx2'
                  conf.set('USE_AVX2', 1, description:
                    'Define to 1 if avx2 assembly is available.')
                  have_avx2 = true
                endif
              endif
            endif
          endif
        endif
      endif
      if get_option('enable-f16c') and cc.has_argument('-mf16c')
        if cc.compiles(
          'asm ("#include <immintrin.h>],' +
          '[__m128 val = _mm_cvtph_ps ((__m128i)_mm_setzero_ps());' +
          ' __m128i val2 = _mm_insert_epi64((__m128i)_mm_setzero_ps(),0,0);");'
          )
          message('Can compile half-floating point code (f16c)')
          f16c_cflags = '-mf16c'
          conf.set('USE_F16C', 1, description:
            'Define to 1 if f16c intrinsics are available.')
          have_f16c = true
        endif
      endif
    endif
  endif
endif

################################################################################
# Check environment

# Check headers
check_headers = [
  ['HAVE_STDATOMIC_H', 'stdatomic.h']
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


# gobject introspection
g_ir = dependency('gobject-introspection-1.0', version: '>=1.32.0',
  required: false)

# lcms
lcms = dependency('lcms2', version: '>=2.8', required: get_option('with-lcms'))
if lcms.found()
  conf.set('HAVE_LCMS', 1, description:
    'Define to 1 if liblcms2 is available')
endif

# vapigen
vapigen   = dependency('vapigen', version:'>=0.20.0', required: false)


################################################################################
# Build utilities

# build from git repo
git_bin = find_program('git', required: false, native: true)

# docs
rsvg_convert_bin = find_program('rsvg-convert', required: false,
  native: true)
w3m_bin = find_program('w3m', required: false, native: true)


################################################################################
# Build flags

# Docs - don't build by default in cross-build environments
build_docs = true
if get_option('with-docs') == 'auto'
  if meson.is_cross_build()
    build_docs = false
    message(
      'configure with -Ddocs=true to cross-build documentation'
    )
  endif
elif get_option('with-docs') == 'false'
  build_docs = false
endif

# Introspection - don't build  by default on cross-build environments
if get_option('enable-gir') == 'auto'
  build_gir = meson.is_cross_build() ? false : true
else
  build_gir = get_option('enable-gir') == 'true' ? true : false
endif
if not g_ir.found()
  build_gir = false
endif

# Vapi - only build if building introspection
if build_gir and get_option('enable-vapi') and vapigen.found()
  build_vapi = true
else
  build_vapi = false
endif

gi_docgen = find_program('gi-docgen', required: get_option('gi-docgen'))
if get_option('gi-docgen').auto()
 build_gi_docgen = build_docs and build_gir and gi_docgen.found() and not meson.is_cross_build()
else
 build_gi_docgen = get_option('gi-docgen').enabled()

 if build_gi_docgen and (not build_docs or not build_gir)
   warning('-Dwith-docs=true and -Denable-gir=true are required to build gi-docgen documentation')
   build_gi_docgen = false
 endif
endif


################################################################################
# Configuration files

# This should not be made visible in babl_dep due to possible name clash
# when built as a sub-project.
rootInclude = include_directories('.')

# config.h
configure_file(
  output: 'config.h',
  configuration: conf
)

# If git is available, always check if git-version.h should be
# updated. If git is not available, don't do anything if git-version.h
# already exists because then we are probably working with a tarball
# in which case the git-version.h we ship is correct.
is_git_repository = run_command(
    git_bin,
    'rev-parse',
    '--is-inside-work-tree',
    check: false,
).returncode() == 0

has_version_h = run_command(python, '-c',
  'import sys,os; sys.exit(0 if os.path.exists("git-version.h") else 1)',
  check: false
).returncode() == 0

if is_git_repository or not has_version_h
  git_version_h = vcs_tag(
    input : 'git-version.h.in',
    output: 'git-version.h',
    replace_string: '@BABL_GIT_VERSION@',
    fallback: 'unknown (unsupported)',
    command: [ git_bin.full_path(), 'describe', '--always' ],
  )

  if not meson.is_subproject()
    meson.add_dist_script(
      [ 'ninja', 'git-version.h', ],
    )
    meson.add_dist_script(
      python, '-c',
      'from pathlib import Path; from shutil import copy2; import sys, os; target_dir = Path(os.environ.get("MESON_DIST_ROOT")); target_dir.mkdir(parents=True, exist_ok=True); copy2(sys.argv[1], target_dir)', git_version_h.full_path()
    )
  endif

  if not is_git_repository or not git_bin.found()
  # We create git-version.h but know it will be useless because we are
  # not in a git repository. Output a warning.
    git_warning = '''

          UNSUPPORTED BUILD!

          This is not a distribution tarball (git-version.h missing) and
          we could not establish the corresponding commit (either this is
          not a git repository or git command is missing). Therefore
          we have no reference for debugging.
          Please either use release tarballs or build from the repository.
    '''
    warning(git_warning)
  endif
else
  git_version_h = files('git-version.h')
endif

################################################################################
# Global variables

xml_insert_file = files('tools' / 'xml-insert.py')
authors_file = files('AUTHORS')
news_file = files('NEWS')
todo_file = files('TODO')
export_symbols_file = files('export-symbols.def')
gen_babl_map_file = files('gen_babl_map.py')

################################################################################
# Install debug data (.pdb) on Windows
# Ideally meson should take care of it automatically.
# See: https://github.com/mesonbuild/meson/issues/12977
if platform_win32 and debugging_format == 'Native' and cc.get_argument_syntax() != 'msvc'
  install_win_debug_script = find_program('./meson_install_win_debug.py')
  meson.add_install_script(install_win_debug_script)
endif

################################################################################
# Subdirs

subdir('babl')
subdir('extensions')
subdir('tests')
subdir('tools')
if build_docs
  subdir('docs')
endif
subdir('bin')

# Create README file from web page
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
    build_by_default: true
  )
endif


# pkg-config file
pkgconfig.generate(
  babl,
  filebase: 'babl-' + api_version,
  name: 'babl',
  description: 'Pixel encoding and color space conversion engine.',
  version: meson.project_version(),
  subdirs: lib_name,
  variables: [
    'datadir=${prefix}/share',
    'pluginsdir=${libdir}/@0@'.format(lib_name),
    'girdir=${datadir}/gir-1.0',
    'typelibdir=${libdir}/girepository-1.0',
  ],
  uninstalled_variables: [
    'babl_libdir=${prefix}/babl',
    'pluginsdir=${prefix}/extensions',
    'girdir=${babl_libdir}',
    'typelibdir=${babl_libdir}',
  ],
)

# dependency for wrap builds
babl_dep = declare_dependency(
  include_directories: bablInclude,
  link_with : babl,
  sources: [
    babl_version_h,
    build_gir ? babl_gir : []
  ],
  variables: {
    'babl_path'   : babl_extensions_build_dir,
    'babl_libdir' : babl_library_build_dir,
    'girdir'      : babl_library_build_dir,
    'typelibdir'  : babl_library_build_dir,
  },
)
meson.override_dependency('babl-' + api_version, babl_dep)

################################################################################
# Build summary
summary(
  {
    'prefix': babl_prefix,
    'libdir': get_option('libdir'),
  }, section: 'Directories'
)
summary(
  {
    'BABL docs (babl website)' : build_docs,
    'gi-docgen'                : build_gi_docgen,
    'Introspection'            : build_gir,
    'VALA support'             : build_vapi,
    'Debug symbols format'     : debugging_format,
  }, section: 'Optional features'
)
summary(
  {
    'mmx'            : have_mmx,
    'sse'            : have_sse,
    'sse2'           : have_sse2,
    'sse4_1'         : have_sse4_1,
    'avx2'           : have_avx2,
    'f16c (half fp)' : have_f16c,
  }, section: 'Processor extensions'
)
summary(
  {
    'lcms' : lcms.found(),
  }, section: 'Optional dependencies'
)
