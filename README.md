# NDCA-dataparse
Network Data Composition Analysis, data parse and analysis repo

## source of data

* [service-names-port-numbers.csv](https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml)

* [\*.pcap](https://mawi.wide.ad.jp/mawi/)

## To run netflow analysis

nfdump -q -r ./../../ddos_hackathon-20200511/aug/17/nfcapd.202008170800 -o "fmt:%pr,%sp,%dp,%pkt,%byt" | ./netflow-comp services_for_cpp.csv 10 | python3 visualize.py

cmake -S . -B build -DPcapPlusPlus_ROOT=./

cmake --build build

## Pcap analysis

Python scapy: too slow

Now using: cpp and the pcapplusplus library

Consider using tshark convert to json then intake from python

## Unrecognize Ports close up

### Port 38880 based on `ddos_hackathon-20200511/aug/17/nfcapd.202008170800`

Port 38880 mainly contain TCP flows, talking to 58207- 38230, as well as 443, 23, and 388.

It also carries UDP data, which is on port 123, default for NTP (Network Time Protocol).

The pattern looks like `138.215.216.190:38880` is talking to multiple devices on a higher port.

Upon checking, port 38880 mainly points to ACC (Avigilon Control Center), which is a Motorola ecosystem of CCTV and cameras (may contain more devices). 

## TODO

Concatenation of multiple netflow file and multiple pcap file

See the differences -> something appears in one time/file and not other

Consider colab with Lightscope and Eric to construct the 3D graph of ports?

Consider 2D map of flows, ie: ports tuple, and thickness of line to represent size?