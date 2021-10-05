`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/09/27 17:14:58
// Design Name: 
// Module Name: Ctrl
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////
 `include "INS.vh"


module Crtl(
    input [31:0] Instr,
    input [5:0] OPCode,
    input [5:0] FunctCode,
    output [1:0] MemtoReg,
    output MemWrite,
    output ALUsrc,
    output [4:0] ALUop,
    output [1:0] EXTop,
    output [1:0] RegDst,
    output RegWrite,
    output [2:0]PCsrc,
    output NPCsrc, 
    output [2:0]LStype
    );
  
     wire [31:0]instr;
	 assign instr = Instr;				
	 		
    assign RegWrite = `R_CAL | `I_CAL | `LD | `jal |`jalr ;
						
    assign RegDst = (`R_CAL | `jalr) ?  `RegDst_A3:
                    (`I_CAL | `LD) ?    `RegDst_A2:
                    (`jal) ?            `RegDst_ra:
                                        2'bxx;

    assign EXTop = (`ori|`andi|`xori) ?                         0 :  // 0扩展   
                   (`LS |(`addiu|`addi|`slti|`sltiu)) ?         1 : // 符号位扩展
                   (`lui) ?                                     2 : // 低位扩展
                                                                2'bxx;

    assign ALUsrc = `LS | `I_CAL;

    assign ALUop = (`subu|`beq) ?       `alu_subu :
                   (`sub) ?             `alu_sub :
                   (`add|`addi) ?       `alu_add :
                   (`addu|`addiu) ?     `alu_addu :
                   (`_and|`andi) ?      `alu_and :
                   (`_xor|`xori) ?      `alu_xor :
                   (`_nor) ?            `alu_nor :
                   (`jalr|`jr|`ori|`_or) ? `alu_or :
                   (`slt|`slti) ?       `alu_slt :
                   (`sltu|`sltiu) ?     `alu_sltu : 
                   (`sll) ?             `alu_sll :
                   (`srl) ?             `alu_srl:
                   (`sra) ?             `alu_sra :
                                        `alu_add;
    
    assign MemWrite = `SW ;
    
    assign MemtoReg = (`R_CAL | `I_CAL)? 0 :
                      (`LD) ? 1 :
                      (`jal|`jalr) ? 2 : 2'b00;

    assign PCsrc = (`nop| `R_CAL | `I_CAL | `LS) ?   `npc_add4 :
						 (`j|`jal) ?                  `npc_j :
						 (`jr|`jalr) ?                `npc_jr :
						 `B ?                         `npc_branch :
						                              3'bxxx;

	 assign NPCsrc = (`j + `jal) ? 1'b1 :
	                 `B ?          1'b0 :
						           1'bx;
						           
	 assign LStype = (`lw|`sw) ?   3'b000:
					 (`lh|`sh) ?   3'b001:
					 (`lhu|`sb) ?  3'b010:
					 (`lb) ?       3'b011:
					 (`lbu) ?      3'b100:
					               0;
	 
endmodule