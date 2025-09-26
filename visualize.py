import sys
import pandas as pd
import matplotlib.pyplot as plt
from io import StringIO

def create_pie_chart(data, value_column, color_map, title, filename):
    """Creates and saves a pie chart with a legend and consistent colors."""
    # --- Data Preparation ---
    data[value_column] = pd.to_numeric(data[value_column], errors='coerce')
    data.dropna(subset=[value_column], inplace=True)
    data = data.set_index('Service')
    data = data.sort_values(by=value_column, ascending=False)
    
    # If there are more than 11 services, group the smallest into an 'Other' slice
    if len(data) > 11:
        top_data = data.head(10)
        other_sum = data.iloc[10:][value_column].sum()
        top_data.loc['Other'] = other_sum
        data = top_data

    # Create an ordered list of colors for the pie chart
    plot_colors = [color_map.get(service, '#808080') for service in data.index] # Default to gray

    # --- Plotting ---
    plt.figure(figsize=(12, 8))
    # Note: We remove 'labels' and 'autopct' from the pie() call
    wedges, texts = plt.pie(
        data[value_column], 
        startangle=90, 
        colors=plot_colors,
        wedgeprops=dict(width=0.4) # This creates the donut hole
    )

    # --- Create Legend ---
    total = data[value_column].sum()
    legend_labels = [
        '{: <15} ({:.1f}%)'.format(name, (value / total) * 100)
        for name, value in data[value_column].items()
    ]

    plt.legend(
        wedges,
        legend_labels,
        title="Services",
        loc="center left",
        bbox_to_anchor=(1, 0, 0.5, 1) # Position legend outside the chart
    )

    plt.title(title, fontsize=16, pad=20)
    plt.tight_layout(pad=1.5)
    
    # --- Saving Figure ---
    plt.savefig(filename, bbox_inches='tight')
    # Use print for Python 3
    try:
        print("Chart saved to {}".format(filename))
    except TypeError:
        # Fallback for older Python versions
        sys.stdout.write("Chart saved to {}\n".format(filename))
    plt.close()

def main():
    """Main function to read data and generate charts."""
    df = None
    
    # Check if the script is being run interactively or without input
    if sys.stdin is None or sys.stdin.isatty():
        sys.stderr.write("This script is intended for pipeline use.\n")
        sys.stderr.write("Generating sample charts for demonstration.\n")
        
        # CORRECTED: Sample data is now comma-separated
        sample_data = """https://,10532050,8905120300
dns,830100,81203040
ssh,52300,5120400
other,210500,22304050
"""
        data_io = StringIO(sample_data)
        df = pd.read_csv(
            data_io,
            sep=',', # CORRECTED: Separator is now a comma
            names=["Service", "Total_Packets", "Total_Bytes"]
        )
    else:
        # This part for the pipeline was already correct
        all_input_lines = sys.stdin.readlines()
        if len(all_input_lines) < 3:
            sys.stderr.write("Error: Not enough data piped to the script.\n")
            return
            
        data_as_string = "".join(all_input_lines[2:])
        data_io = StringIO(data_as_string)
        
        try:
            df = pd.read_csv(
                data_io,
                sep=',', # Expecting comma-separated values
                names=["Service", "Total_Packets", "Total_Bytes"]
            )
        except Exception as e:
            sys.stderr.write("Error reading data with pandas: {}\n".format(e))
            return

    if df is None or df.empty:
        sys.stderr.write("No data was parsed to plot.\n")
        return

    # --- Create Unified Color Map ---
    unique_services = df['Service'].unique()
    colors = plt.get_cmap('tab20')(range(len(unique_services)))
    color_map = {service: color for service, color in zip(unique_services, colors)}

    # --- Generate Charts ---
    create_pie_chart(
        df.copy(),
        'Total_Packets', 
        color_map,
        'NetFlow Composition by Packet Count', 
        'packet_composition.png'
    )
    
    create_pie_chart(
        df.copy(),
        'Total_Bytes', 
        color_map,
        'NetFlow Composition by Data Volume (Bytes)', 
        'byte_composition.png'
    )
    
if __name__ == "__main__":
    main()