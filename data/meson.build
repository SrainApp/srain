desktop_conf = configuration_data()
desktop_conf.set('app_id', app_id)
desktop_conf.set('app_pretty_name', app_pretty_name)
desktop_conf.set('app_generic_name', app_generic_name)
desktop_conf.set('app_desc', app_desc)
desktop_conf.set('app_exec', app_exec)
desktop_conf.set('icon', app_id)
desktop = i18n.merge_file(
  type: 'desktop',
  input: configure_file(
    input: 'srain.desktop.in.in',
    output: 'srain.desktop.in',
    configuration: desktop_conf
  ),
  output: '@0@.desktop'.format(app_id),
  po_dir: po_dir,
  install: true,
  install_dir: join_paths(data_dir, 'applications')
)

subdir('metainfo')
subdir('icons')
