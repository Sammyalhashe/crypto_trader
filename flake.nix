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
                  ninja
                  zig
                  zls
                  cmake-language-server
                  doxygen
                  graphviz
                  direnv

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
                  if [[ -x "$(command -v conan)" ]]; then
                      conan profile detect || true
                  fi
                '';

                scripts.prepare.exec = ''
                  if [[ -x "$(command -v conan)" ]]; then
                      make prepare CONAN=1
                  else
                      make prepare GENERATOR=Ninja
                  fi
                '';
                scripts.build.exec = "make build";
                scripts.bo.exec = "make bo";
                scripts.fbuild.exec = "make fast-build";
                scripts.fbo.exec = "make fbo";
                scripts.clean.exec = "make clean";
                scripts.test.exec = "make test";
                scripts.run.exec = "make run";
                scripts.docs.exec = "cmake --build cmake.bld/Linux/full --target doc_doxygen";
                scripts.serve-docs.exec = "python3 -m http.server 8000 --directory cmake.bld/Linux/full/doc_doxygen/html";
              }
            ];
          };
        }
      );
    };
}
