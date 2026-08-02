{ lib, stdenv, libusb1 }:

stdenv.mkDerivation {
  pname = "quadcastrgb";
  version = "1.0.5"; # keep in sync with VERSION in the Makefile

  src = lib.cleanSource ../..;

  buildInputs = [ libusb1 ];

  makeFlags = [ "CC=${stdenv.cc.targetPrefix}cc" ];

  # The default install paths point into $HOME, which does not exist while
  # building; the Makefile lets us redirect them, same as the AUR package does.
  installFlags = [
    "BINDIR_INS=${placeholder "out"}/bin/"
    "MANDIR_INS=${placeholder "out"}/share/man/man1/"
  ];

  postInstall = ''
    install -Dm644 packages/nix/60-quadcastrgb.rules \
      $out/lib/udev/rules.d/60-quadcastrgb.rules
  '';

  meta = {
    description = "Set the RGB lights of HyperX Quadcast microphones";
    homepage = "https://github.com/Ors1mer/QuadcastRGB";
    license = lib.licenses.gpl2Only;
    mainProgram = "quadcastrgb";
    # The program builds on MacOS and FreeBSD as well, but neither is tested
    # here, so only Linux is claimed.
    platforms = lib.platforms.linux;
  };
}
