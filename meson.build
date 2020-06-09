#####################
# Project meta info #
#####################

project(
  'srain', 'c',
  version: '1.1.0',
  license: 'GPL3',
  meson_version: '>= 0.49.0'
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
endif

############################
# Installation directories #
############################

prefix = get_option('prefix')
bin_dir = prefix / get_option('bindir')
locale_dir = prefix / get_option('localedir')
data_dir = prefix / get_option('datadir')
pkg_data_dir = data_dir / meson.project_name()
icons_dir = data_dir / 'icons'
po_dir = meson.source_root () / 'po'
gettext_package = meson.project_name()
sys_conf_dir = prefix / get_option('sysconfdir')

gnome = import('gnome')
i18n = import('i18n')

subdir('data')
subdir('po')
subdir('src')
