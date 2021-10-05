`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/09/27 17:14:58
// Design Name: 
// Module Name: ALU
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

`include"INS.vh"
module ALU(
    input [31:0] SrcA,
    input [31:0] SrcB,
    input [4:0] Shift,
    input [4:0] ALUop,
    output [31:0] ALUresult,
    output ov
    );


	 integer i;
	 reg [31:0] result;
	 reg V;
	 reg [32:0] temp;
	 
	 assign ALUresult = result ;
	 assign ov = V;
	 
	 initial begin
		result = 0;
	 end
	 
	 
	 always @(*) begin
		case(ALUop) 
			`alu_add: begin
				temp = {SrcA[31],SrcA} + {SrcB[31],SrcB} ;
				V = temp[32] ^ temp[31] ;
				result = temp[31:0];
			end
			`alu_addu: begin
				result=SrcA + SrcB;
			end
			`alu_sub: begin
				temp = {SrcA[31],SrcA} - {SrcB[31],SrcB} ;
				V = temp[32] ^ temp[31] ;
				result = temp[31:0];    
			end
            `alu_or: begin
		    	result=SrcA | SrcB;
		    end
			`alu_subu: begin
				result=SrcA - SrcB;
		    end
		    `alu_and: begin
		    	result=SrcA & SrcB;
		    end
		    `alu_xor: begin
		    	result=SrcA ^ SrcB;
		    end
		    `alu_nor: begin
		    	result=~ (SrcA | SrcB);
		    end
		    `alu_sll: begin
		    	result=SrcB << Shift;
		    end
		    `alu_srl: begin
		    	result=SrcB >> Shift;
		    end
		    `alu_sra: begin
		    	result=$signed(SrcB) >>> Shift;
		    end
		    `alu_slt: begin
		    	result=($signed(SrcA) < $signed(SrcB))? 1 : 0;
		    end
		    `alu_sltu: begin
		    	result=(SrcA < SrcB)? 1 : 0;
		    end
		endcase
	end

endmodule 

