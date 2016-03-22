udt (reliable datagram protocol)
=========================

Using udt for iDPL

## Current iteration

### For appserver and appclient

1. google udt
2. wget, scp, and/or tar -xvcf [source] to get file
3. run make, once in the home directory of udt and then in app
4. Sometimes, appclient and/or appserver will not listen (or error while loading shared libraries: libudt.so: cannot open shared object file: No such file or directory"
Solution:
export LD_LIBRARY_PATH=../src


5. Usage
(run the server first before the client or else connect  error)
				[alanyee@point-a app]$ curl http://ipecho.net/plain; echo
				99.99.99.99

So,

[alanyee@point-a app]$ appserver 8888
server is ready at port: 8888

Then,

[alanyee@point-b app]$ appclient 99.99.99.99 8888
SendRate(Mb/s)	RTT(ms)	CWnd	PktSndPeriod(us)	RecvACK	RecvNAK
754.663		1.029	3965	4.95023			940	2
950.176		0.983	4007	4.7214			1183	0
945.76		0.974	4226	4.51442			1177	0
950.051		0.977	3964	6.79545			1182	3
949.803		0.985	3926	6.37086			1182	0
950.366		0.971	3758	7.59848			1181	1
949.895		0.993	3995	7.07193			1183	0
950.149		0.981	4021	6.61366			1182	0
949.973		0.968	4297	5.91715			1182	0
950.256		0.985	3953	3.86373			1182	0
949.906		0.997	3907	3.68589			1183	0
950.183		0.978	3928	3.55842			1182	0
949.955		0.972	3827	3.43976			1182	0
950.242		0.996	4089	3.32777			1182	0
949.953		0.969	3946	3.18665			1182	0
950.228		1.128	3853	3.11168			1183	0

Resulting,

[alanyee@point-a app]$ appserver 8888
server is ready at port: 8888
new connection: 999.999.999.999:13587


Breaking the client stops the client while the server reads:
recv:Connection was broken.

But breaking the server stops the server while the client continues to print 
out zeros for RecvACK and Sendrate.

### For sendfile and recvfile (similar to app client and server)

0. have or create a target file i.e. 
vi t.txt

1. [alanyee@point-a app]$ sendfile 8888
server is ready at port: 8888

2. [alanyee@point-b app]$ recvfile 99.99.99.99 8888 t.txt h.txt

3. Resulting in:
[alanyee@point-a app]$ sendfile 8888
server is ready at port: 8888
new connection: 999.999.999.999:14319
speed = 298.667Mbits/sec

[alanyee@point-b app]$ cat h.txt
Here is a test.

Sendfile remains open while recvfile closes. Upon breaking sendfile, recvfile results in:

[alanyee@point-b app]$ recvfile 99.99.99.99 8888 t.txt h.txt
connect: Connection setup failure: connection time out.

## TODOS
* Oneshot (-1) for recvfile
* If it can't open a port, stderr
* Sender (-t): total time of test
* sendfile to udtfileserver
* recvfile to udtrecvfile
* Remove windows support. Only testing for Linux/Unix support
* [Google's C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
* Add "auto". Right now, code works with gcc version: 4.4.7

## Original

http://udt.sourceforge.net/