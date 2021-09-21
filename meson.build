#####################
# Project meta info #
#####################

project(
  'srain', 'c',
  version: '1.3.0',
  license: 'GPL3',
  meson_version: '>= 0.45.0'
)

app_id = 'im.srain.Srain'
app_name = meson.project_name()
app_pretty_name = 'Srain'
app_generic_name = 'Srain IRC Client'
app_desc = 'Modern IRC client written in GTK'
app_url = 'https://srain.im'
app_bugreport = 'https://github.com/SrainApp/' + app_name + '/issues/new'
app_exec = app_name

# Meson builtin option
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
pkg_data_dir = join_paths(data_dir, meson.project_name())
# https://github.com/mesonbuild/meson/issues/825
pkg_doc_dir = join_paths(data_dir, 'doc', meson.project_name())
icons_dir = join_paths(data_dir, 'icons')
po_dir = join_paths(meson.source_root (), 'po')
sys_conf_dir = join_paths(prefix, get_option('sysconfdir'))
pkg_sys_conf_dir = join_paths(sys_conf_dir, meson.project_name())

srcs = []

subdir('data')
subdir('po')
subdir('src')
subdir('doc')
