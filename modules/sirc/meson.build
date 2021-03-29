sirc_srcs = [
  'sirc-module.c',
  'sirc-messenger.h',
  'sirc-messenger.c',
  'sirc-login-flow.h',
  'sirc-login-flow.c',
]

gnome = import('gnome')
sirc_res = gnome.compile_resources(
  'sirc-resources',
  'sirc.gresource.xml',
  c_name: 'sirc',
)

sirc_lib = shared_module(
  'sirc',
  sources: sirc_srcs + sirc_res,
  dependencies: [glib_dep, gtk_dep],
  include_directories: srn_inc,
  install: true,
  install_dir: pkg_sys_mod_dir,
)
