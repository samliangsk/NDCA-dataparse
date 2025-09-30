#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <utility>
#include <cctype>

// Your include style
#include "PcapFileDevice.h"
#include "Packet.h"
#include "EthLayer.h"
#include "IPv4Layer.h"
#include "TcpLayer.h"
#include "UdpLayer.h"

// Structs and type definitions
struct FlowStats {
    long long packet_count = 0;
    long long byte_count = 0;
};
using ServiceKey = std::pair<std::string, int>;

// Helper function to remove trailing whitespace
void trim_right(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// Function to load the protocol/port-to-service mapping from your CSV
std::map<ServiceKey, std::string> loadServices(const std::string& filename) {
    std::map<ServiceKey, std::string> services;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Could not open services file: " << filename << std::endl;
        exit(1);
    }

    while (getline(file, line)) {
        std::stringstream ss(line);
        std::string protocol, port_str, service_name;

        if (getline(ss, protocol, ',') && getline(ss, port_str, ',') && getline(ss, service_name)) {
            try {
                trim_right(service_name);
                trim_right(protocol);
                if (!service_name.empty()) {
                    services[{protocol, std::stoi(port_str)}] = service_name;
                }
            } catch (const std::exception& e) { /* Ignore invalid lines */ }
        }
    }
    return services;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <pcap_file> <services.csv> <top_n>" << std::endl;
        return 1;
    }

    std::string pcap_filename = argv[1];
    std::string services_filename = argv[2];
    int top_n = std::stoi(argv[3]);

    auto services = loadServices(services_filename);
    
    std::map<std::string, FlowStats> stats;
    std::map<ServiceKey, FlowStats> unrecognized_stats;

    pcpp::IFileReaderDevice* reader = pcpp::IFileReaderDevice::getReader(pcap_filename);
    if (!reader->open()) {
        std::cerr << "Error: Could not open pcap file: " << pcap_filename << std::endl;
        delete reader;
        return 1;
    }

    pcpp::RawPacket rawPacket;
    while (reader->getNextPacket(rawPacket)) {
        pcpp::Packet parsedPacket(&rawPacket);

        int src_port = 0;
        int dst_port = 0;
        std::string proto = "Unknown";
        
        pcpp::TcpLayer* tcpLayer = parsedPacket.getLayerOfType<pcpp::TcpLayer>();
        if (tcpLayer != nullptr) {
            proto = "TCP";
            src_port = tcpLayer->getSrcPort();
            dst_port = tcpLayer->getDstPort();
        } else {
            pcpp::UdpLayer* udpLayer = parsedPacket.getLayerOfType<pcpp::UdpLayer>();
            if (udpLayer != nullptr) {
                proto = "UDP";
                src_port = udpLayer->getSrcPort();
                dst_port = udpLayer->getDstPort();
            } else if (parsedPacket.isPacketOfType(pcpp::IPv4)) { // *** SYNTAX FIX IS HERE ***
                pcpp::IPv4Layer* ipLayer = parsedPacket.getLayerOfType<pcpp::IPv4Layer>();
                switch (ipLayer->getProtocol()) {
                    case pcpp::ICMP: proto = "ICMP"; break;
                    case pcpp::GRE: proto = "GRE"; break;
                    default: proto = "OtherIP"; break;
                }
            }
        }

        long long bytes = rawPacket.getRawDataLen();
        long long packets = 1;

        std::string service_name = "Unrecognized";
        ServiceKey key = (dst_port < src_port) ? 
                         ServiceKey{proto, dst_port} : 
                         ServiceKey{proto, src_port};

        if (services.count(key)) {
            service_name = services.at(key);
        } else {
            unrecognized_stats[key].packet_count += packets;
            unrecognized_stats[key].byte_count += bytes;
        }
        
        stats[service_name].packet_count += packets;
        stats[service_name].byte_count += bytes;
    }

    reader->close();
    delete reader;
    
    // --- REPORTING LOGIC (RESTORED) ---
    if (!unrecognized_stats.empty()) {
        std::vector<std::pair<ServiceKey, FlowStats>> sorted_unrecognized;
        for (const auto& pair : unrecognized_stats) { sorted_unrecognized.push_back(pair); }
        std::sort(sorted_unrecognized.begin(), sorted_unrecognized.end(), 
            [](const auto& a, const auto& b) { return a.second.byte_count > b.second.byte_count; });

        std::ofstream outfile("unrecognized_pcap.txt");
        if (outfile.is_open()) {
            outfile << "Protocol,Port,Packets,Bytes\n";
            for (const auto& pair : sorted_unrecognized) {
                outfile << pair.first.first << "," << pair.first.second << ","
                        << pair.second.packet_count << "," << pair.second.byte_count << "\n";
            }
        }
    }

    std::vector<std::pair<std::string, FlowStats>> sorted_stats(stats.begin(), stats.end());
    std::sort(sorted_stats.begin(), sorted_stats.end(), 
        [](const auto& a, const auto& b) { return a.second.byte_count > b.second.byte_count; });

    std::ofstream outfile1("Top-Services_pcap.txt");
    outfile1 << "--- Top " << top_n << " Services Report (by Bytes) ---\n";
    outfile1 << "Service,Total Packets,Total Bytes\n";
    
    FlowStats other_stats;
    for (size_t i = 0; i < sorted_stats.size(); ++i) {
        if (i < static_cast<size_t>(top_n)) {
            const auto& pair = sorted_stats[i];
            outfile1 << pair.first << "," << pair.second.packet_count << "," << pair.second.byte_count << std::endl;
        } else {
            other_stats.packet_count += sorted_stats[i].second.packet_count;
            other_stats.byte_count += sorted_stats[i].second.byte_count;
        }
    }
    
    if (other_stats.packet_count > 0) {
        outfile1 << "other" << "," << other_stats.packet_count << "," << other_stats.byte_count << std::endl;
    }

    std::cout << "Processing complete. Report saved to Top-Services_pcap.txt and unrecognized_pcap.txt" << std::endl;

    return 0;
}