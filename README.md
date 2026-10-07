CLIENT
Connecting to server 127.0.0.1:5010 ...
Username: admin
Password: ********

[LOGIN SUCCESS] Welcome,
================================
           MAIN MENU
================================
  1) Arithmetic
  2) Chat
  3) Logout
Select an option: 1

================================
         ARITHMETIC MODE
================================
Enter one value at a time. Type 'back' to return to the menu.
First number: 12
Operator (+, -, *, /): -
Second number: 1

[RESULT] 12 - 1 = 11
First number: back

[SESSION] Returning to the main menu.

================================
           MAIN MENU
================================
  1) Arithmetic
  2) Chat
  3) Logout
Select an option: 2

================================
             CHAT
================================
You speak first. Type 'over' to hand over your turn or 'bye' to end chat.

say 'over' to toggle the turn . 'bye' to end chat.
Client: Hi cpm
Client: over

LISTENING to server...
Server: dont talk to me
Server: over

say 'over' to toggle the turn . 'bye' to end chat.
Client: ok
Client: bye

[CHAT] Chat ended.

================================
           MAIN MENU
================================
  1) Arithmetic
  2) Chat
  3) Logout
Select an option: 3

[SESSION] You have been logged out. Goodbye!
Connection closed by server.
[24bcs006@mepcolinux ex7]$./a.out
Connecting to server 127.0.0.1:5010 ...
Username: admin
Password: ******

[LOGIN FAILED] Incorrect password. Please try again.
Username: admi

[LOGIN FAILED] Username not found. Please try again.
Username: admin
Password: ********

[LOGIN SUCCESS] Welcome,
================================
           MAIN MENU
================================
  1) Arithmetic
  2) Chat
  3) Logout
Select an option: 2

================================
             CHAT
================================
You speak first. Type 'over' to hand over your turn or 'bye' to end chat.

say 'over' to toggle the turn . 'bye' to end chat.
Client: hey
Client: over

LISTENING to server...
Server: helo
what's up
Server: over

say 'over' to toggle the turn . 'bye' to end chat.
Client: Client: bye

[CHAT] Chat ended.

================================
           MAIN MENU
================================
  1) Arithmetic
  2) Chat
  3) Logout
Select an option: 3

[SESSION] You have been logged out. Goodbye!
Connection closed by server.

SERVER
[SERVER] UDP server is running on port 5010.

[SERVER] Waiting for a client...
[SERVER] Client connected from 127.0.0.1:38002.
[AUTH] Login attempt for username 'admin'
[AUTH] Password received for 'admin123' (hidden).
[AUTH] User 'admin123' authenticated successfully.
[MENU] admin123 selected option 1.
[SESSION] admin123 entered arithmetic mode.
[CALC] admin123: 12 - 1 = 11
[SESSION] admin123 left arithmetic mode.
[MENU] admin123 selected option 2.
[SESSION] admin123 entered chat mode.
[CHAT] admin123: Hi cpm
[CHAT] admin123: over

[CHAT] Your turn. Type 'over' to hand over or 'bye' to end chat.
[YOU] dont talk to me
[CHAT] Server: dont talk to me
[YOU] over
[CHAT] Server: over
[CHAT] admin123: ok
[CHAT] admin123: bye
[SESSION] admin123 left chat mode.
[MENU] admin123 selected option 3.
[SESSION] admin123 logged out.

[SERVER] Waiting for a client...
[SERVER] Client connected from 127.0.0.1:50685.
[AUTH] Login attempt for username 'admin'
[AUTH] Password received for 'admin1' (hidden).
[AUTH] Incorrect password for 'admin1' (attempt 1 of 3).
[AUTH] Login attempt for username 'admi'
[AUTH] Unknown username 'admi'.
[AUTH] Login attempt for username 'admin'
[AUTH] Password received for 'admin123' (hidden).
[AUTH] User 'admin123' authenticated successfully.
[MENU] admin123 selected option 2.
[SESSION] admin123 entered chat mode.
[CHAT] admin123: hey
[CHAT] admin123: over

[CHAT] Your turn. Type 'over' to hand over or 'bye' to end chat.
[YOU] helo
[CHAT] Server: helo
[YOU] over
[CHAT] Server: over
[CHAT] admin123: what's up
[CHAT] admin123: bye
[SESSION] admin123 left chat mode.
[MENU] admin123 selected option 3.
[SESSION] admin123 logged out.

[SERVER] Waiting for a client...
