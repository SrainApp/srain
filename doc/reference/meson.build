# XXX: gi-docgen needs a deps file

toml_conf = configuration_data()
toml_conf.set('app_version', meson.project_version())
toml_conf.set('app_url', app_url)
toml_conf.set('app_desc', app_desc)

gidocgen = find_program('gi-docgen')

md_files = []

srn_toml = configure_file(
  input: 'srain.toml.in',
  output: 'srain.toml',
  configuration: toml_conf
)

custom_target('srain-doc',
  input: [srn_toml, srn_gir[0]],
  output: 'srain',
  command: [
    gidocgen,
    'generate',
    '--config=@INPUT0@',
    '--output-dir=@OUTPUT@',
    '--no-namespace-dir',
    '--content-dir=@0@'.format(meson.current_source_dir()),
    '@INPUT1@',
  ],
  depend_files: [md_files],
  build_by_default: true,
  install: true,
  install_dir: pkg_ref_dir,
)
