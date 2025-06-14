zig_srcs = [
  'hello.zig',
]

zig_objs = [] # for use by ../meson.build

foreach src : zig_srcs
  filename = src.split('.')[0]
  obj = custom_target(
    'zig_' + filename,
    output: filename + '.o',
    input: src,
    command: ['zig', 'build-obj', '@INPUT@', '-OReleaseSafe', '-femit-bin=@OUTPUT@'],
    build_by_default: true
  )
  zig_objs += [obj]
endforeach
