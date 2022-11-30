# QuadcastRGB
## About
The program (or driver, if you wish) allows changing the RGB colors of the
microphone HyperX Quadcast S just like NGenuity does. Should support all
platforms, Unix-like preferably. Only Linux and partially FreeBSD were tested
so far.

Available modes are *solid, blink, cycle, wave, lightning, and pulse*. The
program runs as a daemon for each of these modes except *solid*.

### Features:
- *free software (GPL-2.0-only)*
- *cross-platform*
- *cli*
- *daemon*

### Things yet to be done:
- *aur, deb, rpm packages*
- *several microphones support*
- *extra modes: ~pulse,~ visualizer*
- *~refuse to work if another instance is running~*
- *~the original modes: solid, blink, cycle, wave, lightning~*
- *~brightness & speed~*
- *~delay - **blink**-specific option~*
- *~actual installation~*

### Examples:
```bash
# Default solid color (#f20000, red) for the whole micro:
quadcastrgb solid 
# Random blinking colors:
quadcastrgb blink
# Default cycle (rainbow) mode for the whole micro:
quadcastrgb -a cycle 
# Purple color for the upper part and yellow for the lower:
quadcastrgb -u solid 4c0099 -l solid ff6000 
# Default cycle mode for the upper diode with 50% brightness and yellow lightning for the lower:
quadcastrgb -u -b 50 cycle -l lightning ff6000 
```
## Install
### Compiling from source
If you are lucky, this should be enough:
```bash
make install # for linux
gmake install OS=freebsd # for freebsd
```
## Basic problems during&after Install
### Problem 1: make failed
Check the dependencies:  
 - gcc v12.2.0 OR clang v14.0.6 (most versions should do fine)
 - libusb-1.0 v1.0.26
 - gettext v0.21-2

### Problem 2: command not found
Check the $PATH and manpath. The program follows XDG specifications, so the
binary is stored in $HOME/.local/bin (should be in $PATH) and the man is in
$HOME/.local/share/man (should be in $MANPATH). It is possible to move them,
of course.

### Problem 3: couldn't open the microphone
Check the error code. If it is 4, try running the program under superuser
privileges. If that was the problem, you should eventually create a dev rule
for the microphone to allow certain users access to it.

### How to create the udev rule
```bash
# Here the rules are stored:
cd /etc/udev/rules.d 
# Do under superuser:
vi 10-quadcast-perm.rules 
```
Write this line, save & exit (:wq):
```bash
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0951", ATTRS{idProduct}=="171f", MODE="0660", GROUP="hyperrgb" 
```
Now the microphone is accessible for the group "hyperrgb". Add your user to the
group and it's done.

### Problem 4: launching the program a second time does nothing
The problem probably is that the previous instance is still running. Kill it
with *kill* or *killall*.

### How to end the program
Simply kill the process by name:
```bash
killall quadcastrgb
```

