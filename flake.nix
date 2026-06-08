{
  description = "C++ dev environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.${system}.default = pkgs.mkShell {
      nativeBuildInputs = with pkgs; [ nodejs gcc cmake ninja pkg-config ];
      buildInputs       = with pkgs; [ boost gbenchmark ];
      packages          = with pkgs; [ gdb clang-tools
      (pkgs.writeShellScriptBin "start-sudoku" ''
      set -e
      echo "start building sudoku server..."
      mkdir -p build
      cd build
      cmake ..
      make
      cd ..

      pids=()
      cleanup() {
        kill "''${pids[@]}" 2>/dev/null
        wait "''${pids[@]}" 2>/dev/null
      }
      trap cleanup EXIT INT TERM

      echo "starting sudoku server..."
      ./build/server 127.0.0.1 8080 1 & pids+=(''$!)

      echo "starting webserver..."
      cd src/
      node app.js & pids+=(''$!)

      echo ""
      echo "==> Sudoku running at http://localhost:3000"
      echo "==> Press Ctrl+C to stop both servers"
      wait
      '')
      ];

      shellHook = ''
      npm install express
      npm install ejs
      '';
      };

      packages.${system}.server = pkgs.stdenv.mkDerivation {
        pname = "sudoku-server";
        version = "1.0";
        src = ./.;
        nativeBuildInputs = with pkgs; [ cmake ninja pkg-config ];
        buildInputs       = with pkgs; [ boost gbenchmark ];
      };
    };
}
