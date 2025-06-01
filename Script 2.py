import re

def remove_xml_tags(input_file, output_file):
    tag_pattern = re.compile(r'<[^>]+>')  # Regex to match XML tags
    
    with open(input_file, 'r', encoding='utf-8') as infile, open(output_file, 'w', encoding='utf-8') as outfile:
        for line in infile:
            clean_line = re.sub(tag_pattern, '', line).strip()  # Remove tags and trim spaces
            if clean_line:  # Skip empty lines
                outfile.write(clean_line + '\n')

if __name__ == "__main__":
    input_file = "text.txt"  # Your large file with XML
    output_file = "clean_text.txt"  # New cleaned file without XML tags
    remove_xml_tags(input_file, output_file)
    print("Cleaning complete! Check clean_text.txt")

