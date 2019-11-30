labels = {}
replace = {}

add_replace = ""

def build_command_byte(fmt, cmd_type, dest):
    return [fmt * 0x80 + cmd_type * 0x10 + dest]

def interpret_arg(arg):
    global add_replace
    global current

    if arg in labels:
        return labels[arg]

    if arg.startswith("R"):
        return int(arg[1:])
    try:
        return int(arg, 0)
    except ValueError:
        add_replace = arg
        return 0


def get_compiled(args):
    op = args[0].upper()
    args = args[1:]

    if op in ["RLL", "RLA", "RRL", "RRA"]:
        lr = 0 if op[1] == "L" else 1
        la = 0 if op[2] == "L" else 1
        i = 0 if args[2].startswith("R") else 1

        last_bytes = [lr * 0b10000000 + la * 0b01000000 + i * 0b00100000 + interpret_arg(args[1])]

        if i == 1:
            imm = interpret_arg(args[2])
            last_bytes += [(imm & 0xFF00) >> 8, imm&0xFF]
        else:
            last_bytes.append(interpret_arg(args[2]) * 16)
            last_bytes.append(0)


        return build_command_byte(1, 2, interpret_arg(args[0])) + last_bytes

    if op.startswith("ADD") or op.startswith("SUB") or op.startswith("MUL") or op.startswith("DIV"):
        i = 0 if args[2].startswith("R") else 1
        u = op.endswith("U")

        if op.startswith("ADD"):
            op_type = 0
        elif op.startswith("SUB"):
            op_type = 1
        elif op.startswith("MUL"):
            op_type = 2
        elif op.startswith("DIV"):
            op_type = 3

        last_bytes = [u * 0x80 + i * 0x40 + op_type * 0x10 + interpret_arg(args[1])]

        if i == 1:
            imm = interpret_arg(args[2])
            last_bytes += [(imm & 0xFF00) >> 8, imm&0xFF]
        else:
            last_bytes.append(interpret_arg(args[2]) * 16)
            last_bytes.append(0)

        return build_command_byte(1, 3, interpret_arg(args[0])) + last_bytes

    if op.startswith("OR") or op.startswith("AND") or op.startswith("XOR"):
        i = 0 if args[2].startswith("R") else 1

        if op.startswith("OR"):
            op_type = 1
        elif op.startswith("AND"):
            op_type = 0
        elif op.startswith("XOR"):
            op_type = 2

        last_bytes = [op_type * 0x40 + i * 0x20 + interpret_arg(args[1])]

        if i == 1:
            imm = interpret_arg(args[2])
            last_bytes += [(imm & 0xFF00) >> 8, imm&0xFF]
        else:
            last_bytes.append(interpret_arg(args[2]) * 16)
            last_bytes.append(0)

        return build_command_byte(1, 1, interpret_arg(args[0])) + last_bytes

    if op.startswith("C"):
        n = "N" in op
        l = "L" in op
        e = "E" in op
        i = 0 if args[2].startswith("R") else 1
        u = op.endswith("U")

        fmt = 4 if not u else 6

        last_bytes = [n * 0x80 + l * 0x40 + e * 0x20 + i * 0x10 + interpret_arg(args[1])]

        if i == 1:
            imm = interpret_arg(args[2])
            last_bytes += [(imm & 0xFF00) >> 8, imm&0xFF]
        else:
            last_bytes.append(interpret_arg(args[2]) * 16)
            last_bytes.append(0)

        return build_command_byte(1, fmt, interpret_arg(args[0])) + last_bytes

    if op.startswith("J"):
        f = "F" in op
        l = "L" in op
        i = 0 if args[-1].startswith("R") else 1

        r1 = interpret_arg(args[1]) if len(args) > 1 else 0
        r2 = interpret_arg(args[2]) if len(args) > 2 else 0

        last_bytes = [f * 0x80 + l * 0x40 + i * 0x20 + r1 % 256]

        if i == 1:
            imm = interpret_arg(args[-1])
            last_bytes += [(imm & 0xFF00) >> 8, imm&0xFF]
        else:
            last_bytes.append(r2 * 16)
            last_bytes.append(0)

        return build_command_byte(1, 5, interpret_arg(args[0])) + last_bytes

    if op.startswith("S") or op.startswith("R"):
        o = "O" in op
        u = "U" in op
        bw = "W" in op
        rs = op.startswith("S")
        i = 1 if len(args) > 2 else 0

        last_bytes = [rs * 0x80 + o * 0x40 + u * 0x20 + bw * 0x10  + interpret_arg(args[1])]

        if i == 1:
            imm = interpret_arg(args[2])
            last_bytes += [(imm & 0xFF00) >> 8, imm&0xFF]

        return build_command_byte(i, 0, interpret_arg(args[0])) + last_bytes

def assemble(data, start=0):
    global labels
    global add_replace
    global replace

    result = []

    data = ''.join(data.split(","))
    lines = [item.strip() for item in data.split("\n") if item.strip() != ""]

    for line in lines:
        if line.startswith("."):
            if line.split(" ")[0][1:] == "SEG":
                place = int(line.split(" ")[1], 0)

                while len(result) < place:
                    result.append(0)

        elif not line.startswith("#"):
            if line.endswith(":"):
                labels[line.split(":")[0]] = len(result)
            else:
                result += get_compiled(line.split(" "))

            if add_replace != "":
                replace[len(result) - 2] = add_replace
                add_replace = "" 

    for k in replace:
        result[k] = (labels[replace[k]] & 0xFF00) >> 8
        result[k + 1] = labels[replace[k]] & 0xFF
    
    return result