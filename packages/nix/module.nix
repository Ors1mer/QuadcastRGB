# A NixOS module: installs the program and, unlike a plain package, also puts
# the udev rule in place so that the microphone is writable without superuser
# rights. Exposed by the flake as nixosModules.default.
self:

{ config, lib, pkgs, ... }:

let
  cfg = config.services.quadcastrgb;
in
{
  options.services.quadcastrgb = {
    enable = lib.mkEnableOption "the RGB control for HyperX Quadcast microphones";

    package = lib.mkOption {
      type = lib.types.package;
      default = self.packages.${pkgs.stdenv.hostPlatform.system}.quadcastrgb;
      defaultText = lib.literalExpression "quadcastrgb";
      description = "The package to install and to take the udev rule from.";
    };

    arguments = lib.mkOption {
      type = lib.types.listOf lib.types.str;
      default = [ ];
      example = [ "-b" "50" "solid" "4c0099" ];
      description = ''
        Command line arguments to apply on login, see the man page. The empty
        list, the default, only installs the program and sets no lights.

        Note that the microphone forgets the colors when it loses power, hence
        a login service rather than a system one.
      '';
    };
  };

  config = lib.mkIf cfg.enable {
    environment.systemPackages = [ cfg.package ];
    services.udev.packages = [ cfg.package ];

    systemd.user.services.quadcastrgb = lib.mkIf (cfg.arguments != [ ]) {
      description = "Set the RGB lights of the HyperX Quadcast microphone";
      wantedBy = [ "default.target" ];
      serviceConfig = {
        Type = "forking";
        ExecStart = "${lib.getExe cfg.package} ${lib.escapeShellArgs cfg.arguments}";
        # The program stops on its own whenever the microphone stops talking
        # back — muting it is enough on some models, unplugging it always is —
        # and it exits successfully doing so, hence Restart=always rather than
        # on-failure. This doubles as the way the lights come back after the
        # mic is replugged, since it forgets them when it loses power.
        Restart = "always";
        RestartSec = 5;
      };

      # No rate limiting: with no microphone attached the program exits at
      # once, and retrying every few seconds costs nothing but means the
      # lights appear by themselves once it is plugged back in.
      unitConfig.StartLimitIntervalSec = 0;
    };
  };
}
