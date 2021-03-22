libsources = ['sirc-messenger.c', 'sirc-messenger.h']

gnome = import('gnome')
gobj_dep = dependency('gobject-2.0')
gtk_dep = dependency('gtk4')

girlib = shared_library(
  'sirc',
  sources : libsources,
  dependencies : [gobj_dep, gtk_dep],
  install : true
)

gnome.generate_gir(
  girlib,
  sources : libsources,
  nsversion : '1.0',
  namespace : 'Sirc',
  symbol_prefix : 'sirc',
  identifier_prefix : 'Sirc',
  includes : ['GObject-2.0', 'Gtk-4.0'],
  install : true,
)
