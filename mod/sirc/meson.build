libsirc_srcs = ['sirc-messenger.c', 'sirc-messenger.h']

libsirc = shared_library(
  'sirc',
  sources: libsirc_srcs,
  dependencies: [glib_dep, gtk_dep],
  include_directories: libsif_inc,
  install: true,
  link_with: libsif,
)

gnome = import('gnome')
gnome.generate_gir(
  libsirc,
  sources: libsirc_srcs,
  nsversion: '1.0',
  namespace: 'Sirc',
  symbol_prefix: 'sirc',
  identifier_prefix: 'Sirc',
  includes: ['GObject-2.0', 'Gtk-4.0'],
  install: true,
)
