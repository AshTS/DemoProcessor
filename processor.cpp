#include "processor.hpp"

#include <stdio.h>

#include "defines.hpp"

#define HIGHLIGHT "\u001b[32m"

Processor::Processor()
{
    for (int i = 0; i < RAM_SIZE; i++)
    {
        RAM[i] = 0;
    }

    for (int i = 0; i < 16; i++)
    {
        registers[i] = 0;
    }
}

void Processor::execute_instruction()
{
    unsigned char format = (RAM[program_counter] & 0b10000000) >> 7;
    unsigned char type = (RAM[program_counter] & 0b01110000) >> 4;
    unsigned char dest = (RAM[program_counter] & 0b00001111) >> 0;

    // printf("Fmt: %d Type: %d Dest: %d\n", format, type, dest);

    switch (type)
    {
        case 0: // Data Movement
        {
            unsigned char op_type = (RAM[program_counter + 1] & 0xF0) >> 4;
            unsigned char source = (RAM[program_counter + 1] & 0x0F);

            bool load_or_store = (op_type & 0b1000) > 3;
            bool offset = (op_type & 0b0100) > 2;
            bool upper = (op_type & 0b0010) > 1;
            unsigned char size = (op_type & 0b0001);

            unsigned short address = read_register(load_or_store ? dest : source);

            if (format == 1 && offset)
            {
                address = (unsigned int)address + RAM[program_counter + 2] * 256 + RAM[program_counter + 3];
            }

            if (size == 0)
            {
                if (load_or_store)
                {
                    unsigned char data = (read_register(source) & (0xFF << ((int)upper * 8)) >> ((int)upper * 8));
                    RAM[address] = data;
                }
                else
                {
                    unsigned short result = read_register(dest);

                    unsigned char data = RAM[address];

                    if (!upper)
                    {
                        result = (result&0xFF00) + (unsigned short)data;
                    }
                    else
                    {
                        result = (result&0x00FF) + data * 0xFF;
                    }

                    write_register(dest, result);
                }
                
            }
            else
            {
                if (load_or_store)
                {
                    unsigned char data0, data1;

                    data0 = (read_register(source) & 0xFF00) >> 8;
                    data1 = (read_register(source) & 0x00FF) >> 0;

                    RAM[address] = data0;
                    RAM[address + 1] = data1;
                }
                else
                {

                    write_register(dest, RAM[address] * 256 + RAM[address + 1]);
                }
                
            }
            
        }
        break;

        case 1: //Logic Ops
        {
            unsigned char op_type = (RAM[program_counter + 1] & 0b11000000) >> 6;
            unsigned char source = (RAM[program_counter + 1] & 0x0F);

            unsigned short immediate = RAM[program_counter + 2] * 256 + RAM[program_counter + 3];
            unsigned char source_2 = (RAM[program_counter + 2] & 0xF0) >> 4;

            bool use_immediate = (RAM[program_counter + 1] & 0b00100000) >> 5;

            unsigned short result = 0;

            unsigned short arg0 = read_register(source);
            unsigned short arg1 = 0;

            if (!use_immediate)
            {
                arg1 = read_register(source_2);
            }
            else
            {
                arg1 = immediate;
            }
            

            switch (op_type)
                {
                    case 0:
                    result = arg0 & arg1;
                    break;

                    case 1:
                    case 3:
                    result = arg0 | arg1;
                    break;

                    case 2:
                    result = arg0 ^ arg1;
                    break;

                    default:
                    break;
                }

            write_register(dest, result);
        }
        break;
        
        case 2: //Shift Operations
        {
            unsigned char op_type = (RAM[program_counter + 1] & 0b00110000) >> 4;
            unsigned char source = (RAM[program_counter + 1] & 0x0F);

            unsigned short immediate = RAM[program_counter + 2] * 256 + RAM[program_counter + 3];
            unsigned char source_2 = (RAM[program_counter + 2] & 0xF0) >> 4;

            bool left_right = (RAM[program_counter + 1] & 0b10000000) >> 7;
            bool logical_arithmatic = (RAM[program_counter + 1] & 0b01000000) >> 6;
            bool use_immediate = (RAM[program_counter + 1] & 0b00100000) >> 5;

            unsigned short result = 0;

            signed short arg0 = (signed short)read_register(source);
            signed short arg1 = 0;

            if (use_immediate)
            {
                arg1 = immediate;
            }
            else
            {
                arg1 = read_register(source_2);
            }
            

            if (!logical_arithmatic)
            {
                if (left_right)
                {
                    result = arg0 >> arg1;
                }
                else
                {
                    result = arg0 << arg1;
                }
            }
            else
            {
                if (left_right)
                {
                    result = result / (1 << arg1);
                }
                else
                {
                    result = result * (1 << arg1);
                }
                
            }

            write_register(dest, result);
        }
        break;

        case 3: //Arithmatic Ops
        {
            unsigned char op_type = (RAM[program_counter + 1] & 0b00110000) >> 4;
            unsigned char source = (RAM[program_counter + 1] & 0x0F);

            unsigned short immediate = RAM[program_counter + 2] * 256 + RAM[program_counter + 3];
            unsigned char source_2 = (RAM[program_counter + 2] & 0xF0) >> 4;

            bool use_unsigned = (RAM[program_counter + 1] & 0b10000000) >> 7;
            bool use_immediate = (RAM[program_counter + 1] & 0b01000000) >> 6;

            unsigned short result = 0;

            signed int arg0 = (signed short)read_register(source);
            signed int arg1 = 0;

            if (!use_immediate)
            {
                arg1 = read_register(source_2);
            }
            else
            {
                arg1 = immediate;
            }
            
            if (use_unsigned)
            {
                arg0 = (unsigned short)arg0;
                arg1 = (unsigned short)arg1;
            }

            switch (op_type)
            {
                case 0:
                result = arg0 + arg1;
                break;

                case 1:
                result = arg0 - arg1;
                break;

                case 2:
                result = arg0 * arg1;
                break;

                case 3:
                result = arg0 / arg1;
                break;

                default:
                break;
            }

            write_register(dest, result);
        }
        break;

        case 4: //Comparisons
        case 6:
        {
            bool invert = (RAM[program_counter + 1] & 0b10000000) >> 7;
            bool less = (RAM[program_counter + 1] & 0b01000000) >> 6;
            bool equal = (RAM[program_counter + 1] & 0b00100000) >> 5;
            bool use_immediate = (RAM[program_counter + 1] & 0b00010000) >> 4;
            bool use_unsigned = (type == 6);

            unsigned char source_0 = (RAM[program_counter + 1] & 0x0F) >> 0;
            unsigned char source_1 = (RAM[program_counter + 2] & 0xF0) >> 4;

            unsigned short immediate = RAM[program_counter + 2] * 256 + RAM[program_counter + 3];

            signed int val0 = read_register(source_0);
            signed int val1 = 0;

            if (use_immediate)
            {
                val1 = immediate;
            }
            else
            {
                val1 = read_register(source_1);
            }
            

            if (use_unsigned)
            {
                val0 = (unsigned short)val0;
                val0 = (unsigned short)val0;
            }
            
            unsigned short result = 0;

            if (less && val0 < val1)
            {
                result |= 0b001;
            }

            if (equal && val0 == val1)
            {
                result |= 0b001;
            }

            if (invert)
            {
                result ^= 0b001;
            }

            write_register(dest, result);
        }
        break;

        case 5: //Jumps
        {
            bool use_flags = (RAM[program_counter + 1] & 0b10000000) >> 7;
            bool use_link = (RAM[program_counter + 1] & 0b01000000) >> 6;
            bool use_immediate = (RAM[program_counter + 1] & 0b00100000) >> 5;

            unsigned char reg0 = dest;
            unsigned char reg1 = RAM[program_counter + 1] & 0x0F;
            unsigned char reg2 = (RAM[program_counter + 2] & 0xF0) >> 4;

            unsigned short immediate = RAM[program_counter + 2] * 256 + RAM[program_counter + 3];

            unsigned char link = 0;
            unsigned char flag = 0;
            unsigned short addr = 0;
            if (use_link)
            {
                link = reg0;
                reg0 = reg1;
                reg1 = reg2;
            }

            if (use_flags)
            {
                flag = reg0;
                reg0 = reg1;
                reg1 = reg2;
            }

            if (use_immediate)
            {
                addr = immediate;
            }
            else
            {
                addr = read_register(reg0);
            }

            
            if (use_flags && read_register(flag) == 0)
            {
                break;
            }

            if (use_link)
            {
                write_register(link, program_counter + format * 2 + 2);
            }

            program_counter = addr;
            return;
        }
        break;

        default:
        break;
    }

    program_counter += format * 2 + 2;
}

