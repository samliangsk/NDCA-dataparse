#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <utility>

// A simple struct to hold the statistics for each service
struct FlowStats {
    long long packet_count = 0;
    long long byte_count = 0;
};

// Use a pair of <protocol, port> as the key to uniquely identify a service
using ServiceKey = std::pair<std::string, int>;


// Helper function to remove trailing whitespace (like '\r') from a string
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
                // *** ADD THIS LINE TO FIX THE ISSUE ***
                trim_right(service_name);

                // For extra safety, you can also trim the protocol string
                trim_right(protocol);
                
                if (!service_name.empty()) { // Avoid adding entries with empty names
                    services[{protocol, std::stoi(port_str)}] = service_name;
                }
            } catch (const std::exception& e) {
                std::cerr << "Warning: Invalid line in services file: " << line << std::endl;
            }
        }
    }
    return services;
}

int main(int argc, char* argv[]) {
    // We now expect two arguments: the services file and the "N" for Top N
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <services.csv> <top_n>" << std::endl;
        return 1;
    }

    // 1. Load service definitions and the top_n value
    std::map<ServiceKey, std::string> services = loadServices(argv[1]);
    int top_n = 0;
    try {
        top_n = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid number for <top_n>: " << argv[2] << std::endl;
        return 1;
    }
    
    // 2. Prepare to aggregate statistics
    std::map<std::string, FlowStats> stats;
    std::string line;

    // 3. Process flow data from standard input
    while (getline(std::cin, line)) {
        std::stringstream ss(line);
        std::string proto, src_port_str, dst_port_str, packets_str, bytes_str;

        if (getline(ss, proto, ',') && getline(ss, src_port_str, ',') && 
            getline(ss, dst_port_str, ',') && getline(ss, packets_str, ',') && 
            getline(ss, bytes_str)) {
            
            try {
                int src_port = std::stoi(src_port_str);
                int dst_port = std::stoi(dst_port_str);
                proto.erase(std::remove_if(proto.begin(), proto.end(), [](unsigned char c) { return std::isspace(c); }), proto.end());
                long long packets = std::stoll(packets_str);
                long long bytes = std::stoll(bytes_str);

                std::string service_name = "other";
                ServiceKey dst_key = {proto, dst_port};
                ServiceKey src_key = {proto, src_port};
                ServiceKey key;
                key = dst_port < src_port? dst_key: src_key;
                // std::cout<<proto << " "<< dst_port<< " "<< src_port<< std::endl;
                if (services.count(key)) {
                    service_name = services.at(key);
                }
                else{
                    service_name = "other!!";
                }
                // service_name = services[key];
                // std::cout<<service_name<<std::endl;
                stats[service_name].packet_count += packets;
                stats[service_name].byte_count += bytes;

            } catch (const std::exception& e) { /* Ignore parsing errors */ }
        }
    }

    // 4. Prepare, sort, and print the "Top N" report
    // Convert map to a vector for sorting
    std::vector<std::pair<std::string, FlowStats>> sorted_stats(stats.begin(), stats.end());

    // Sort the vector in descending order based on byte count
    std::sort(sorted_stats.begin(), sorted_stats.end(), [](const auto& a, const auto& b) {
        return a.second.byte_count > b.second.byte_count;
    });

    std::cout << "\n--- Top " << top_n << " Services Report (by Bytes) ---\n";
    std::cout << std::left << std::setw(15) << "Service"
              << std::right << std::setw(20) << "Total Packets"
              << std::right << std::setw(20) << "Total Bytes" << std::endl;
    std::cout << std::string(55, '-') << std::endl;
    
    FlowStats other_stats;
    for (size_t i = 0; i < sorted_stats.size(); ++i) {
        if (i < top_n) {
            const auto& pair = sorted_stats[i];
            // Output as: Service,PacketCount,ByteCount
            std::cout << pair.first << ","
                    << pair.second.packet_count << ","
                    << pair.second.byte_count << std::endl;
        } else {
            // Aggregate the rest into "other"
            other_stats.packet_count += sorted_stats[i].second.packet_count;
            other_stats.byte_count += sorted_stats[i].second.byte_count;
        }
    }
    // Also update the "other" print statement
    if (other_stats.packet_count > 0) {
        std::cout << "other" << ","
                << other_stats.packet_count << ","
                << other_stats.byte_count << std::endl;
    }

    return 0;
}