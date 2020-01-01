# Demo Processor
This is the demonstration processor for this [C Compiler](https://www.github.com/CarterTS/CCompiler). It is designed with RISC principles in mind, however was also made to still represent some problems in code generations for the compiler.

# Build Instructions
The Demo Processor project can be built without any needed dependencies using make,

```
make main
```

# Usage

The assembler packaged with the emulator can be run from the root directory of the repository with

```
python3 assembler/main.py in_file out_file
```

This command will assemble the in_file and output the bytecode to the out_file.

The emmulator can be run using

```
./main file [-D]
```

This command will load the binary data from the file into RAM and the -D command will make the emulator load in debug mode which will display the status of registers and the segment of RAM near the program counter.
