RTT on wifi:
    google.com:
        Trial 0: 11
        Trial 1: 13
        Trial 2: 11
        Trial 3: 11
        Trial 4: 11
        Trial 5: 11
        Trial 6: 12
        Trial 7: 13
        Trial 8: 10
        Trial 9: 11
        AVG RTT: 11.4 ms
    example.com:
        Trial 0: 7
        Trial 1: 7
        Trial 2: 9
        Trial 3: 7
        Trial 4: 8
        Trial 5: 8
        Trial 6: 7
        Trial 7: 8
        Trial 8: 8
        Trial 9: 9
        AVG RTT: 7.8 ms
    Linux server:
        Trial 0: 0
        Trial 1: 0
        Trial 2: 0
        Trial 3: 0
        Trial 4: 0
        Trial 5: 0
        Trial 6: 0
        Trial 7: 0
        Trial 8: 0
        Trial 9: 0
        AVG RTT: 0ms
        Note: I assume the OS is doing some optimization when doing this socket connection seeing that it comes from
        the same computer, which is why it would be 0ms.
These numbers relative to each other make sense to me, as google.com has a lot more text. Also, these numbers match my
terminal's `ping` command time.

Supports both IP and domain name lookup. Many sites have redirects and other weird CDN blocks if you try to access through
straight IP for some reason, I know my website does this too. But, 52.5.170.204 for komkon.org works well with my program
and can download through IP or domain.

Makefiles:
All that's required when building is a linux machine and the gcc compiler. If you dont have gcc switch out gcc in the
makefiles for your computer's compiler.

Server notes:
When connecting to the server, use the format <ip>/TMDG.html as the server gets the file by name. You should get
    "HTTP/1.1 200 OK
    Content-length: 58327"
as this is what I received when I tested on 130.215.36.85(linux.wpi.edu).
Note that any other headers in the request will not be processed as they are outside the scope of the server part. All connections
are treated as "Connection: close". Also, by default, C closes sockets. Since our server lives in a while loop, and nothing happens
after this while loop ends, I rely on this.