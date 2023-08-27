from intelhex import IntelHex
ih = IntelHex('test_.hex')
# ih.tofile("hex256.bin", format="bin")
path_now = "hex2data_" + ".txt"
file = open (path_now, "a")

data_prefix = "static const unsigend char hexdata[] PROGMEM = \n{"
data_subfix = "\n};\n"

print(data_prefix, end='')
file.write(data_prefix)

for i, data in enumerate(ih.tobinarray()):
    if(i%16 == 0): 
        print("\n  ", end ='')
        file.write("\n  ")
    print("0x%02X, "%data, end='')
    file.write("0x%02X, "%data)
    # if(i >= 4096 ):
    #     break

print(data_subfix)
file.write(data_subfix)