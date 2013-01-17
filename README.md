These two Arduino apps work together to send a single ASCII character from one RadioBlock/Arduino setup to another.

The mySender_v2 app goes on the Arduino with the RadioBlock with the address of 0 on it.
The myReceiver_v2 app goes on the ARduino with the RadioBlock with the addressed of 1 on it.

This is barely working but it's a proof that the Arduino library at least works. These apps will be fleshed out further in the very near future.

One important point is that these devices seem to be able to send and receive information pretty quickly but these demo's don't really try that hard to be quick. When I tried playing around with the timings, I hit an interesting issue where the sending Arduino's RadioBlock would seem to crash after a few minutes.

Other then that, have fun!
