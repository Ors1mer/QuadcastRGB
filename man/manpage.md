% QUADCASTRGB(1) quadcastrgb 1.0.1
% Ors1mer <ors1mer_dev [[at]] proton.me>
% 2023 February 20

# NAME
quadcastrgb - set RGB mode for the microphone HyperX Quadcast S

# SYNOPSIS
**quadcastrgb** [-h] [-v] [-a|-u|-l] [-b bright] [-s speed] [-d delay] mode [COLORS]...

# DESCRIPTION
**quadcastrgb** looks for a Quadcast S micro, then connects to it and
sends colors to display depending on the given parameters.

The program becomes a daemon that continuously transfers data to the device.
Should be killed with SIGTERM or SIGINT once the user doesn't need it. It also
ends automatically if can't send data to the device anymore (e. g. the mic was
unplugged)

Available modes:  
- solid  
- blink  
- cycle  
- wave  
- lightning  
- pulse

# OPTIONS
## General options
**-h**, **--help**
: Display help message and version, then exit

**-v**, **--verbose**
: Explain what is being done

## Light diode groups
**-a**, **--all**
: Apply the following options for both upper & lower diodes.
This is the default state, it may be omitted

**-u**, **--upper**
: Apply the following options to the upper diode only

**-l**, **--lower**
: Apply the following options to the lower diodes only

## Additional parameters
**-b**
: Set the brightness in percents 0 through 100 (100 by default) for the
specified mode

**-s**
: Set speed of a gradient/animation, integer 1-100 (81 by default).
Ignored in solid mode

**-d**
: Set delay for the blink mode, integer 1-100 (10 by default).
Ignored in all other modes

# EXAMPLES
**quadcastrgb solid**
: Set default solid color (#f20000) for the whole micro

**quadcastrgb blink**
: Set random blinking colors

**quadcastrgb -a cycle**
: Set default cycle (rainbow) mode for the whole micro

**quadcastrgb -u solid 4c0099 -l solid ff6000**
: Set purple color for the upper part and yellow for the lower

**quadcastrgb -u -b 50 cycle -l lightning ff6000**
: Set default cycle mode for the upper diode with 50% brightness and yellow
lightning for the lower

# EXIT VALUES
**0**
: Success

**1**
: Some argument error (i. e. typo or a wrong argument specified)

**2**
: A libusb function error (probably due to failed memory allocation)

**3**
: The microphone isn't connected or couldn't be detected by the program

**4**
: Failed to open the microphone descriptor. Consider running the program with
the root privileges or creating a dev rule. Also, stop all programs that might
use the device

**5**
: Failed to transfer some packets

# CAVEATS
In order for a new instance to work the previous one must be killed.

It is possible to use '#rrggbb' notation for colors, but the '#' must be
scaped to be parsed successfully by shell. In order to do this escape this
character: '\#rrggbb', or write the hexcolor in double quotes.

# COPYRIGHT
Copyright (C) 2023 Ors1mer. License GPLv2: GNU GPL version 2 only
<https://www.gnu.org/licenses/gpl-2.0.en.html>.
This is free software: you are free to change and redistribute it. There is
NO WARRANTY, to the extent permitted by law.
