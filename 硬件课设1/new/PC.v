`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/09/27 17:18:12
// Design Name: 
// Module Name: PC
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

module PC(
    input [31:0] add4,
    input [31:0] NPC,
    input [31:0] j_r1,
    input [2:0] PCsrc,
    input Branch,
    input Clk,
    input Reset,
    output reg [31:0] PC
    );
	 

	 wire [31:0] PC_sel,NPC_in;
	 integer PC_RESET = 32'h00000000;
	 
	 assign PC_sel = (PCsrc==`npc_add4) ?                          add4 :
	 				 (PCsrc==`npc_j) ?                             NPC :
	 				 (PCsrc==`npc_jr) ?                            j_r1 :  //jrÌø×ªrs£¬bgezalrÌø×ªrt
					 (PCsrc==`npc_branch) ?  ((Branch==1) ? NPC :  add4) :
					                                               add4;
	 assign NPC_in =  PC_sel ;
	 
	 initial begin
		PC = PC_RESET;
	 end

	 always@(posedge Clk) begin
		if(Reset) begin
			PC <= PC_RESET;
		end
		else begin
            PC <= NPC_in;
		end
	 end


endmodule
