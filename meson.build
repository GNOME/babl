project('babl', 'c',
  license: 'GPL3+',
  version: '0.1.39',
  meson_version: '>=0.40.0',
  default_options: [
    'sysconfdir=/etc',
    'localstatedir=/var',
    'sharedstatedir=/var/lib'
  ],
  # http://bugzilla.gnome.org/enter_bug.cgi?product=GEGL
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

conf.set10('BABL_UNSTABLE', not stable)

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
# Host system detection

host_cpu = host_machine.cpu()
message('Architecture: ' + host_cpu)

if   host_cpu.startswith('i') and host_cpu.endswith('86')
  have_x86 = true
  conf.set10('ARCH_X86',    true)
elif host_cpu == 'x86_64'
  have_x86 = true
  conf.set10('ARCH_X86',    true)
  conf.set10('ARCH_X86_64', true)
elif host_cpu == 'ppc' or host_cpu == 'powerpc'
  have_ppc = true
  conf.set10('ARCH_PPC',    true)
elif host_cpu == 'ppc64' or host_cpu == 'powerpc64'
  have_ppc = true
  conf.set10('ARCH_PPC',    true)
  conf.set10('ARCH_PPC64',  true)
else
  error('Unknown host architecture')
endif


host_os = host_machine.system()
message('Host os: ' + host_os)

platform_win32 = (host_os.startswith('mingw') or host_os.startswith('cygwin'))
# Is this native win32
os_win32 = host_os.startswith('mingw')

platform_osx = host_os.startswith('darwin')
if platform_osx
  if cc.get_id() != 'clang'
    error('You should use CLang++ on OSx.')
  endif
endif

platform_android = host_os.contains('android')

path_sep = ( os_win32 ? ';'  : ':' )
dirs_sep = ( os_win32 ? '\\' : '/' )
conf.set('BABL_PATH_SEPARATOR', '\'' + path_sep + '\'')
conf.set_quoted('BABL_DIR_SEPARATOR',  dirs_sep)

################################################################################
# Extra warnings

extra_warnings_list = [
  '-Wdeclaration-after-statement',
  '-Winit-self',
  '-Wmissing-declarations',
  '-Wmissing-prototypes',
  '-Wold-style-definition',
  '-Wpointer-arith',
]
extra_warnings = []
foreach warning : extra_warnings_list
  if cc.has_argument(warning)
    extra_warnings += warning
  endif
endforeach


################################################################################
# Check for compiler CPU extensions

have_tls_run = cc.run('int main() { static __thread char buf[1024]; return 0; }')
conf.set('HAVE_TLS', (have_tls_run.compiled() and have_tls_run.returncode() == 0))

has_ssem = cc.has_argument('-mfpmath=sse')
if has_ssem
  add_project_arguments('-mfpmath=sse',
    language: 'c')
endif

has_mmx  = cc.has_argument('-mmmx') and get_option('enable-mmx')
if has_mmx
  add_project_arguments(   '-mmmx',
    language: 'c')
endif

has_sse  = cc.has_argument('-msse') and get_option('enable-sse')
if has_sse
  add_project_arguments(   '-msse',
    language: 'c')
endif

has_sse2 = cc.has_argument('-msse2') and get_option('enable-sse2')
if has_sse2
  add_project_arguments(   '-msse2',
    language: 'c')
endif

has_sse41= cc.has_argument('-msse4.1') and get_option('enable-sse4_1')
if has_sse41
  add_project_arguments(   '-msse4.1',
    language: 'c')
endif

had_f16c= cc.has_argument('-mf16c') and get_option('enable-f16c')
if had_f16c
  add_project_arguments(   '-mf16c',
    language: 'c')
endif

have_dlfcn_h = cc.has_header('dlfcn.h')
have_dl_h    = cc.has_header('dl.h')
if not (have_dlfcn_h or have_dl_h)
  error('Header dlfcn.h or dl.h not provided. Please provide one of them.')
endif

conf.set('HAVE_DLFCN_H', have_dlfcn_h)
conf.set('HAVE_DL_H', have_dl_h)


shared_lib_ext = (platform_win32 ? '.dll' : '.so')
conf.set_quoted('SHREXT', shared_lib_ext)

################################################################################
# Utilities

rsvg_convert = find_program('rsvg-convert', required: false)
w3m          = find_program('w3m', required: false)


################################################################################
# Dependencies

if os_win32
  math = []
  thread = []
else
  math = cc.find_library('m')
  thread  = cc.find_library('pthread')
endif

if platform_android
  log = cc.find_library('log')
else
  log = []
endif

dl = cc.find_library('dl')


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

configure_file(
  output: 'config.h',
  configuration: conf
)


################################################################################
# Subdirs

rootInclude = include_directories('.')
version_script = join_paths(
  meson.source_root(), meson.current_source_dir(), 'babl.map'
)

subdir('babl')
subdir('extensions')
subdir('tests')
subdir('tools')
if get_option('with-docs')
  subdir('docs')
endif


if w3m.found()
  custom_target('README',
    input : [ join_paths('docs', 'index.html'), ] ,
    output: [ 'README' ] ,
    command: [
      w3m,
      '-cols', '72',
      '-dump',
      '@INPUT@',
    ],
    capture: true,
  )
endif
