`define npc_add4 0
`define npc_j 1
`define npc_jr 2
`define npc_branch 3
`define npc_bgezalr 4
`define npc_eret 5

`define handlerAddr 32'h0000_4180
`define OP 		instr[31:26]
`define Func	instr[5:0]

`define nop    (`R & (`Func == 6'b000000) &(instr==32'h00000000))
`define R 		(`OP == 6'b000000)
`define addu	(`R & (`Func == 6'b100001))
`define subu	(`R & (`Func == 6'b100011))
`define ori     (`OP == 6'b001101)
`define lw      (`OP == 6'b100011)
`define sw      (`OP == 6'b101011)
`define beq     (`OP == 6'b000100)
`define lui     (`OP == 6'b001111)
`define j	    (`OP == 6'b000010)
`define jal     (`OP == 6'b000011)
`define jr      (`R & (`Func == 6'b001000))
`define add		(`R & (`Func == 6'b100000))
`define addi	(`OP == 6'b001000)
`define addiu	(`OP == 6'b001001)
`define sub		(`R & (`Func == 6'b100010))
`define sll		(`R & (`Func == 6'b000000) &(instr!=32'h00000000))
`define srl		(`R & (`Func == 6'b000010))
`define sra		(`R & (`Func == 6'b000011))

`define _and	(`R & (`Func == 6'b100100))
`define _or		(`R & (`Func == 6'b100101))
`define _nor	(`R & (`Func == 6'b100111))
`define _xor	(`R & (`Func == 6'b100110))
`define andi	(`OP == 6'b001100)
`define xori	(`OP == 6'b001110)
`define slt		(`R & (`Func == 6'b101010))
`define slti	(`OP == 6'b001010)
`define sltu	(`R & (`Func == 6'b101011))
`define sltiu	(`OP == 6'b001011)
`define bne		(`OP == 6'b000101)
`define blez	(`OP == 6'b000110)
`define bgez	((`OP == 6'b000001) & (instr[20:16] == 5'b00001))
`define bgtz	(`OP == 6'b000111)
`define bltz	((`OP == 6'b000001) & (instr[20:16] == 5'b00000))
`define jalr	(`R & (`Func == 6'b001001))
`define sh		(`OP == 6'b101001)
`define sb		(`OP == 6'b101000)
`define lh		(`OP == 6'b100001)
`define lhu		(`OP == 6'b100101)
`define lb		(`OP == 6'b100000)
`define lbu		(`OP == 6'b100100)

`define eret    (`OP == 6'b010000 &(instr[5:0]  == 5'b011000))


`define RegDst_A3 2'b00
`define RegDst_A2 2'b01
`define RegDst_ra 2'b10

`define alu_addu 5'b00000
`define alu_subu 5'b00001
`define alu_or 5'b00010
`define alu_and 5'b00011
`define alu_nor 5'b00100
`define alu_xor 5'b00101
`define alu_sllv 5'b00110
`define alu_sll 5'b00111
`define alu_srlv 5'b01000
`define alu_srl 5'b01001
`define alu_srav 5'b01010
`define alu_sra 5'b01011
`define alu_slt 5'b01100
`define alu_add 5'b01101
`define alu_sub 5'b01110
`define alu_sltu 5'b01111
`define alu_clz  5'b10000

`define R_CAL (`add|`addu|`sub|`subu|`_and|`_xor|`_nor|`_or|`slt|`sltu|`srl||`sll||`sra)
`define I_CAL (`addiu|`addi|`andi|`ori|`slti|`sltiu|`xori|`lui)
`define LD (`lw|`lb|`lbu|`lh|`lhu)
`define SW (`sw|`sh|`sb)
`define LS (`LD | `SW)
`define B (`beq|`bgez|`bgtz|`blez|`bltz|`bne)
