python = import('python')
python3_dep = python.find_installation('python3').dependency(embed: true)
pygobject_dep = dependency('pygobject-3.0')

spy_srcs = [
  'spy-module.c',
]

gnome = import('gnome')
spy_res = gnome.compile_resources(
  'spy-resources',
  'spy.gresource.xml',
  c_name: 'spy',
)

spy_lib = shared_module(
  'spy',
  sources: spy_srcs + spy_res,
  dependencies: [glib_dep, gobject_dep, gio_dep, gi_dep, python3_dep, pygobject_dep, gtk_dep, meta_dep],
  include_directories: srn_inc,
  install: true,
  install_dir: pkg_sys_bind_dir,
)
