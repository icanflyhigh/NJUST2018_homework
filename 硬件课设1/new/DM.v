`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2021/09/27 17:18:12
// Design Name: 
// Module Name: DM
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


module DM(
    input Clk,
    input Reset,
    input [31:0] Addr,
    input [31:0] WD,
    input WE,
    input [2:0] LStype,
    output [31:0] RD
    );
    
	reg[31:0] dm[0:1023];
	wire[31:0] wdata;
	integer i;
	
	assign RD = (LStype == 3'b000)?dm[Addr[11:2]]:
				(LStype == 3'b001 && Addr[1] == 0)?{{16{dm[Addr[11:2]][15]}},dm[Addr[11:2]][15:0]}:
				(LStype == 3'b001 && Addr[1] == 1)?{{16{dm[Addr[11:2]][31]}},dm[Addr[11:2]][31:16]}:
				(LStype == 3'b010 && Addr[1] == 0)?{16'h0000,dm[Addr[11:2]][15:0]}:
				(LStype == 3'b010 && Addr[1] == 1)?{16'h0000,dm[Addr[11:2]][31:16]}:
				(LStype == 3'b011 && Addr[1:0] == 0)?{{24{dm[Addr[11:2]][7]}},dm[Addr[11:2]][7:0]}:
				(LStype == 3'b011 && Addr[1:0] == 1)?{{24{dm[Addr[11:2]][15]}},dm[Addr[11:2]][15:8]}:
				(LStype == 3'b011 && Addr[1:0] == 2)?{{24{dm[Addr[11:2]][23]}},dm[Addr[11:2]][23:16]}:
				(LStype == 3'b011 && Addr[1:0] == 3)?{{24{dm[Addr[11:2]][31]}},dm[Addr[11:2]][31:24]}:
				(LStype == 3'b100 && Addr[1:0] == 0)?{24'h000000,dm[Addr[11:2]][7:0]}:
				(LStype == 3'b100 && Addr[1:0] == 1)?{24'h000000,dm[Addr[11:2]][15:8]}:
				(LStype == 3'b100 && Addr[1:0] == 2)?{24'h000000,dm[Addr[11:2]][23:16]}:
				(LStype == 3'b100 && Addr[1:0] == 3)?{24'h000000,dm[Addr[11:2]][31:24]}:0;
				
	assign wdata = (LStype == 3'b000)?WD:
                   (LStype == 3'b001 && Addr[1] == 0)?{dm[Addr[11:2]][31:16],WD[15:0]}:
                   (LStype == 3'b001 && Addr[1] == 1)?{WD[15:0],dm[Addr[11:2]][15:0]}:
                   (LStype == 3'b010 && Addr[1:0] == 0)?{dm[Addr[11:2]][31:8],WD[7:0]}:
                   (LStype == 3'b010 && Addr[1:0] == 1)?{dm[Addr[11:2]][31:16],WD[7:0],dm[Addr[11:2]][7:0]}:
                   (LStype == 3'b010 && Addr[1:0] == 2)?{dm[Addr[11:2]][31:24],WD[7:0],dm[Addr[11:2]][15:0]}:
                   (LStype == 3'b010 && Addr[1:0] == 3)?{WD[7:0],dm[Addr[11:2]][23:0]}:0;
					  
	initial begin
		for(i=0;i<1024;i=i+1) begin
				dm[i] <= 32'b0;
			end
	end
	
	always @(posedge Clk) begin
		if(Reset == 1) begin
			for(i=0;i<1024;i=i+1) begin
				dm[i] <= 32'b0;
			end
		end	
		else if(Reset == 0 && WE == 1) begin
			dm[Addr[11:2]] <= wdata;
		end
	end
					  
endmodule