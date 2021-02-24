meta_h = configuration_data()
meta_h.set_quoted('GETTEXT_PACKAGE', app_name)
meta_h.set_quoted('PACKAGE', app_name)
meta_h.set_quoted('PACKAGE_NAME', app_name)
meta_h.set_quoted('PACKAGE_VERSION', meson.project_version())
meta_h.set_quoted('PACKAGE_DESC', app_desc)
meta_h.set_quoted('PACKAGE_APPID', app_id)
meta_h.set_quoted('PACKAGE_BUILD', '@VCS_TAG@')
meta_h.set_quoted('PACKAGE_WEBSITE', app_url)
meta_h.set_quoted('PACKAGE_BUGREPORT', app_bugreport)
meta_h.set_quoted('PACKAGE_COPYRIGHT_DATES', '2016 - 2021')
meta_h.set_quoted('PACKAGE_DATA_DIR', data_dir)
meta_h.set_quoted('PACKAGE_CONFIG_DIR', sys_conf_dir)
meta_h.set_quoted('PACKAGE_CODESET', 'UTF-8')

build_id_generator = [
  join_paths(meson.source_root() , 'script', 'gen-build-id.sh'),
  meson.source_root(),
]

generated_meta_h = declare_dependency(
  sources: vcs_tag(
    command: build_id_generator,
    input: configure_file(output: 'srn-meta.h.in', configuration: meta_h),
    output: 'srn-meta.h',
    fallback: 'unknown',
  )
)

srcs += [
  'main.c',
  'srn-app.h',
  'srn-app.c',
]

deps = [
  dependency('gtk4', version: '>= 3.99.4'),
  dependency('glib-2.0', version: '>= 2.39.3'),
  dependency('gobject-introspection-1.0'),
  generated_meta_h,
]

exec = executable(app_exec, srcs,
  dependencies: deps,
  install: true,
  install_dir: bin_dir,
  export_dynamic: true,
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
