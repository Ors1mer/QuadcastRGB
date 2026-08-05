{
  description = "Set the RGB lights of HyperX Quadcast microphones";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forEachSystem = f:
        nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in
    {
      packages = forEachSystem (pkgs: rec {
        quadcastrgb = pkgs.callPackage ./packages/nix/package.nix { };
        default = quadcastrgb;
      });

      overlays.default = final: prev: {
        quadcastrgb = final.callPackage ./packages/nix/package.nix { };
      };

      # Installing the package alone is not enough: without the udev rule the
      # microphone is only writable by root. The module does both.
      nixosModules.default = import ./packages/nix/module.nix self;

      devShells = forEachSystem (pkgs: {
        default = pkgs.mkShell {
          packages = [ pkgs.gcc pkgs.gnumake pkgs.libusb1 pkgs.gettext ];
        };
      });
    };
}
