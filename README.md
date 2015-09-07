# Pokemini
Pokemon Mini emulator for 3DS

This is a Pokemon Mini emulator for 3DS. I'ts in 3dsx and CIA format.

It's a port on 3DS of the excellent Pokemini v0.6 by JustBurn with platform files adapted from PSP and NDS versions. Everything you'll see on your 3DS if from JustBurn, I only ported SDL code to 3DS libs and made it run at fullspeed.

Warnings
=====
This is an alpha release, sometimes the program freezes and there are little things to fix in the CIA version.

Contents
=====
- PokeMini_3DS.c is the main platform file and PokeMini_3DS.h it's header. It's based on the platform files of the PSP and 3DS versions with some ugly tweak for frame timing
- PokeMini_3DS_sound.c is the file that handles sounds function and PokeMini_3DS.h it's header. It's platform specific as 3DS has no SDL libs yet.
- The other .c and .h files are mods or tweaks of official Pokemini v0.6, made to work with 3DS platform. Most of this mods could be integrated in the next Pokemini release if JustBurn wants (The mods are mostly related to the use of an alpha channel in 32bpp mode and for handling the rgb32 blit mode that is implemented but not selectable). 
- Everything else is 3DSX and CIA file formats related, you can learn about it on gbatemp.org, in the 3DS homebrew section.

How to build
=====

Dovnload the sources of Pokemini v0.6, creade a folder named 3DS inside the Platform subdirecory, copy the content of this repository inside and the run make to buld the 3DSX version, or run make CIA to buid both 3DSX and CIA versions.

Dependancies
=====

You need devkitPro uptdated with the last version of ctrulib.

You also need to download, compile and install Xerpi's sf2dlib (http://github.com/xerpi/sf2dlib).

Licence
=====

Pokemini is released under the following licence, everything made by me (if any) is public domain and I'll be happy if JustBurn find time to include my work (whel will be stable) in it distribution.


  PokeMini - Pokémon-Mini Emulator
  Copyright (C) 2009-2015  JustBurn

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
