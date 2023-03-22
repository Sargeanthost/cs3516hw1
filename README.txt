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
These numbers relative to each other make sense to me, as google.com has a lot more text. Also, these numbers match my
terminal's `ping` command time.

Supports both IP and domain name lookup. Many sites have redirects and other weird CDN blocks if you try to access through
straight IP for some reason, I know my website does this too. But, 52.5.170.204 for komkon.org works well with my program
and can download through IP or domain.
