`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/09/27 17:18:12
// Design Name: 
// Module Name: IFU
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


module IFU(
    input Clk,
    input Reset,
    input [31:0] rd1,
    input Branch,
    input [25:0] I26,
    input [2:0]PCsrc,
    input NPCsrc,
    output [31:0] Instruction,
    output [31:0] PC4
    );
	integer i;
	wire [31:0] PCr, NPC;
	wire [31:0] RA;
    
    PC pc(.add4(PC4), .NPC(NPC), .j_r1(rd1), .PCsrc(PCsrc), .Branch(Branch), .Clk(Clk), .Reset(Reset), .PC(PCr));
	IM im(.addr(PCr), .instr(Instruction));
	
	assign NPC = (NPCsrc==1) ? {PC4[31:28],I26[25:0],2'b00} : (PC4 + {{14{I26[15]}},I26[15:0],2'b00});
	assign RA = PCr;
	assign PC4 = RA + 4;

endmodule


