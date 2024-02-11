# Generate header file which contains meta info macro
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
meta_h.set_quoted('PACKAGE_AUTHOR', 'Shengyu Zhang')
meta_h.set_quoted('PACKAGE_EMAIL', 'i@silverrainz.me')
meta_h.set_quoted('PACKAGE_COPYRIGHT_DATES', '2016 - 2024')
meta_h.set_quoted('PACKAGE_DATA_DIR', data_dir)
meta_h.set_quoted('PACKAGE_CONFIG_DIR', sys_conf_dir)

if get_option('app_indicator')
  meta_h.set('ENABLE_APP_INDICATOR', 1)
endif

build_id_generator = [
  join_paths(meson.source_root() , 'script', 'gen-build-id.sh'),
  meson.source_root(),
]

generated_meta_h = declare_dependency(
  sources: vcs_tag(
    command: build_id_generator,
    input: configure_file(output: 'meta.h.in', configuration: meta_h),
    output: 'meta.h',
    fallback: 'unknown',
  )
)

srcs += [
  'config/manager.c',
  'config/password.c',
  'config/reader.c',
  'core/app.c',
  'core/app_config.c',
  'core/app_irc_event.c',
  'core/app_ui_event.c',
  'core/app_url.c',
  'core/chat.c',
  'core/chat_command.c',
  'core/chat_config.c',
  'core/chat_user.c',
  'core/login_config.c',
  'core/message.c',
  'core/server.c',
  'core/server_cap.c',
  'core/server_config.c',
  'core/server_state.c',
  'core/server_user.c',
  'core/srain.c',
  'core/user_config.c',
  'filter/filter.c',
  'filter/log_filter.c',
  'filter/pattern_filter.c',
  'filter/user_filter.c',
  'lib/command.c',
  'lib/command_test.c',
  'lib/extra_data.c',
  'lib/libecdsaauth/base64.c',
  'lib/libecdsaauth/keypair.c',
  'lib/libecdsaauth/op.c',
  'lib/i18n.c',
  'lib/log.c',
  'lib/markup_renderer.c',
  'lib/path.c',
  'lib/pattern_set.c',
  'lib/ret.c',
  'lib/utils.c',
  'lib/version.c',
  'render/mention_renderer.c',
  'render/mirc_colorize_renderer.c',
  'render/mirc_strip_renderer.c',
  'render/pattern_render.c',
  'render/render.c',
  'render/url_renderer.c',
  'sirc/io_stream.c',
  'sirc/sirc.c',
  'sirc/sirc_cmd_builder.c',
  'sirc/sirc_cmd.c',
  'sirc/sirc_config.c',
  'sirc/sirc_event_hdr.c',
  'sirc/sirc_context.c',
  'sirc/sirc_parse.c',
  'sirc/sirc_utils.c',
  'sui/nick_menu.c',
  'sui/sui_app.c',
  'sui/sui_buffer.c',
  'sui/sui.c',
  'sui/sui_channel_buffer.c',
  'sui/sui_chat_buffer.c',
  'sui/sui_common.c',
  'sui/sui_completion.c',
  'sui/sui_config.c',
  'sui/sui_connect_panel.c',
  'sui/sui_dialog_buffer.c',
  'sui/sui_event_hdr.c',
  'sui/sui_join_panel.c',
  'sui/sui_message.c',
  'sui/sui_message_list.c',
  'sui/sui_misc_message.c',
  'sui/sui_notification.c',
  'sui/sui_prefs_dialog.c',
  'sui/sui_recv_message.c',
  'sui/sui_send_message.c',
  'sui/sui_server_buffer.c',
  'sui/sui_side_bar.c',
  'sui/sui_side_bar_item.c',
  'sui/sui_theme.c',
  'sui/sui_url_previewer.c',
  'sui/sui_user.c',
  'sui/sui_user_list.c',
  'sui/sui_window.c',
]

deps = [
  dependency('gtk+-3.0', version: '>= 3.22.15'),
  dependency('glib-2.0', version: '>= 2.39.3'),
  dependency('libconfig', version: '>= 1.5'),
  dependency('libsoup-2.4'),
  dependency('openssl'),
  dependency('libsecret-1'),
  generated_meta_h,
]

if get_option('app_indicator')
  deps += [dependency('ayatana-appindicator3-0.1')]
endif

incdirs = [
  include_directories('inc'),
  include_directories('config'),
  include_directories('core'),
  include_directories('filter'),
  include_directories('render'),
  include_directories('lib'),
  include_directories('sirc'),
  include_directories('sui'),
]

executable(
  app_exec, srcs,
  include_directories: incdirs,
  dependencies: deps,
  install: true,
  install_dir: bin_dir,
  gui_app: true
)
