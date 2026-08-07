# micronucleus bootloader on the FOSDEM-85 to run peewee

## current research

the lambda diode research kitchen in the too hot summer of 2026 has performed a port of the peewee firmware, formerly known as coconut research lab's solarbird from the ATtiny45 to FOSDEM-85's ATtiny85, and furthermore evaluated following results:

- the micronucleus bootloader is great
    - it's on the FOSDEM-85 by default
    - we can program our application simply by `avrdude -c micronucleus -U flash:w:peewee.hex:i --noverify-memory` (yes, due to size constraints it can't read back the flash)
    - the default micronucleus the FOSDEM-85 comes with stays ~6 sec in the BL then jumps
    - so the peewee can't just use any short blink of sunlight to emit a short tweet
    - this is a huge disadvantage for peewee

- brownout detection
    - is enabled by default on the FOSDEM-85
    - set to 2.7V which is a sane value for a 5V application
    - disabling brownout detection is great for peewee as it leads to clock collapse and great audio when the solar cell is shaded
    - however the tiny85 tends to hang without any brownout detection after a few sunlight-darkness cycles (this is not fully consistent, but we'd need more funding for more research here!)
    - this is a huge disadvantage for peewee

- running solar applications
    - works great
    - place the solar cell instead of the BAT connector
    - has the bonus that the FOSDEM-85 then has a blocking diode for the cell in darkness
    - however there's no overvoltage protection
    - the solar cell must under no circumstances deliver more than 5V (later research will say 5.6V here)
    - this is a huge disadvantage for peewee

## measures

- flash a modified bootloader that doesn't run through a timeout but gets triggered by a jumper
    - micronucleus is all prepared for this
    - `#define ENTRYMODE ENTRY_JUMPER` is almost all we need
    - get https://github.com/micronucleus/micronucleus/ e.g  4dd1b49f39cd9d6fc736a720777ed62f4564e0e2
    - copy the directory `fosdem85peewee/` from here to micronucleus/firmware/configuration`
    - `make CONFIG=fosdem85peewee flash`
    - downside: this needs a "real" programmer, obviously micronucleus can't flash itself
    - staying in bootloader then requires a jumper from `#1` to `GND`. iow: connect the two second outermost toes of peewee

- flash the fuses for a brown out detection level of 1.8V
    - `make CONFIG=fosdem85peewee flash`

- overvoltage protection
    - by extensive research funding we'd conclude that a zener diode around the solar cell can protect peewee the penguin from overvoltage strikes from solar bursts
    - the coconut solar observation services on cool feet may come up with a wiring scheme for said zener diode



    may your resistance be negative

