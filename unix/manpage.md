% QUADCASTRGB(1) quadcastrgb 0.1
% Ors1mer <https://gitlab.com/Ors1mer>, <https://github.com/Ors1mer>
% August 2022

# NAME
quadcastrgb - change RGB mode for the microphone HyperX Quadcast S

# SYNOPSIS
**quadcastrgb** [-h] [-v] [-a|-u|-l] [-b bright] [-s speed] mode [COLORS]...

# DESCRIPTION
**quadcastrgb** looks for a connected Quadcast S micro, then connects to it and
changes the rgb mode and colors depending on the given parameters. By default,
if no error occured writes nothing and finishes successfully, else writes an
error message to stderr and finishes with an error code.

Available modes:  
- solid  
- blink (not supported yet)  
- cycle (not supperted yet)  
- lightning (not supported yet)  
- wave (not supported yet)  

# OPTIONS
## General options
**-h**, **--help**
: Display help message and version, then exit.

**-v**, **--verbose**
: Explain what is being done.

## Light diode groups
**-a**, **--all**
: Apply the following options for both upper & lower diodes.
This is the default state, it may be omitted.

**-u**, **--upper**
: Apply the following options to the upper diode only.

**-l**, **--lower**
: Apply the following options to the lower diodes only.

## Additional parameters
**-b**
: (Not supported.) Set the brightness in percents 0 through 100 (100 by default)
for the specified mode.

**-s**
: (Not supported.) Set speed of a gradient/animation, integer 1-10 (5 by
default). Does nothing in solid mode.

# EXAMPLES
**quadcastrgb solid**
: Set default solid color (#ff0000) for the whole micro

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
: Some argument error (i. e. typo or a wrong argument specified).

**2**
: A libusb function error (probably due to failed memory allocation).

**3**
: The microphone isn't connected.

**4**
: Failed to open the microphone descriptor. Consider running the program with
the root privileges (not recommended) or creating a dev rule (recommended).

**5**
: Failed to transfer some packets.

# CAVEATS
It isn't possible to edit only one (upper or lower) diode group without
changing the other. If a mode set to only one diode group, the other is set to
solid black (i. e. no color).

# COPYRIGHT
Copyright © 2022 Ors1mer. License GPLv3+: GNU GLP version 3 or later
<https://gnu.org/licenses/gpl.html>. This is free software: you are free to
change and redistribute it. There is NO WARRANTY, to the extent permitted by
law.
