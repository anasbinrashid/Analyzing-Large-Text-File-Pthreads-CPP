import os
import glob

# Define input and output directories
input_directory = r"D:\ANAS\WORK\SEMESTER 6\Parallel & Distributed Computing\Assignment 1\un\xml\en"
output_file = r"D:\ANAS\WORK\SEMESTER 6\Parallel & Distributed Computing\Assignment 1\text.txt"

def extract_text_from_xml(directory):
    """Extracts text from all XML files in the given directory and its subdirectories."""
    all_text = []
    
    # Iterate over year folders (y2000, y2001, ..., y2009)
    for year_folder in sorted(os.listdir(directory)):
        year_path = os.path.join(directory, year_folder)
        if os.path.isdir(year_path):  # Ensure it's a directory
            # Find all XML files inside the year folder
            xml_files = glob.glob(os.path.join(year_path, "*.xml"))
            
            for xml_file in xml_files:
                try:
                    with open(xml_file, "r", encoding="utf-8") as file:
                        all_text.append(file.read())
                except Exception as e:
                    print(f"Error reading {xml_file}: {e}")
    
    return "\n".join(all_text)

# Extract text from all XML files
combined_text = extract_text_from_xml(input_directory)

# Write the extracted text to a single file
try:
    with open(output_file, "w", encoding="utf-8") as output:
        output.write(combined_text)
    print(f"Extraction complete. Data saved to {output_file}")
except Exception as e:
    print(f"Error writing to output file: {e}")

