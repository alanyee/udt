udt (reliable datagram protocol)
=========================

Using udt for iDPL.

## Technical Details of Original Version

### Some Notes

* Sometimes `appclient` and/or `appserver` will not work, so try using 
`./appclient` instead of `appclient`.
* Sometimes, appclient and/or appserver still will not listen: 
`error while loading shared libraries: libudt.so: cannot open shared object file: No such file or directory`.
Try `export LD_LIBRARY_PATH=../src` in app directory.
* Sometimes, I refer to `appserver` as receiver and `appclient` as sender.

### Original appserver and appclient

1. Google udt and goto http://udt.sourceforge.net/ (original)
2. `curl`, `scp`, and/or `tar -xvcf` [source] to get file
3. Run `make` in the home directory, as it will also run `make` in the app directory
4. Usage (run the server first before the client or else connect  error)
```
[alanyee@point-a app]$ curl http://ipecho.net/plain
99.99.99.99
```
So,
```
[alanyee@point-a app]$ appserver 8888
server is ready at port: 8888
```
(Or simply `appserver` which defaults to port 9000.) Then,
```
[alanyee@point-b app]$ appclient 99.99.99.99 8888
SendRate(Mb/s)	RTT(ms)	CWnd	PktSndPeriod(us)	RecvACK	RecvNAK
754.663			1.029	3965	4.95023				940		2
950.176			0.983	4007	4.7214				1183	0
945.76			0.974	4226	4.51442				1177	0
950.051			0.977	3964	6.79545				1182	3
949.803			0.985	3926	6.37086				1182	0
950.366			0.971	3758	7.59848				1181	1
949.895			0.993	3995	7.07193				1183	0
950.149			0.981	4021	6.61366				1182	0
949.973			0.968	4297	5.91715				1182	0
950.256			0.985	3953	3.86373				1182	0
949.906			0.997	3907	3.68589				1183	0
950.183			0.978	3928	3.55842				1182	0
949.955			0.972	3827	3.43976				1182	0
950.242			0.996	4089	3.32777				1182	0
949.953			0.969	3946	3.18665				1182	0
950.228			1.128	3853	3.11168				1183	0
```
Resulting in,
```
[alanyee@point-a app]$ appserver 8888
server is ready at port: 8888
new connection: 999.999.999.999:13587
```

Breaking the sender makes the receiver print: `recv:Connection was broken.` 
Yet, the receiver continues running. Breaking the receiver makes the sender 
print out zeros for RecvACK and Sendrate. The sender also continues running.

### Original sendfile and recvfile (similar to appclient and appserver respectively)

1. Have or create a target file i.e. 
vi t.txt

2.
``` 
[alanyee@point-a app]$ sendfile 8888
server is ready at port: 8888
```
So,

`[alanyee@point-b app]$ recvfile 99.99.99.99 8888 t.txt h.txt`

Resulting in,
```
[alanyee@point-a app]$ sendfile 8888
server is ready at port: 8888
new connection: 999.999.999.999:14319
speed = 298.667Mbits/sec

[alanyee@point-b app]$ cat h.txt
Here is a test.
```

Sendfile remains open while recvfile closes. 
Upon breaking sendfile, recvfile results in:
```
[alanyee@point-b app]$ recvfile 99.99.99.99 8888 t.txt h.txt
connect: Connection setup failure: connection time out.
```

## Added Features
### If receiver cannot open a port, print out error to stderr.
```
[alanyee@point-a app]$ appserver 80
bind: Connection setup failure: unable to create/configure UDP socket: .

[alanyee@point-a app]$ appserver 80-0
illegal port number or port is busy.
```

### Oneshot (-1) option for receiver
```
[alanyee@point-a app]$ appserver 8888 -1
server is ready at port: 8888

[alanyee@point-b app]$ appclient 999.999.999.999 8888
```

On one-shot, when the sender breaks the connection for the first time, the 
receiver waits for the pthread (of recvdata) to terminate, blocking all other 
future attempts from the sender to connect. Upon the pthread's termination, 
the receiver itself terminates rather than continue.

```
[alanyee@point-b app]$ appclient 99.99.99.99 8888
SendRate(Mb/s)	RTT(ms)	CWnd	PktSndPeriod(us)	RecvACK	RecvNAK
754.663			1.029	3965	4.95023				940		2
950.176			0.983	4007	4.7214				1183	0
945.76			0.974	4226	4.51442				1177	0
950.051			0.977	3964	6.79545				1182	3
949.803			0.985	3926	6.37086				1182	0
950.366			0.971	3758	7.59848				1181	1
949.895			0.993	3995	7.07193				1183	0
950.149			0.981	4021	6.61366				1182	0
949.973			0.968	4297	5.91715				1182	0
950.256			0.985	3953	3.86373				1182	0
949.906			0.997	3907	3.68589				1183	0
950.183			0.978	3928	3.55842				1182	0
949.955			0.972	3827	3.43976				1182	0
950.242			0.996	4089	3.32777				1182	0
949.953			0.969	3946	3.18665				1182	0
950.228			1.128	3853	3.11168				1183	0
^C

[alanyee@point-a app]$ appserver 8888 -1
server is ready at port: 8888
new connection: 999.999.999.999:14319
recv:Connection was broken.
```
### Sender (-t): total time of test
```
appclient 99.99.99.99 8888 -t 2 
SendRate(Mb/s)	RTT(ms)	CWnd	PktSndPeriod(us)	RecvACK	RecvNAK
258.51          0.469   9065    23.3852             308     172
587.991         0.521   9708    19.2079             732     0
Summary: 846501 bytes sent
send:Connection was broken.
```
The timer option stops the sender after a user-specified number of seconds. 
When stopped, the sender prints out a summary detailing how many bytes have 
been sent, and then the sender closes.

### Remove Windows support (Work in Progress)
I am only testing for Linux/Unix support. So, removing Windows support code 
such as macro guards improves the code's readability. Fat meat is greasy.

## TODOS
* ~~If it can't open a port, stderr~~
* ~~One-shot (-1) option for receiver~~
* ~~Sender (-t): total time of test~~
* sendfile (-1)
* sendfile to udtfileserver
* recvfile to udtrecvfile
* (WIP) Remove Windows support
* [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
* Add "auto". Right now, code works with gcc version: 4.4.7
* replace atoi with strtol
