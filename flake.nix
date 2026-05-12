{
  description = "C++ dev environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.${system}.default = pkgs.mkShell {
      nativeBuildInputs = with pkgs; [ gcc cmake ninja pkg-config ];
      buildInputs       = with pkgs; [ boost gbenchmark ];
      packages          = with pkgs; [ gdb clang-tools ];

      #shellHook = ''
      #export CMAKE_PREFIX_PATH="${pkgs.gbenchmark}":$CMAKE_PREFIX_PATH
      #'';
      };
    };
}