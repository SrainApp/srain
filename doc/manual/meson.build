# meson.build for Sphinx documentation
#

doc_builders = get_option('doc_builders')
sphinx = find_program('sphinx-build', required: doc_builders.length() != 0)
sphinx_source_dir = meson.current_source_dir()
sphinx_build_dir = meson.current_build_dir()

# Build documentation
foreach b: doc_builders
  run_command(sphinx, '-b', b, sphinx_source_dir, join_paths(sphinx_build_dir, b))
endforeach

# `in` operator requires meson 0.49.0
if doc_builders.contains('man')
  install_man(join_paths(sphinx_build_dir, 'man', 'srain.1'))
endif

# `in` operator requires meson 0.49.0
if doc_builders.contains('html')
  install_subdir(join_paths(sphinx_build_dir, 'html'), install_dir: pkg_man_dir)
endif
