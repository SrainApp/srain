# Mod should not reslove symbol in srn_exec
gir_extra_args = [
  '--cflags-begin',
  '-Wl,--as-needed',
  '-Wl,--allow-shlib-undefined',
  '--cflags-end',
]

dep = declare_dependency(link_args: ['--as-needed', '--allow-shlib-undefined'])

subdir('sirc')
# subdir('stg')
