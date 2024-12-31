# JoybusProbe

This repo contains a simple debug probe that implements a basic bidirectional communication protocol over Joybus.

It's based on [PicoJoybus](https://github.com/Polprzewodnikowy/PicoJoybus/) by [Polprzewodnikowy](https://github.com/Polprzewodnikowy/).

# Usage

Flash the UF2 file to your Pico, then connect pin 34 (GPIO 28) to the data line on your N64 controller port. You might want to tie the grounds together, too.
Connect the Pico to your PC and it should appear as a USB TTY.

All of my software requires it to be controller port 3 for now.
