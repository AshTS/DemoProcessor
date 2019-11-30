import sys
import assemble

IN_FILE = "test.asm"
OUT_FILE = "out.bin"

if len(sys.argv) > 1:
    IN_FILE = sys.argv[1]

if len(sys.argv) > 2:
    OUT_FILE = sys.argv[2]

f = open(IN_FILE, 'r')
input_data = f.read()
f.close()

result = bytearray(assemble.assemble(input_data))

f = open(OUT_FILE, 'wb')
f.write(result)
f.close()
