{
  inputs = {
    nixpkgs.url = "github:cachix/devenv-nixpkgs/rolling";
    systems.url = "github:nix-systems/default";
    devenv.url = "github:cachix/devenv";
    devenv.inputs.nixpkgs.follows = "nixpkgs";
  };

  nixConfig = {
    extra-trusted-public-keys = "devenv.cachix.org-1:w1cLUi8dv3hnoSPGAuibQv+f9TZLr6cv/Hm9XgU50cw=";
    extra-substituters = "https://devenv.cachix.org";
  };

  outputs =
    {
      self,
      nixpkgs,
      devenv,
      systems,
      ...
    }@inputs:
    let
      forEachSystem = nixpkgs.lib.genAttrs (import systems);
    in
    {
      packages = forEachSystem (system: {
        devenv-up = self.devShells.${system}.default.config.procfileScript;
        devenv-test = self.devShells.${system}.default.config.test;
      });

      devShells = forEachSystem (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = devenv.lib.mkShell {
            inherit inputs pkgs;
            modules = [
              {
                # https://devenv.sh/reference/options/
                packages = with pkgs; [
                  cmake
                  ccache
                  gcc
                  clang
                  clang-tools
                  cppcheck
                  python3
                  ninja
                  zig
                  zls
                  cmake-language-server
                  doxygen
                  graphviz
                  direnv
                  git

                  # build dependencies
                  boost-build
                  spdlog
                  openssl
                  nlohmann_json
                  boost
                  gtest
                  sqlitecpp
                  sqlite
                ];

                scripts = {
                  prepare.exec = ''
                    if [[ -x "$(command -v conan)" ]]; then
                        make prepare CONAN=1
                    else
                        make prepare GENERATOR=Ninja
                    fi
                  '';
                  build.exec = "make build";
                  bo.exec = "make bo";
                  fbuild.exec = "make fast-build";
                  fbo.exec = "make fbo";
                  clean.exec = "make clean";
                  test.exec = "make test";
                  run.exec = "make run";
                  docs.exec = "cmake --build cmake.bld/Linux/full --target doc_doxygen";
                  serve-docs.exec = "python3 -m http.server 8000 --directory cmake.bld/Linux/full/doc_doxygen/html";
                };

                git-hooks.hooks.style-check.enable = true;
                git-hooks.hooks.style-check.entry = "python3 style_checker.py";
                git-hooks.hooks.style-check.package = pkgs.python3;
              }
            ];
          };
        }
      );
    };
}
