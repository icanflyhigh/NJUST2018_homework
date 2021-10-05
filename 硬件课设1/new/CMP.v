`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/09/29 15:10:02
// Design Name: 
// Module Name: CMP
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

module CMP(
    input [31:0] RD1,
    input [31:0] RD2,
    input [31:0] Instr,
    output Branch
    );
	 
	 wire [31:0] instr;
	 assign instr = Instr;
	 
	 assign Branch = (`beq && (RD1==RD2)) |
				     (`bgez && ($signed(RD1)>=0)) |
				     (`bgtz && ($signed(RD1)>0)) |
				     (`blez && ($signed(RD1)<=0)) |
				     (`bltz && ($signed(RD1)<0)) |
				     (`bne && (RD1!=RD2)) ?  1:0;


endmodule
