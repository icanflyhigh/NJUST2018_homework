`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/09/27 17:18:12
// Design Name: 
// Module Name: RF
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


module RF(
    input Clk,
    input Reset,
    input [4:0] R1,
    input [4:0] R2,
    input [4:0] Waddr,
    input [31:0] WD,
    input En,
    output [31:0] RD1,
    output [31:0] RD2
    );
    
	integer i;
	reg [31:0] rf [31:0];

	assign RD1 = rf[R1][31:0];
	assign RD2 = rf[R2][31:0];

	initial begin
		for(i=0;i<32;i=i+1) begin
			rf[i][31:0] = 0;
		end
	end

	always @(posedge Clk) begin
		if (Reset) begin
			for(i=0;i<32;i=i+1) begin
				rf[i][31:0] <= 0;
			end
			// reset
		end
		else if (En==1) begin
			if(Waddr == 0) begin
				rf[Waddr] <= 0;
			end
			else begin
				rf[Waddr] <= WD;
		   end
		end
	end
endmodule
