# TOML Configuration
## Table of Contents
- [Overview](#overview)
- [Configuration Location](#configuration-location)
- [Visual Mapping](#visual-mapping)
- [TOML](#toml)
  - [Defaults](#defaults)
  - [Aliases](#aliases)
  - [Unkown Windows](#unkown-windows)
  - [Application Tables](#application-tables)
- [Helpful Commands](#helpful-commands)
- [Debugging](#debugging)
## Overview
`xorg-discord-rpc` determines your activity by reading Xorg window properties like:

- Foucsed Window Class `WM_CLASS`
- Focused Window Title `WM_TITLE`

This allows the configuration file to:

- Choose what information appears as Details and State
- Map window classes to friendly app names
- Assign Discord Rich Presence image assets
- Customize behavior for unknown or empty workspaces
## Configuration Location 
Default location:
```sh
~/.config/xorg-presence/config.toml
```
You may load a custom file using:
```sh
xorg-discord-rpc -l /path/to/config.toml
```
## Visual Mapping
The image below shows how each configuration field maps to each RPC element.

<img src="assets/member-view-status.png" align="right" width="500">
<img src="assets/current-activity.png" align="center" width="435">

- WHITE -> Large Image
- GREEN -> Small Image
- RED -> Details
- BLUE -> State
- YELLOW ->  Application Title
## TOML
### Defaults
The `[defaults]` table controls how RPC behaves when no application-specific rule matches.

**IMPORTANT:** 


If you set `large_image` to 'wm', the program will grab the name of your current window manager.
Make sure you have an image set with the name of your window manager in your assets or else the 
problem may encounter issues when trying to run.
```toml
[defaults]
details = "app"    # "app", "class", or "title"
state = "title"    # "app", "class", "title", or empty for none
large_image = "wm"
small_image = ""
```
### Aliases
Aliases map raw Xorg window classes to your application names.

**IMPORTANT:** 


If your application doesn't contain a toml-friendly name just rewrite it without any special characters. 
- Use a command like `wmctrl -lx` to inspect window classes
- Aliases must match keys under `[apps.*]`
```toml
[aliases]
firefox = "browser"
zen = "browser"
emacs = "editor"
orgwezfurlongwezterm = "terminal" # WM_CLASS is "org.wezfurlong.wezterm"
alacritty = "terminal"
rosalinesmupengui = "gaming" # WM_CLASS is "Rosalie's Mupen GUI"
```
### Unkown Windows
When no window is focused (like an empty workspace, or if no match is found), the `[unknown]` table is used.
```toml
[unknown]
name = "Empty Workspace"
small_image = "gnu"
```
### Application Tables
Each application profile lives under `[apps.<name>]`.
```toml
[apps.browser]
small_image = "zen"

[apps.editor]
name = "DOOM Emacs"
small_image = "doom"

[apps.terminal]
large_image = "archlinux"
small_image = "alacritty"
```
**Fields:**


- `name` Overrides the displayed application name
- `large_image` Overrides the default large image 
- `small_image`

Unset fields fall back to `[defaults]`.
## Helpful commands 
```sh
wmctrl -lx            # list window classes & titles
xprop -id <WINID>     # inspect window properties
xwininfo -id <WINID>  # detailed window info
```

## Debugging
If something doesn’t appear:
- Check asset names match Discord Developer Portal
- Double-Check `WM_CLASS` spelling
