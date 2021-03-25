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

spy_lib = shared_library(
  'spy',
  sources: spy_srcs + spy_res,
  dependencies: [glib_dep, gobject_dep, gio_dep, python3_dep, pygobject_dep],
  include_directories: srn_inc,
  install: true,
  export_dynamic: true,
  link_args: '-rdynamic',
  install_dir: pkg_sys_mod_dir,
)
