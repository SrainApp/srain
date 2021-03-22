libsif_inc = include_directories('.')

libsif_hdrs = [
  'sif/sif.h',
  'sif/sif-messenger.h',
  'sif/sif-flow.h',
]

install_headers(
  libsif_hdrs,
  subdir: 'sif',
)

libsif_srcs = [
  'sif-flow.c',
  'sif-messenger.c',
] + libsif_hdrs

libsif = library(
  'sif',
  sources : libsif_srcs,
  dependencies : [glib_dep, gtk_dep],
  install : true
)

gnome = import('gnome')
gnome.generate_gir(
  libsif,
  sources : libsif_srcs,
  nsversion : '1.0',
  namespace : 'Sif',
  symbol_prefix : 'sif',
  identifier_prefix : 'Sif',
  includes : ['GObject-2.0', 'Gtk-4.0'],
  install : true,
)

pkgconfig = import('pkgconfig')
pkgconfig.generate(
  name: 'SrainInterface',
  filebase: 'libsif',
  description: 'Interface between Srain\'s client and messengers',
  libraries: [libsif],
)
