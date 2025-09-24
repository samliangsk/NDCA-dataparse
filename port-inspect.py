import csv
# from collections import Counter

PORT_SERVICE_CSV = "./service-names-port-numbers.csv"

def load_port_map_from_csv(csv_file):
    """
    Loads a port-to-service mapping from a two-column CSV file (port,service).

    Args:
        csv_file (str): The path to the CSV file.

    Returns:
        dict: A dictionary mapping integer port numbers to service name strings.
              Returns an empty dictionary if the file cannot be read.
    """
    port_map = {}
    with open(csv_file, mode='r', newline='') as infile:
        reader = csv.reader(infile)
        port_map = {}
        next(reader)
        
        # skipping the first row
        try:
            next(reader)
            pass
        except StopIteration:
            # File is empty
            return {}

        # Process remaining rows
        for row in reader:
            if not row or len(row) < 2:
                continue # Skip empty or malformed rows

            service = row[0].strip()
            port_str = row[1].strip()
            proto = row[2].strip()
            if(port_str == ''):
                break
            if '-' in port_str:
                start_end = port_str.split('-', 1)
                for i in range(int(start_end[0]),int(start_end[1])):
                    port = i
                    port_map[(port,proto)] = service

            else: 
                port = int(port_str)
                port_map[(port,proto)] = service
            

    return port_map


if __name__ == "__main__":
    port_service_map = load_port_map_from_csv(PORT_SERVICE_CSV)
