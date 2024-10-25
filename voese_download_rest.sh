#!/usr/bin/expect -f
set timeout -1
spawn sftp vo@192.168.88.230
expect "password:"
send -- "voese\n"
expect "sftp>"
send -- "get -r REST_server/\n"
expect "sftp>"
send -- "bye\n"

# It has to be made an executable with: chmod +x voese_download_rest.sh
# Warning: you need the expect command: sudo apt  install expect
