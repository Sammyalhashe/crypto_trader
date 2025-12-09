const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const module = b.createModule(.{
        .root_source_file = b.path("order_book.zig"),
        .target = target,
        .optimize = optimize,
    });

    const lib = b.addLibrary(.{
        .name = "order_book",
        .linkage = .dynamic,
        .root_module = module,
    });

    lib.installHeadersDirectory(b.path("."), "zig", .{ .include_extensions = &.{".h"} });
    b.installArtifact(lib);
}
