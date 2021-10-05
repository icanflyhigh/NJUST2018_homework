`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/09/27 17:18:12
// Design Name: 
// Module Name: IM
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


module IM(
    input [31:0] addr,
	 output [31:0] instr
    );
	 
	reg [31:0] IM[127:0];
	integer i;
	wire [31:0] addr1;
	
	assign addr1 = addr[8:2];
	
	initial begin
		for(i=0; i<128; i=i+1) begin
			IM[i] = 32'b0;
		end
		$readmemh("D:/vivado_workplace/IM_Management/IM.txt",IM);
	end
	
	assign instr = IM[addr1][31:0];

endmodule

