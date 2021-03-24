sirc_srcs = [
  'sirc-messenger.h',
  'sirc-messenger.c',
]

sirc_lib = shared_module(
  'sirc',
  sources: sirc_srcs,
  dependencies: [glib_dep, gtk_dep],
  include_directories: srn_inc,
  install: true,
)

gnome = import('gnome')
gnome.generate_gir(
  sirc_lib,
  sources: sirc_srcs,
  nsversion: '1.0',
  namespace: 'Sirc',
  symbol_prefix: 'sirc',
  identifier_prefix: 'Sirc',
  install: true,
)
