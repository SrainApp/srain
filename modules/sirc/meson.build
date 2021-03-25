sirc_srcs = [
  'sirc-module.c',
  'sirc-messenger.h',
  'sirc-messenger.c',
]

sirc_lib = shared_module(
  'sirc',
  sources: sirc_srcs,
  dependencies: [glib_dep, gtk_dep],
  include_directories: srn_inc,
  install: true,
  install_dir: pkg_sys_mod_dir,
)
