# Pocket Orchestra

## TODO

- [ ] MacOS drivers
- [ ] Windows drivers
- [ ] 2 more songs
- [ ] At least one more dancer. Best to make a unique dancer for each song.
- [x] Can we use the USB connection to report high scores to a server?
- - YES
- - stty -F /dev/ttyACM0 cs8 9600 ignbrk -brkint -icrnl -imaxbel -opost -onlcr 
- -   -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke noflsh -ixon -crtscts 
- - ^ that doesn't appear to be necessary
- - Then read /dev/ttyACM0 like any file on the server side.
- - Device side, usb_send(v,c);

## Waves

- 0: Basic quiet sine.
- 1: Sharp organ.
- 2: Square lead.
- 3: Distortion lead.
- 4: Wacky synth brass.
- 5: Church organ.
- 6: Heavy strings.
- 7: Bells.
