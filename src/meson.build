srn_hdrs = [
  'srn.h',
  'srn-app.h',
  'srn-flow.h',
  'srn-messenger.h',
]

install_headers(srn_hdrs)

srn_inc = include_directories('.')

srn_srcs = [
  'main.c',
  'srn-app.c',
  'srn-flow.c',
  'srn-messenger.c',
  'srn-flow-controller.h',
  'srn-flow-controller.c',
  'srn-module-manager.h',
  'srn-module-manager.c',
  'srn-window.h',
  'srn-window.c',
   srn_res,
] + srn_hdrs

srn_exec = executable(
  app_exec,
  srn_srcs,
  dependencies: [glib_dep, gmodule_dep, gtk_dep, gi_dep, meta_dep],
  export_dynamic: true,
  install: true,
  install_dir: bin_dir,
)

srn_gir = gnome.generate_gir(
  srn_exec,
  sources: srn_srcs,
  nsversion: '1.0',
  namespace: 'Srn',
  symbol_prefix: 'srn',
  identifier_prefix: 'Srn',
  includes: ['GObject-2.0', 'Gtk-4.0'],
  install: true,
)
