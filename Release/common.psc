##clear receive, send windows, clear count
C:recv
C:send
C:count
C:recv count
C:recv count send

##edit mode, edit this document, after save, back to command mode.
E:edit

##display mode
D:hex
D:char
D:hex char

##open uart or tcp client
T:com3
T:com4
T:CNCA0
T:192.168.0.251:7788

##open uart or tcp client and log received data
T:com3:115200:810:L
T:com5:::L
T:192.168.0.251:7788:L

##open another command document.
F:at.txt

AT+CSQ
M:AT+CSQ[0D 0A]
