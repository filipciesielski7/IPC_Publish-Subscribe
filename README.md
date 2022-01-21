<h1 align="center">
    IPC Publish-Subscribe 💬
</h1>

_Also available in: [Polski](README.pl.md)_

## About

The "IPC Publish-Subscribe" project allows logged-in users of the system to communicate with each other by transmitting their broadcast messages based on types to which they previously subscribed. A server is responsible for proper operation of the system and all activities pass through it.

## Client functionality

1. Login to the system
2. Display the menu
3. Display available messages types to subscribe
4. Register to the broadcast system
5. Register a message type
6. Broadcast new message
7. Receive a message synchronously
8. Receive a message asynchronously
9. Block a user
10. Logout from the system

## The server

Additionally created server lists all the infomation about the actions it takes and their effects, which was helpful and convenient while writing and testing the program. A time limitation of its operation has been also implemented, which means that the server will operate for a maximum number of seconds (after the last client logs out of the system) as the value of SERVER_STANDBY_TIME, which can be freely configured and is set to 10 seconds by default.

## Compilation

Unix:

<code>gcc -Wall server.c -o server</code>\
<code>gcc -Wall client.c -o client</code>

## Running

To run the whole program, we should start with the server:

<code>./server</code>

Then, in each new console window, we have the option to run the client program:

<code>./client</code>
