# Bộ Gia Tốc FFT 8 Điểm trên Nền Tảng Zynq SoC

Dự án thiết kế và mô phỏng bộ gia tốc FFT (Fast Fourier Transform) 8 điểm dựa trên nền tảng Zynq SoC, sử dụng công nghệ FPGA và giao tiếp AXI DMA để tăng tốc xử lý tín hiệu số.

## 🎥 Video Demo
Dưới đây là video demo chạy mô phỏng và giới thiệu chi tiết về hoạt động của hệ thống:

[![Video Demo](https://img.youtube.com/vi/yyik_VXHduI/0.jpg)](https://www.youtube.com/watch?v=yyik_VXHduI)

*Click vào hình ảnh trên để xem video trực tiếp trên YouTube.*

---

## 📌 Giới Thiệu Đề Tài
Đề tài tập trung vào việc nghiên cứu, thiết kế và kiểm chứng bộ gia tốc FFT 8 điểm trên vùng logic lập trình (Programmable Logic - PL) của Zynq SoC, giao tiếp với Processing System (PS) thông qua bộ điều khiển **AXI DMA** và luồng dữ liệu **AXI-Stream**.

- **Đầu vào/Đầu ra:** Nhận frame gồm 8 mẫu số phức (phần thực và ảo biểu diễn bằng số thực dấu phẩy động 32-bit Single Precision).
- **Kiến trúc FFT IP:** Sử dụng kiến trúc Pipelined Streaming I/O với thuật toán Radix-2.
- **Truyền nhận dữ liệu:** Kênh MM2S truyền dữ liệu từ DDR RAM sang FFT IP và kênh S2MM nhận kết quả từ FFT IP ghi ngược lại DDR RAM.

## ⚙️ Sơ Đồ Khối Hệ Thống
Hệ thống bao gồm các thành phần chính:
1. **Processing System (PS):** Bộ vi xử lý ARM điều khiển quá trình cấu hình DMA, chuẩn bị dữ liệu đầu vào và kiểm tra kết quả đầu ra.
2. **AXI DMA:** Trung chuyển dữ liệu tốc độ cao giữa DDR RAM và FFT IP qua hai kênh: MM2S (Memory-Mapped to Stream) và S2MM (Stream to Memory-Mapped).
3. **FFT IP Core:** Thực hiện tính toán FFT 8 điểm trực tiếp trên phần cứng FPGA.

## 📊 Kết Quả Đánh Giá (Mô Phỏng)
### Tài Nguyên Phần Cứng Sử Dụng (Vivado Implementation)
- **LUT:** 8,511
- **FF:** 13,143
- **BRAM:** 24
- **DSP:** 8
- **Failed Routes:** 0 (Định tuyến thành công 100%)

### Công Suất Tiêu Thụ (Power Report Estimate)
- **Tổng công suất:** 1.774 W
- **Công suất động (Dynamic Power):** 1.631 W (Khối PS7 chiếm phần lớn với 1.529 W)
- **Công suất tĩnh (Static Power):** 0.143 W

---

## 📂 Cấu Trúc Thư Mục Dự Án
- `tutorial_final_project_N16/`: Dự án Vivado chứa cấu hình Block Design, FFT IP và AXI DMA.
- `CK_HWSW.docx`: Báo cáo chi tiết đề tài.
- `AXI DMA.docx`: Tài liệu nghiên cứu chi tiết về AXI DMA.
- `SƠ ĐỒ.docx` & các file `.drawio`, `.png`: Sơ đồ khối thiết kế hệ thống.
