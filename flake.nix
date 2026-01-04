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
                  mold
                  lld
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

                enterShell = ''
                  echo -e "\n\033[1;36mCrypto Trader Dev Environment\033[0m"
                  echo -e "\033[1;34mAvailable commands:\033[0m"
                  echo -e "  \033[1;32mprepare\033[0m     - Prepare the build directory"
                  echo -e "  \033[1;32mbuild\033[0m       - Build the project (includes prepare)"
                  echo -e "  \033[1;32mfbuild\033[0m      - Fast build (targets executable only)"
                  echo -e "  \033[1;32mbo\033[0m          - Build Only (skips prepare)"
                  echo -e "  \033[1;32mfbo\033[0m         - Fast Build Only (skips prepare)"
                  echo -e "  \033[1;32mclean\033[0m       - Clean build artifacts"
                  echo -e "  \033[1;32mtest\033[0m        - Run tests"
                  echo -e "  \033[1;32mrun\033[0m         - Run the main application"
                  echo -e "  \033[1;32mstyle-check\033[0m - Run style checks"
                  echo -e "  \033[1;32mdocs\033[0m        - Generate documentation"
                  echo -e "  \033[1;32mserve-docs\033[0m  - Serve documentation on port 8000"
                  echo ""
                '';

                scripts = {
                  prepare.exec = ''
                    if [[ -x "$(command -v conan)" ]]; then
                        make prepare CONAN=1
                    else
                        make prepare GENERATOR=Ninja
                    fi
                  '';
                  build.exec = "make build";
                  style-check.exec = "make style-check";
                  bo.exec = "make bo";
                  fbuild.exec = "make fast-build";
                  fbo.exec = "make fbo";
                  clean.exec = "make clean";
                  test.exec = "make test";
                  run.exec = "make run";
                  docs.exec = "cmake --build cmake.bld/Linux/full --target doc_doxygen";
                  serve-docs.exec = "python3 -m http.server 8000 --directory cmake.bld/Linux/full/doc_doxygen/html";
                };
              }
            ];
          };
        }
      );
    };
}
