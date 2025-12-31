# NixSupport.cmake
#
# This module encapsulates build configuration adjustments required for
# compatibility with the Nix package manager environment.
#
# Nix stores libraries in non-standard paths (e.g., /nix/store/...).
# Standard CMake builds often strip RPATHs from build-tree binaries, assuming
# that libraries are in standard system locations (like /usr/lib) or that
# LD_LIBRARY_PATH will be set.
#
# In a Nix environment, binaries (especially test executables run during the build)
# fail to find shared libraries (like libgtest.so) unless the RPATH is explicitly
# preserved and pointing to the Nix store paths.

# We only apply these fixes on UNIX systems that are not macOS (Apple),
# as macOS uses a different mechanism (LC_RPATH) and usually handles this differently.
# This check is a heuristic; a more robust check would look for the NIX_STORE environment variable.
if(UNIX AND NOT APPLE)
    message(STATUS "Configuring RPATH for Nix/Linux environment compatibility...")

    # CMAKE_SKIP_BUILD_RPATH:
    # Default is FALSE. If set to TRUE, CMake will not add any RPATH to the
    # executables in the build tree. We strictly ensure this is FALSE so that
    # the build-tree binaries contain the paths to their dependencies.
    set(CMAKE_SKIP_BUILD_RPATH FALSE)

    # CMAKE_BUILD_WITH_INSTALL_RPATH:
    # Default is FALSE. If TRUE, the build-tree binaries are built with the
    # RPATH they will have when installed. This is often empty or relative.
    # We want the build-tree RPATH (which includes absolute paths to dependencies),
    # so we ensure this is FALSE.
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

    # CMAKE_INSTALL_RPATH_USE_LINK_PATH:
    # Default is FALSE. If TRUE, CMake appends the paths of any linked shared libraries
    # (that are outside the project's build directory) to the RPATH of the installed
    # binary. While this primarily affects the *install* tree, setting it to TRUE
    # is generally good practice in Nix to ensure installed binaries also find their deps.
    # More critically for the build tree, it signals intent to rely on RPATHs derived
    # from link directories.
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    # Force the build RPATH to include the directories where libraries are found.
    # This is crucial for Nix, where libraries are in /nix/store/... and not in
    # standard system paths.
    # By default, CMake might strip these if it thinks they are system paths,
    # or if CMAKE_SKIP_BUILD_RPATH is accidentally true.
    # We explicitly tell CMake to use the link paths for the build RPATH as well.
    list(APPEND CMAKE_BUILD_RPATH "${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}")
endif()
