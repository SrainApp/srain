#####################
# Project meta info #
#####################

project(
  'srain', 'c',
  version: '2.0.0rc0',
  license: 'GPL3',
  meson_version: '>= 0.46.0'
)

app_id = 'im.srain.Srain'
app_name = meson.project_name()
app_pretty_name = 'Srain'
app_generic_name = 'Srain IM Client'
app_desc = 'Modern instant messaging client'
app_url = 'https://srain.im'
app_bugreport = 'https://github.com/SrainApp/' + app_name + '/issues/new'
app_exec = app_name

if get_option('buildtype') == 'debug'
  app_pretty_name += ' (Development)'
  app_generic_name += ' (Development)'
  app_desc += ' (Development version)'
endif

############################
# Installation directories #
############################

prefix = get_option('prefix')
bin_dir = join_paths(prefix, get_option('bindir'))
data_dir = join_paths(prefix, get_option('datadir'))
lib_dir = join_paths(prefix, get_option('libdir'))
pkg_data_dir = join_paths(data_dir, meson.project_name())
# https://github.com/mesonbuild/meson/issues/825
pkg_doc_dir = join_paths(data_dir, 'doc', meson.project_name())
icons_dir = join_paths(data_dir, 'icons')
po_dir = join_paths(meson.source_root (), 'po')
sys_conf_dir = join_paths(prefix, get_option('sysconfdir'))
pkg_sys_conf_dir = join_paths(sys_conf_dir, meson.project_name())

####################
# Meta info header #
####################

meta_data = configuration_data()
meta_data.set_quoted('GETTEXT_PACKAGE', app_name)
meta_data.set_quoted('PACKAGE', app_name)
meta_data.set_quoted('PACKAGE_NAME', app_name)
meta_data.set_quoted('PACKAGE_VERSION', meson.project_version())
meta_data.set_quoted('PACKAGE_DESC', app_desc)
meta_data.set_quoted('PACKAGE_APPID', app_id)
meta_data.set_quoted('PACKAGE_BUILD', '@VCS_TAG@')
meta_data.set_quoted('PACKAGE_WEBSITE', app_url)
meta_data.set_quoted('PACKAGE_BUGREPORT', app_bugreport)
meta_data.set_quoted('PACKAGE_COPYRIGHT_DATES', '2016 - 2021')
meta_data.set_quoted('PACKAGE_DATA_DIR', data_dir)
meta_data.set_quoted('PACKAGE_LIB_DIR', lib_dir)
meta_data.set_quoted('PACKAGE_CONFIG_DIR', sys_conf_dir)
meta_data.set_quoted('PACKAGE_CODESET', 'UTF-8')

build_id_generator = [
  join_paths(meson.source_root() , 'utils', 'gen-build-id.sh'),
  meson.source_root(),
]

meta_h = vcs_tag(
  command: build_id_generator,
  input: configure_file(
    output: 'srn-meta.h.in',
    configuration: meta_data
  ),
  output: 'srn-meta.h',
  fallback: 'unknown',
)

################
# Dependencies #
################

glib_dep = dependency('glib-2.0', version: '>= 2.39.3')
gtk_dep = dependency('gtk4', version: '>= 4.0.0')
gi_dep = dependency('gobject-introspection-1.0')
meta_dep = declare_dependency(sources: meta_h)

##################
# Subdirectories #
##################

subdir('data')
subdir('src')
subdir('mod')
# subdir('po')
# subdir('doc')
