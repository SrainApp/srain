srcs = [
  'main.c',
  'srn-app.h',
  'srn-app.c',
  'srn-flow-controller.h',
  'srn-flow-controller.c',
  'srn-window.h',
  'srn-window.c',
]

subdir('data')

exec = executable(app_exec, srcs,
  dependencies: [glib_dep, gtk_dep, gi_dep, meta_dep],
  include_directories: libsif_inc,
  install: true,
  install_dir: bin_dir,
  export_dynamic: true,
  link_with: libsif,
)

gnome.generate_gir(
  exec,
  sources : srcs,
  nsversion : '1.0',
  namespace : 'Srn',
  symbol_prefix : 'srn',
  identifier_prefix : 'Srn',
  includes : ['Gtk-4.0', 'GObject-2.0'],
  install : true,
)
