# peewee

a port of the ATtiny13 solarBird firmware to the ATtiny85 based FOSDEM-85.

called peewee the penguin.

<img src="pics/peewee.jpg">

connect the solar cell (must not exceed 5V!) to or instead of the battery connector. bonus: there's already a diode that protects from feeding current in the solar cell in darkness.

connect the piezo tweeter to `GND` and `#0`

# origin

don't miss the analog version:
https://wiki.sgmk-ssam.ch/wiki/Solar_bird


main repo:
https://github.com/schaum/solarBird

randomisation:
https://github.com/fifthepoch/solarBird

# bootloader

the FOSDEM-85 comes with micronucleus as a bootloader which is great, but we prefer a slightly adopted version for a solar peewee. see the directory `bootloader` for more info.

