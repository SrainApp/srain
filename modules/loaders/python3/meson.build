python = import('python')
python3_dep = python.find_installation('python3').dependency(embed: true)

spy_srcs = [
  'spy-module.c',
  'spy-loader.h',
  'spy-loader.c',
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
  dependencies: [glib_dep, gobject_dep, gio_dep, python3_dep, meta_dep],
  include_directories: srn_inc,
  install: true,
  install_dir: pkg_sys_bind_dir,
)
