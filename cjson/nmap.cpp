/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nmap.h"
#include "hthreadpool.h"

int host_discover_task(std::string segment, void *nmap) {
    Nmap *hosts = (Nmap *)nmap;
    return host_discover(segment.c_str(), hosts);
}

int test_nmap() {
    char *segment = strdup("192.168.1.x/24");
    char *split = strchr(segment, '/');
    int n = 24;
    if (split) {
        *split = '\0';
        n = atoi(split + 1);
        if (n != 24 && n != 16) {
            return -2;
        }
    }

    Nmap hosts;
    char ip[INET_ADDRSTRLEN];
    if (n == 24) {
        host_discover(segment, &hosts);
    }
    else if (n == 16) {
        Nmap segs;
        int up_nsegs = segment_discover(segment, &segs);
        if (up_nsegs == 0) return 0;
#if 1
        for (auto &pair : segs) {
            if (pair.second == 1) {
                uint32_t addr = pair.first;
                uint8_t *p = (uint8_t *)&addr;
                // 0,255 reserved
                for (int i = 1; i < 255; ++i) {
                    p[3] = i;
                    hosts[addr] = 0;
                }
            }
        }
        nmap_discover(&hosts);
#else
        if (up_nsegs == 1) {
            for (auto &pair : segs) {
                if (pair.second == 1) {
                    inet_ntop(AF_INET, (void *)&pair.first, ip, sizeof(ip));
                    host_discover(ip, &hosts);
                }
            }
        }
        else {
            // ThreadPool + host_discover
            Nmap *hosts = new Nmap[up_nsegs];
            // use ThreadPool
            HThreadPool tp(4);
            tp.start();
            std::vector<std::future<int>> futures;
            int i = 0;
            for (auto &pair : segs) {
                if (pair.second == 1) {
                    inet_ntop(AF_INET, (void *)&pair.first, ip, sizeof(ip));
                    auto future = tp.commit(host_discover_task, std::string(ip), &hosts[i++]);
                    futures.push_back(std::move(future));
                }
            }
            // wait all task done
            int nhosts = 0;
            for (auto &future : futures) {
                nhosts += future.get();
            }
            // filter up hosts
            std::vector<uint32_t> up_hosts;
            for (int i = 0; i < up_nsegs; ++i) {
                Nmap &nmap = hosts[i];
                for (auto &host : nmap) {
                    if (host.second == 1) {
                        up_hosts.push_back(host.first);
                    }
                }
            }
            delete[] hosts;
        }
#endif
    }

    // filter up hosts
    std::vector<uint32_t> up_hosts;
    for (auto &pair : hosts) {
        if (pair.second == 1) {
            up_hosts.push_back(pair.first);
        }
    }
    // print up hosts
    printf("Up hosts %lu:\n", up_hosts.size());
    for (auto &host : up_hosts) {
        inet_ntop(AF_INET, (void *)&host, ip, sizeof(ip));
        // printf("%s\n", ip);
    }
    return 0;
}
