Color files are straightforward text files that you can edit in any
text editor, such as Notepad.

Comments are designated by "#", and will last until the end of the line.
Unless stated otherwise, add whitespace around everything.
See the Minepedia for blocks: http://minecraftwiki.net/wiki/Blocks,_items_and_data_values

QUICK START
===========
Add the following three lines to a file (without the quotes):
"cnbtcolors v1
options: shadesides
-------------"

Then, for each block type, add a line that looks like this (again,
without the quotes):
"1 t 0,0,0 T 128,128,128"
This will make the stone block (1) be black at the bottom of
the map (t 0,0,0) and grey at the top of the map (T 128,128,128).
To make a color transparent, add a fourth value like this:
"0,0,0,128"
This will make the color black and partially transparent. The closer
the value to 0, the more transparent it is.

FULL DESCRIPTION
================
The first line of the file is the version. This must always be there.
ex:    "cnbtcolors v1"

The second line contains options. The "options:" must always be there,
even if no options are used.
Available options:
    "shadesides" - shades the sides of blocks by a specific color so
                   they are darker. Default is off.
    "alternate"  - alternates the brightness of each level so they are
                   easier to tell apart in angled. Default is off.
ex:   "options: shadesides"
      "options:"

After the options, there is a separator consisting of multiple "-".
This must always be there.
ex: "-----------------"

After the separator, there is a list of blocks with their colors.
Each line has the following structure:
[block ID] [flag color1] [flag color2] ...
The block ID corresponds to the Minecraft block. For example, block 0
is air. There are four special values for this:
        "b" - the background color for the image
        "e" - the color used when a block is in the map, but not in the colorfile
        "s" - the color used to shade sides when the option is selected
        "a" - the color used when shading a block on every other level
Each color consists of a type and a color triplet. The type is
one of:
    "T" - top (of the block) bright (towards the top of the map)
    "t" - top (     ""     ) dark   (towards the bottom of the map)
    "S" - bottom (     ""     ) bright
    "s" - bottom (     ""     ) dark
If not all of the types are specified, they will be filled in based on
the other types that have been provided. If you only provide one type / color,
all of the other types will have the same color. For the background,
the color for the top at block heigh 127 will be used only.

The color triplet can have either three or four values. The first
three are R, G, and B. The optional fourth value is the alpha channel.
By default, the alpha is set to 255.

In the color triplet, NO SPACES are allowed.

ex: "0 t 0,0,0,0" makes the air block (0) transparent
    "e t 255,0,255" makes the error color to be bright magenta
    "3 t 0,0,0 T 255,255,0" makes the dirt block (3) black at the bottom and yellow at the top
    "2 t 0,255,0 s 0,0,0 S 255,255,0" makes the grass block (2) bright green on the top, with the side behaving the same as the dirt - black at the bottom, yellow at the top
