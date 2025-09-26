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

// Holds the details of a single unrecognized flow
// struct UnrecognizedFlow {
//     std::string protocol;
//     int src_port;
//     int dst_port;
//     long long packets;
//     long long bytes;
// };

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
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <services.csv> <top_n>" << std::endl;
        return 1;
    }

    std::map<ServiceKey, std::string> services = loadServices(argv[1]);
    int top_n = 0;
    try {
        top_n = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid number for <top_n>: " << argv[2] << std::endl;
        return 1;
    }
    
    std::map<std::string, FlowStats> stats;
    std::string line;

    // --- MODIFIED CODE: Use a map to aggregate unrecognized flows ---
    std::map<ServiceKey, FlowStats> unrecognized_stats;

    // Process flow data from standard input
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
                ServiceKey key = (dst_port < src_port) ? 
                                 ServiceKey{proto, dst_port} : 
                                 ServiceKey{proto, src_port};

                if (services.count(key)) {
                    service_name = services.at(key);
                } else {
                    service_name = "Unrecognized";
                    // --- Aggregate stats for the unrecognized key ---
                    unrecognized_stats[key].packet_count += packets;
                    unrecognized_stats[key].byte_count += bytes;
                }
                
                stats[service_name].packet_count += packets;
                stats[service_name].byte_count += bytes;

            } catch (const std::exception& e) { /* Ignore parsing errors */ }
        }
    }

    // --- Sort and write the AGGREGATED unrecognized flows ---
    if (!unrecognized_stats.empty()) {
        // 1. Convert map to a vector so we can sort by value (FlowStats)
        std::vector<std::pair<ServiceKey, FlowStats>> sorted_unrecognized;
        for (const auto& pair : unrecognized_stats) {
            sorted_unrecognized.push_back(pair);
        }

        // 2. Sort the vector by byte count in descending order
        std::sort(sorted_unrecognized.begin(), sorted_unrecognized.end(), 
            [](const auto& a, const auto& b) {
            return a.second.byte_count > b.second.byte_count;
        });

        // 3. Write the sorted data to the output file
        std::ofstream outfile("unrecognized.txt");
        if (outfile.is_open()) {
            outfile << "Protocol,Port,Packets,Bytes\n";
            for (const auto& pair : sorted_unrecognized) {
                outfile << pair.first.first << ","      // Protocol
                        << pair.first.second << ","     // Lower Port
                        << pair.second.packet_count << ","
                        << pair.second.byte_count << "\n";
            }
        }
    }

    // Prepare, sort, and print the "Top N" report (this part is unchanged)
    std::vector<std::pair<std::string, FlowStats>> sorted_stats(stats.begin(), stats.end());
    std::sort(sorted_stats.begin(), sorted_stats.end(), [](const auto& a, const auto& b) {
        return a.second.byte_count > b.second.byte_count;
    });
    std::ofstream outfile1("Top-Services.txt");
        if (!outfile1.is_open()) {
            std::cerr << "Error: Could not open services file: Top-Services.txt"  << std::endl;
        }

    std::cout << "\n--- Top " << top_n << " Services Report (by Bytes) ---\n";
    outfile1 << "--- Top " << top_n << " Services Report (by Bytes) ---\n";
    std::cout << "Service,Total Packets,Total Bytes\n";
    outfile1 << "Service,Total Packets,Total Bytes\n";
    
    FlowStats other_stats;
    for (size_t i = 0; i < sorted_stats.size(); ++i) {
        if (i < static_cast<size_t>(top_n)) {
            const auto& pair = sorted_stats[i];
            std::cout << pair.first << ","
                      << pair.second.packet_count << ","
                      << pair.second.byte_count << std::endl;

            outfile1  << pair.first << ","
                      << pair.second.packet_count << ","
                      << pair.second.byte_count << std::endl;
        } else {
            other_stats.packet_count += sorted_stats[i].second.packet_count;
            other_stats.byte_count += sorted_stats[i].second.byte_count;
        }
    }
    
    if (other_stats.packet_count > 0) {
        std::cout << "other" << ","
                  << other_stats.packet_count << ","
                  << other_stats.byte_count << std::endl;
        
        outfile1  << "other" << ","
                  << other_stats.packet_count << ","
                  << other_stats.byte_count << std::endl;
    }

    return 0;
}