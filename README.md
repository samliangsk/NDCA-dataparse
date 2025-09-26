# NDCA-dataparse
Network Data Composition Analysis, data parse and analysis repo

## source of data

* [service-names-port-numbers.csv](https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.xhtml)

* [\*.pcap](https://mawi.wide.ad.jp/mawi/)

## To run netflow analysis

nfdump -q -r ./../../ddos_hackathon-20200511/aug/17/nfcapd.202008170800 -o "fmt:%pr,%sp,%dp,%pkt,%byt" | ./netflow-comp services_for_cpp.csv 10 | python3 visualize.py

