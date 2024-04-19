#include <bits/stdc++.h>
#define PC_START 0;

using namespace std;

enum instr {ADD, SUB, MUL, INC, AND, OR, XOR, NOT, SLLI, SRLI, LI, LD, ST, JMP, BEQZ, HLT};

int main()
{
    int I$[256];
    for(int i = 0; i < 256; i++)
    {
        cin >> hex >> I$[i];
    }

    int D$[256];
    for(int i = 0; i < 256; i++)
    {
        cin >> hex >> D$[i];
    }

    int RF[16];
    for(int i = 0; i < 256; i++)
    {
        cin >> hex >> RF[i];
    }

    int PC = PC_START;
    bool IF_control = 1, ID_control = 1, EX_control = 1, MEM_control = 1, WB_control = 1;
    vector<int> busy(16, 0);

    int IR, PC1;

    //ID uses IR

    int ID_opcode, rd, rs1, rs2; //temp registers
    int A, B, imm, EX_opcode, rd1, PC2, jump_label, to_store_temp; //EX buffer

    int ALUoutput, Cond, MEM_opcode, A1, B1, to_store, to_load, rd2; //MEM buffer


    int LMD, WB_opcode, to_write, rd3; //WB buffer
    

    while(1)
    {


/*************************Write back*************************/
        if(WB_control == 1)
        {
            if(WB_opcode == HLT)
            {
                break;
            }
            else if(WB_opcode < LI)
            {
                RF[rd3] = to_write;
            }
            else if(WB_opcode < ST)
            {
                RF[rd3] = LMD;
            }
        }

/*************************Memory access*************************/
        if(MEM_control == 1)
        {
            if(MEM_opcode == LD)
            {
                LMD = D$[ALUoutput];
            }
            else if(MEM_opcode == LI)
            {
                LMD = to_load;
            }
            else if(MEM_opcode == ST)
            {
                D$[ALUoutput] = to_store;
            }

            WB_opcode = MEM_opcode;
            to_write = ALUoutput;
            rd3 = rd2;
        }


/*************************Execute*************************/
        if(EX_control == 1)
        {
            switch(EX_opcode)
            {
                case ADD:
                    ALUoutput = A + B;
                    break;

                case SUB:
                    ALUoutput = A - B;
                    break;

                case MUL:
                    ALUoutput = A * B;
                    break;
                
                case INC:
                    ALUoutput = A + 1;
                    break;
                
                case AND:
                    ALUoutput = A & B;
                    break;

                case OR:
                    ALUoutput = A | B;
                    break;

                case XOR:
                    ALUoutput = A ^ B;
                    break;
                
                case NOT:
                    ALUoutput = ~A;
                    break;
                
                case SLLI:
                    ALUoutput = A << imm;
                    break;
                
                case SRLI:
                    ALUoutput = A >> imm;
                    break;

                case LI:
                    to_load = imm;
                    break;

                case LD:
                    ALUoutput = A + imm;
                    break;

                case ST:
                    ALUoutput = A + imm;
                    to_store = to_store_temp;
                    break;

                case JMP:
                    int dir = jump_label & 0x80;
                    if(dir == 1)
                    {
                        PC = PC2 - (jump_label<<1);
                    }
                    else
                    {
                        PC = PC2 + (jump_label<<1);
                    }
                    break;

                case BEQZ:
                    Cond = (A == 0);
                    if(Cond == 1)
                    {
                        int dir = jump_label & 0x80;
                        if(dir == 1)
                        {
                            PC = PC2 - (jump_label<<1);
                        }
                        else
                        {
                            PC = PC2 + (jump_label<<1);
                        }
                    }
                    break;

                case HLT:
                    break;
            }

            MEM_opcode = EX_opcode;
            rd2 = rd1;


        }


/*************************Instruction decode*************************/
        if(ID_control == 1)
        {
            //set ID_opcode, rd, rs1, rs2, imm
            ID_opcode = IR>>12;
            rd = IR>>8 - (ID_opcode<<4);
            rs1 = IR>>4 - ID_opcode<<8 - rd<<4;
            rs2 = IR - ID_opcode<<12 - rd<<8 - rs1<<4;

            //mark rd as busy if the instruction will write to it
            if(ID_opcode < JMP) busy[rd] = 1;

            //check if operands are available
            int stall_pipeline = 0;
            if(ID_opcode < SLLI && ID_opcode != NOT && ID_opcode != INC)
            {
                if(busy[rs1] || busy[rs2])
                {
                    stall_pipeline = 1;
                    //stall the pipeline for some number of stages
                }
                else
                {
                    A = RF[rs1];
                    B = RF[rs2];
                }
            }
            else if(ID_opcode == NOT)
            {
                if(busy[rs1])
                {
                    stall_pipeline = 1;
                    //stall the pipeline
                }
                else
                {
                    A = RF[rs1];
                }
            }
            else if(ID_opcode == INC)
            {
                if(busy[rd])
                {
                    stall_pipeline = 1;
                    //stall the pipeline
                }
                else
                {
                    A = RF[rd];
                }
            }
            else if(ID_opcode < JMP && ID_opcode != LI)
            {
                if(busy[rs1])
                {
                    stall_pipeline = 1;
                    //stall the pipeline
                }
                else
                {
                    A = RF[rs1];
                    imm = rs2;
                    if(ID_opcode == ST)
                    {
                        to_store_temp = RF[rd];
                    }
                }
            }
            else if(ID_opcode == LI)
            {
                imm = (rs1<<4) + rs2;
            }
            else if(ID_opcode == JMP)
            {
                jump_label = (rd<<4) + rs1;
            }
            else if(ID_opcode == BEQZ)
            {
                if(busy[rd])
                {
                    stall_pipeline = 1;
                    //stall the pipeline
                }
                else
                {
                    A = RF[rd];
                    jump_label = (rs1<<4) + rs2;
                }
            }

            EX_opcode = ID_opcode;
            rd1 = rd;
            PC2 = PC1;

        }


/*************************Instruction fetch*************************/
        if(IF_control == 1)
        {
            IR = ((I$[PC])<<8) + I$[PC+1];
            PC += 2;
        }
        PC1 = PC;

    }
}
