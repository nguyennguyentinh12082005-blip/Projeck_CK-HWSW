`timescale 1ns / 1ps

module tb_xfft_8;

    reg aclk;
    reg aresetn;

    reg  [7:0]  s_axis_config_tdata;
    reg         s_axis_config_tvalid;
    wire        s_axis_config_tready;

    reg  [63:0] s_axis_data_tdata;
    reg         s_axis_data_tvalid;
    wire        s_axis_data_tready;
    reg         s_axis_data_tlast;

    wire [63:0] m_axis_data_tdata;
    wire        m_axis_data_tvalid;
    reg         m_axis_data_tready;
    wire        m_axis_data_tlast;

    wire event_frame_started;
    wire event_tlast_unexpected;
    wire event_tlast_missing;
    wire event_status_channel_halt;
    wire event_data_in_channel_halt;
    wire event_data_out_channel_halt;

    // Clock 100 MHz
    initial begin
        aclk = 0;
        forever #5 aclk = ~aclk;
    end

    // DUT: FFT IP 8-point
    design_1_xfft_0_0 dut (
        .aclk(aclk),
        .aresetn(aresetn),

        .s_axis_config_tdata(s_axis_config_tdata),
        .s_axis_config_tvalid(s_axis_config_tvalid),
        .s_axis_config_tready(s_axis_config_tready),

        .s_axis_data_tdata(s_axis_data_tdata),
        .s_axis_data_tvalid(s_axis_data_tvalid),
        .s_axis_data_tready(s_axis_data_tready),
        .s_axis_data_tlast(s_axis_data_tlast),

        .m_axis_data_tdata(m_axis_data_tdata),
        .m_axis_data_tvalid(m_axis_data_tvalid),
        .m_axis_data_tready(m_axis_data_tready),
        .m_axis_data_tlast(m_axis_data_tlast),

        .event_frame_started(event_frame_started),
        .event_tlast_unexpected(event_tlast_unexpected),
        .event_tlast_missing(event_tlast_missing),
        .event_status_channel_halt(event_status_channel_halt),
        .event_data_in_channel_halt(event_data_in_channel_halt),
        .event_data_out_channel_halt(event_data_out_channel_halt)
    );

    // Hàm ?óng gói s? float complex:
    // TDATA[31:0]  = real
    // TDATA[63:32] = imag
    function [63:0] complex_float;
        input [31:0] real_part;
        input [31:0] imag_part;
        begin
            complex_float = {imag_part, real_part};
        end
    endfunction

    task send_sample;
        input [63:0] data;
        input last;
        begin
            @(posedge aclk);
            s_axis_data_tdata  <= data;
            s_axis_data_tvalid <= 1'b1;
            s_axis_data_tlast  <= last;

            while (!s_axis_data_tready) begin
                @(posedge aclk);
            end

            @(posedge aclk);
            s_axis_data_tvalid <= 1'b0;
            s_axis_data_tlast  <= 1'b0;
            s_axis_data_tdata  <= 64'd0;
        end
    endtask

    initial begin
        aresetn = 0;

        s_axis_config_tdata  = 8'h01;   // Forward FFT
        s_axis_config_tvalid = 0;

        s_axis_data_tdata  = 64'd0;
        s_axis_data_tvalid = 0;
        s_axis_data_tlast  = 0;

        m_axis_data_tready = 1'b1;

        #100;
        aresetn = 1;
        #50;

        // G?i config cho FFT IP
        @(posedge aclk);
        s_axis_config_tvalid <= 1'b1;

        while (!s_axis_config_tready) begin
            @(posedge aclk);
        end

        @(posedge aclk);
        s_axis_config_tvalid <= 1'b0;

        #50;

        // Input gi?ng code Vitis:
        // 11+23i, 32+10i, 91+94i, 15+69i,
        // 47+96i, 44+12i, 96+17i, 49+58i

        send_sample(complex_float(32'h41300000, 32'h41B80000), 1'b0); // 11 + 23i
        send_sample(complex_float(32'h42000000, 32'h41200000), 1'b0); // 32 + 10i
        send_sample(complex_float(32'h42B60000, 32'h42BC0000), 1'b0); // 91 + 94i
        send_sample(complex_float(32'h41700000, 32'h428A0000), 1'b0); // 15 + 69i
        send_sample(complex_float(32'h423C0000, 32'h42C00000), 1'b0); // 47 + 96i
        send_sample(complex_float(32'h42300000, 32'h41400000), 1'b0); // 44 + 12i
        send_sample(complex_float(32'h42C00000, 32'h41880000), 1'b0); // 96 + 17i
        send_sample(complex_float(32'h42440000, 32'h42680000), 1'b1); // 49 + 58i, TLAST

        #2000;

        $stop;
    end

endmodule