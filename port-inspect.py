import csv

# --- Configuration ---
# The source CSV from IANA (https://www.iana.org/assignments/service-names-port-numbers/service-names-port-numbers.csv)
SOURCE_CSV = "./service-names-port-numbers.csv"
# The clean, C++-compatible CSV that this script will create
OUTPUT_CSV = "./services_for_cpp.csv"

def generate_cpp_input_csv(source_file, output_file):
    """
    Reads the complex IANA services CSV, processes it, and writes a simple,
    three-column CSV file compatible with our C++ parser.

    Args:
        source_file (str): The path to the source IANA CSV file.
        output_file (str): The path for the generated output CSV file.
    """
    port_map = {}
    
    # --- Part 1: Read and parse the source CSV ---
    try:
        with open(source_file, mode='r', newline='', encoding='utf-8') as infile:
            reader = csv.reader(infile)
            
            # The IANA CSV has one header row, so we skip it.
            # The original script skipped two rows, which was likely a bug.
            next(reader, None)

            # Process all data rows
            for row in reader:
                # Ensure the row has the necessary columns (Service, Port, Protocol)
                if not row or len(row) < 3:
                    continue

                service = row[0].strip()
                port_str = row[1].strip()
                proto = row[2].strip().upper() # Standardize protocol to uppercase (TCP, UDP)
                if service == "":
                    service = "Unknown"
                # Skip rows that are missing essential information
                if not service or not port_str or not proto:
                    continue

                # Handle port ranges (e.g., "49152-65535")
                if '-' in port_str:
                    try:
                        start, end = map(int, port_str.split('-', 1))
                        # The range must include the 'end' port, so we add +1
                        for port in range(start, end + 1):
                            port_map[(port, proto)] = service
                    except ValueError:
                        continue # Skip malformed ranges
                # Handle single ports
                else:
                    try:
                        port = int(port_str)
                        port_map[(port, proto)] = service
                    except ValueError:
                        continue # Skip non-integer port numbers
                        
    except FileNotFoundError:
        print(f"Error: Source file not found at '{source_file}'")
        return

    # --- Part 2: Write the parsed data to the output CSV ---
    with open(output_file, mode='w', newline='', encoding='utf-8') as outfile:
        writer = csv.writer(outfile)
        
        # Write the data in the format: protocol,port,service
        for (port, protocol), service_name in port_map.items():
            writer.writerow([protocol, port, service_name])


if __name__ == "__main__":
    print(f"Reading services from: '{SOURCE_CSV}'")
    generate_cpp_input_csv(SOURCE_CSV, OUTPUT_CSV)
    print(f"Successfully created C++ compatible file: '{OUTPUT_CSV}'")