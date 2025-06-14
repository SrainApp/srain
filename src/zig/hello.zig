const std = @import("std");

export fn zig_hello() void {
    const stdout = std.io.getStdOut().writer();
    _ = stdout.print("Hello from Zig!\n", .{}) catch {};
}
