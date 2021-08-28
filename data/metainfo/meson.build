metainfo_conf = configuration_data()
metainfo_conf.set('app_id', app_id)
metainfo_conf.set('app_pretty_name', app_pretty_name)
metainfo_conf.set('app_desc', app_desc)
metainfo_conf.set('app_url', app_url)
metainfo_conf.set('app_exec', app_exec)
metainfo_conf.set('gettext_package', app_name)
metainfo_conf.set('desktop_id', app_id + '.desktop')
metainfo = i18n.merge_file(
  type: 'xml',
  input: configure_file(
    input: 'srain.metainfo.xml.in.in',
    output: 'srain.metainfo.xml.in',
    configuration: metainfo_conf,
  ),
  output: '@0@.metainfo.xml'.format(app_id),
  po_dir: po_dir,
  install: true,
  install_dir: join_paths(data_dir, 'metainfo')
)
