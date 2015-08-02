# dvapbridge
dvapbridge allows you to bridge D-STAR amateur radio traffic over the Internet.
It has the following requirements:

 * The server must be accessible to all clients and runs on port 8191.
 * Each client expects the presence of an Internet Labs, Inc DV Access Point
   Dongle (www.dvapdongle.com) connected via USB.

The radio data received from the DV Access Point Dongle connected to a client
will be sent to the server, which will in turn re-broadcast that data to all
other clients connected to the server.

The code has been tested to run on Linux and Mac OS X.
