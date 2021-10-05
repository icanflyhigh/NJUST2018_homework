`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/09/27 17:14:58
// Design Name: 
// Module Name: CPU
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
 
module CPU(
	input clk,
	input reset);
	
	wire [31:0] jaddr, pcplus4, inst, wrdata, rd1, rd2, rdata, out, srca, srcb, aluresult;
	wire [25:0] I26;
	wire [15:0] in;
	wire [5:0] opcode, functcode;
	wire [4:0] ra1, ra2, wraddr, shift, aluop;
	wire [2:0] pcsrc, lstype;
	wire [1:0] memtoreg, extop, regdst;
	wire regwrite, memwrite, branch, npcsrc, alusrc, ov;

   
	assign opcode = inst[31:26];
	assign functcode = inst[5:0];
	assign ra1 = inst[25:21];
	assign ra2 = inst[20:16];
	assign wraddr = (regdst== `RegDst_A3) ? inst[15:11] :
					(regdst== `RegDst_A2) ? inst[20:16] :
					(regdst== `RegDst_ra) ? inst[25:21] : 0;
	assign in = inst[15:0];
	assign I26 = inst[25:0];
	assign srca = rd1;
	assign srcb = (alusrc==0) ? rd2 :
				  (alusrc==1) ? out : 0;
	assign shift = in[10:6];
   assign wrdata = (memtoreg==0) ? aluresult :
    				(memtoreg==1) ? rdata :
    				(memtoreg==2) ? pcplus4 : 0;
	assign jaddr = rd1;

	IFU ifu(.Clk(clk), .Reset(reset), .rd1(rd1), .Branch(branch), .I26(I26), 
	        .PCsrc(pcsrc), .NPCsrc(npcsrc), .Instruction(inst), .PC4(pcplus4));

	Crtl cu(.Instr(inst), .OPCode(opcode), .FunctCode(functcode), .MemtoReg(memtoreg), .MemWrite(memwrite),
		    .ALUsrc(alusrc), .ALUop(aluop), .EXTop(extop), .RegDst(regdst), .RegWrite(regwrite), .PCsrc(pcsrc),
		    .NPCsrc(npcsrc), .LStype(lstype));

    CMP cmp(.RD1(rd1), .RD2(rd2), .Instr(inst), .Branch(branch));

	RF rf( .R1(ra1), .R2(ra2), .Waddr(wraddr), .WD(wrdata), .Clk(clk), .En(regwrite), .Reset(reset), .RD1(rd1), .RD2(rd2));

	EXT ext( .In(in), .EXTop(extop), .Out(out));

	ALU alu( .SrcA(srca), .SrcB(srcb), .Shift(shift), .ALUop(aluop), .ALUresult(aluresult), .ov(ov));

	DM dm( .Addr(aluresult), .WD(rd2), .Clk(clk), .Reset(reset), .WE(memwrite), .LStype(lstype), .RD(rdata));

endmodule
