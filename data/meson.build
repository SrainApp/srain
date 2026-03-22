i18n = import('i18n')
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

gnome = import('gnome')

srcs += gnome.compile_resources('srain-resources', 'srain.gresource.xml')

if get_option('gtk4_experimental')
  blueprint_compiler = find_program('blueprint-compiler', required: false)
  if blueprint_compiler.found()
    gtk4_blueprints = custom_target('gtk4-blueprints',
      input: files(
        'ui-gtk4/app_menu.blp',
        'ui-gtk4/side_bar_item.blp',
        'ui-gtk4/user_list.blp',
      ),
      output: [
        'app_menu.ui',
        'side_bar_item.ui',
        'user_list.ui',
      ],
      command: [
        blueprint_compiler,
        'batch-compile',
        '@OUTDIR@',
        '@CURRENT_SOURCE_DIR@',
        '@INPUT@',
      ],
      build_by_default: true,
    )
  else
    warning('blueprint-compiler not found; skipping gtk4 blueprint generation')
  endif
endif

subdir('metainfo')
subdir('icons')

install_subdir('themes', install_dir: pkg_data_dir)
install_data('builtin.cfg', install_dir: pkg_sys_conf_dir)
