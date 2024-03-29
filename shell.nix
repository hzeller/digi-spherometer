# This is a nix-shell for use with the nix package manager.
# If you have nix installed, you may simply run `nix-shell`
# in this repo, and have all dependencies ready in the new shell.

{ pkgs ? import <nixpkgs> {} }:
pkgs.mkShell {
  buildInputs = with pkgs;
    [
      git
      python3
      kicad
      python310Packages.kicad
      zip
      pkgsCross.avr.buildPackages.gcc8
      avrdude
      openscad
    ];
}
