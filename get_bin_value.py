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
    output += ' '+format(binary_val, '08b')
    
if __name__ == "__main__":
    try:
        while True:
            main()
    except KeyboardInterrupt:
        print ("Output:")
        print(output)
    
