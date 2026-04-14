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
resource_dependencies = []
resource_source_dirs = [meson.current_source_dir()]

if get_option('gtk4_experimental')
  blueprint_compiler = find_program('blueprint-compiler', required: false)
  if blueprint_compiler.found()
    gtk4_blueprints = custom_target('gtk4-blueprint-files',
      input: files(
        'ui-gtk4/app_menu.blp',
        'ui-gtk4/buffer.blp',
        'ui-gtk4/connect_panel.blp',
        'ui-gtk4/image_window.blp',
        'ui-gtk4/join_panel.blp',
        'ui-gtk4/message_list.blp',
        'ui-gtk4/prefs_dialog.blp',
        'ui-gtk4/side_bar_item.blp',
        'ui-gtk4/user_list.blp',
        'ui-gtk4/url_previewer.blp',
        'ui-gtk4/window.blp',
      ),
      output: '.',
      command: [
        blueprint_compiler,
        'batch-compile',
        '@OUTPUT@',
        '@CURRENT_SOURCE_DIR@',
        '@INPUT@',
      ],
    )
    run_target('gtk4-blueprints',
      command: ['true'],
      depends: [gtk4_blueprints],
    )
    resource_dependencies += gtk4_blueprints
    resource_source_dirs += meson.current_build_dir()
  else
    warning('blueprint-compiler not found; skipping gtk4 blueprint generation')
  endif
endif

srcs += gnome.compile_resources('srain-resources', 'srain.gresource.xml',
  source_dir: resource_source_dirs,
  dependencies: resource_dependencies,
)

subdir('metainfo')
subdir('icons')

install_subdir('themes', install_dir: pkg_data_dir)
install_data('builtin.cfg', install_dir: pkg_sys_conf_dir)
