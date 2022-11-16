% QUADCASTRGB(1) quadcastrgb 0.1.9
% Ors1mer <https://gitlab.com/Ors1mer>, <https://github.com/Ors1mer>
% August 2022

# NAME
quadcastrgb - change RGB mode for the microphone HyperX Quadcast S

# SYNOPSIS
**quadcastrgb** [-h] [-v] [-a|-u|-l] [-b bright] [-s speed] [-d delay] mode [COLORS]...

# DESCRIPTION
**quadcastrgb** looks for a connected Quadcast S micro, then connects to it and
changes the rgb mode and colors depending on the given parameters.

There are two possible ways of how the program behaves: writes the color into
the microphone or becomes a demon process and continuously sends data to the
device. The first behavior is only done when solid mode is used. Any other mode
causes the program to become a demon that should be killed with SIGINT once
the user doesn't need it.

Available modes:  
- solid  
- blink  
- cycle (yet to be done)  
- lightning (yet to be done)  
- wave (yet to be done)  

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
: The microphone isn't connected

**4**
: Failed to open the microphone descriptor. Consider running the program with
the root privileges (not recommended) or creating a dev rule (recommended)

**5**
: Failed to transfer some packets

# CAVEATS
It isn't possible to edit only one (upper or lower) diode group without
changing the other. If a mode set to only one diode group, the other is set to
solid black (i. e. no color).

# BUGS
If any mode except solid is set, the microphone may display random unexpected
colors once in a while. This doesn't mean that it is broken in any way as the
device is well-bricking-protected. The solutions are: try executing the program
several more times, reconnect the device.

# COPYRIGHT
Copyright (C) 2022 Ors1mer. License GPLv2: GNU GPL version 2 only
<https://www.gnu.org/licenses/gpl-2.0.en.html>.
This is free software: you are free to change and redistribute it. There is
NO WARRANTY, to the extent permitted by law.
