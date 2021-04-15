#!/usr/bin/expect -f 

spawn west flash
expect "Please select one with desired serial number (1-2):"
send "1\n";
interact
