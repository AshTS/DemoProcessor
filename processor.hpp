#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "defines.hpp"

class Processor
{
    public:
        unsigned char RAM[RAM_SIZE];
        unsigned short registers[16];
        unsigned short program_counter = 0;

        Processor();

        void execute_instruction();
        void dump_ram_segment(unsigned short start, unsigned short length);
        void display_state();

        unsigned short read_register(int reg_num);
        void write_register(int reg_num, unsigned short value);

        void render_display();
};

#endif //PROCESSOR_H
