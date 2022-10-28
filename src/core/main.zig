const std = @import("std");

const cfuncs = @cImport({
        @cInclude("srain.h");
        });

pub fn main() void {
    var rc = cfuncs.c_main();
    std.log.info("srain's c_main returns {}", .{rc});
}