void Processor::dump_ram_segment(unsigned short start, unsigned short length)
{
    printf("      0  1  2  3  4  5  6  7    8  9  A  B  C  D  E  F\n");
    int start_address = start - start % 16;
    int end_address = start_address + length;

    for (int a = start_address; a < end_address; a+=16)
    {
        printf("%03X ", a/16);
        for (int i = 0; i < 8; i++)
        {
            if (a + i == program_counter)
            {
                printf(HIGHLIGHT);
            }

            printf(" %02X\u001b[0m", RAM[a + i]);
        }

        printf("  ");

        for (int i = 0; i < 8; i++)
        {
            if (a + 8 + i == program_counter)
            {
                printf(HIGHLIGHT);
            }

            printf(" %02X\u001b[0m", RAM[a + 8 + i]);
        }

        printf("    ");

        for (int i = 0; i < 16; i++)
        {
            char c = RAM[a + i];

            if (a + i == program_counter || a + i == registers[2])
            {
                printf(HIGHLIGHT);
            }

            if (c > 0x1F && c < 0x80)
            {
                printf("%c\u001b[0m", c);
            }
            else
            {
                printf(".\u001b[0m");
            }
            
        }

        printf("\n");
    }

    
}


void Processor::display_state()
{
    printf("Registers: \n   ");

    for (int i = 0; i < 8; i++)
    {
        printf("%02d: %04X  ", i, registers[i]);
    }

    printf("\n   ");

    for (int i = 8; i < 16; i++)
    {
        printf("%02d: %04X  ", i, registers[i]);
    }

    printf("\n\n");

    printf("Program Counter: 0x%04X\n\n", program_counter);
}

unsigned short Processor::read_register(int reg_num)
{
    if (reg_num == 0)
    {
        return 0;
    }

    return registers[reg_num];
}

void Processor::write_register(int reg_num, unsigned short value)
{
    if (reg_num == 0)
    {
        return;
    }

    registers[reg_num] = value;
}

void Processor::render_display()
{
    unsigned short addr = 0x8000;

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            char symbol = RAM[addr];

            if (symbol < ' ' || symbol >= 128)
            {
                symbol = 254;
            }

            printf("%c", symbol);

            addr+=2;
        }
        printf("\n");
    }
}