const std = @import("std");

export fn add(a: c_int, b: c_int) c_int {
    return a + b;
}

test "math_test" {
    const result = 3;
    try std.testing.expect(result == add(1, 2));
}
