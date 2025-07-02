# Embedded Memory Game
> Authors: [Emily Hix](https://github.com/emilyhix), [Sabaipon Phimmala](https://github.com/bphimmala)

A fast-paced memory game using Arduino Uno and FRDM-K64F development board. The game begins by lighting up one LED for a short amount of time. The player must then press the button corresponding to this LED. With each following round, another light is added to the sequence. The player must watch and then press the buttons in the correct sequence. The game ends when the player fails to copy the pattern and an incorrect button is pressed.

We utilized GPIO, SPI, and FTM. GPIO was used for the LEDs (output) and buttons (input). SPI was used to send strings to a 16x2 LCD screen, which displayed game messages and the player’s score. FTM was utilized to seed our program so that the light sequences were pseudorandom.

## Demo
https://github.com/user-attachments/assets/f3f4fdc0-ff63-425d-9d65-6462b1c8951f
