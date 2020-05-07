# get_bin_value.py
'''
This standalone program converts the letters A through G (and P for decimal point)
to their respective binary value for a common-anode seven segment display.
'''

output = ''
flags = {
    "a": 0,
    "b": 1,
    "c": 2,
    "d": 3,
    "e": 4,
    "f": 5,
    "g": 6,
    "p": 7
}
def main():    
    value = input("Please enter flags to build your binary value\n")
    global output
    binary_val = 0x00
    for c in value:
        binary_val |= 1<<flags[c]
    binary_val = 0x00FF & ~binary_val
    output += ' 0b'+format(binary_val, '08b') # Prints 8-bit numbers, separated by a space.
    
if __name__ == "__main__":
    try:
        while True:
            main()
    except KeyboardInterrupt: # This program only quits when the user inputs "CTRL+C"
        print ("Output:")
        print(output)
    
