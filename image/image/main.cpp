#define _CRT_SECURE_NO_WARNINGS
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <conio.h>

// --- โครงสร้างปุ่ม GUI ---
typedef struct {
	int x, y, w, h;
	LPCTSTR text; // ใช้ LPCTSTR เพื่อแก้ปัญหาเส้นแดงที่ข้อความ
} Button;

// --- ฟังก์ชันช่วยเหลือ (Utility Functions) ---

// คำนวณความกว้างแถวที่ต้องหารด้วย 4 ลงตัว (Alignment) ตามกฎ BMP Stride
int calculate_stride(int width, int bit_count) {
	return ((width * bit_count + 31) / 32) * 4;
}

// อ่านไฟล์ BMP 24-bit (ใช้ Struct ที่มีอยู่แล้วใน Windows ไม่ต้องประกาศซ้ำ)
unsigned char* read_bmp(const char* path, int& w, int& h) {
	FILE* fp = fopen(path, "rb");
	if (!fp) return NULL;

	BITMAPFILEHEADER fh;
	BITMAPINFOHEADER ih;

	fread(&fh, sizeof(fh), 1, fp);
	fread(&ih, sizeof(ih), 1, fp);

	// ตรวจสอบว่าเป็น BMP 24-bit จริงไหม
	if (ih.biBitCount != 24) {
		fclose(fp);
		return NULL;
	}

	w = ih.biWidth;
	h = abs(ih.biHeight);

	int stride = calculate_stride(w, 24);
	unsigned char* data = (unsigned char*)malloc(stride * h);

	if (data) {
		fseek(fp, fh.bfOffBits, SEEK_SET);
		fread(data, stride * h, 1, fp);
	}

	fclose(fp);
	return data;
}

// แสดงผลภาพบนหน้าจอ EasyX (รองรับทั้ง 24-bit สี และ 8-bit ขาวดำ)
void show_image(unsigned char* data, int w, int h, int startX, int startY, int bit_count) {
	int stride = calculate_stride(w, bit_count);
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			if (bit_count == 24) {
				unsigned char b = data[y * stride + x * 3 + 0];
				unsigned char g = data[y * stride + x * 3 + 1];
				unsigned char r = data[y * stride + x * 3 + 2];
				putpixel(startX + x, startY + (h - 1 - y), RGB(r, g, b));
			}
			else {
				unsigned char g = data[y * stride + x];
				putpixel(startX + x, startY + (h - 1 - y), RGB(g, g, g));
			}
		}
	}
}

// --- Image Processing Functions ---

// Task 2: 24-bit to 8-bit Grayscale (ขาวดำ)
unsigned char* convert_to_gray(unsigned char* data_in, int w, int h) {
	int stride_in = calculate_stride(w, 24);
	int stride_out = calculate_stride(w, 8);
	unsigned char* data_out = (unsigned char*)malloc(stride_out * h);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			unsigned char b = data_in[y * stride_in + x * 3 + 0];
			unsigned char g = data_in[y * stride_in + x * 3 + 1];
			unsigned char r = data_in[y * stride_in + x * 3 + 2];
			// สูตรจากสไลด์ HIT
			data_out[y * stride_out + x] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
		}
	}
	return data_out;
}

// Task 3: Sobel Edge Detection (ตรวจจับขอบ)
unsigned char* sobel_edge(unsigned char* gray_in, int w, int h) {
	int stride = calculate_stride(w, 8);
	unsigned char* data_out = (unsigned char*)malloc(stride * h);
	int gx[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
	int gy[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

	for (int y = 1; y < h - 1; y++) {
		for (int x = 1; x < w - 1; x++) {
			int sumX = 0, sumY = 0;
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					int val = gray_in[(y + i) * stride + (x + j)];
					sumX += val * gx[i + 1][j + 1];
					sumY += val * gy[i + 1][j + 1];
				}
			}
			int mag = (int)sqrt(sumX * sumX + sumY * sumY);
			data_out[y * stride + x] = (mag > 255) ? 255 : (unsigned char)mag;
		}
	}
	return data_out;
}

// Task 4: Laplacian Sharpening (เพิ่มความคมชัด)
unsigned char* sharpen_image(unsigned char* data_in, int w, int h) {
	int stride = calculate_stride(w, 24);
	unsigned char* data_out = (unsigned char*)malloc(stride * h);
	int kernel[3][3] = { {-1, -1, -1}, {-1, 9, -1}, {-1, -1, -1} };

	for (int y = 1; y < h - 1; y++) {
		for (int x = 1; x < w - 1; x++) {
			for (int c = 0; c < 3; c++) {
				int sum = 0;
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						sum += data_in[(y + i) * stride + (x + j) * 3 + c] * kernel[i + 1][j + 1];
					}
				}
				data_out[y * stride + x * 3 + c] = (sum > 255) ? 255 : (sum < 0 ? 0 : (unsigned char)sum);
			}
		}
	}
	return data_out;
}

// --- ฟังก์ชัน GUI วาดปุ่ม ---
void draw_button(Button b) {
	setfillcolor(0x3E80C0);
	fillrectangle(b.x, b.y, b.x + b.w, b.y + b.h);
	settextcolor(WHITE);
	setbkmode(TRANSPARENT);
	settextstyle(18, 0, _T("Arial"));
	outtextxy(b.x + (b.w - textwidth(b.text)) / 2, b.y + (b.h - textheight(b.text)) / 2, b.text);
}

int main() {
	// 1. สร้างหน้าต่าง GUI
	initgraph(800, 500);
	setbkcolor(0x1B436B);
	cleardevice();

	// 2. อ่านไฟล์รูปภาพ
	int w, h;
	unsigned char* original_data = read_bmp("input.bmp", w, h);

	if (!original_data) {
		settextcolor(RED);
		settextstyle(20, 0, _T("Arial"));
		outtextxy(10, 10, _T("Error: Cannot find input.bmp or it is not 24-bit!"));
		_getch(); // รอการกดปุ่ม
		return 0;
	}

	// 3. กำหนดปุ่ม
	Button btns[4] = {
		{635, 55, 150, 45, _T("Original Image")},
		{635, 150, 150, 45, _T("Convert Gray")},
		{635, 240, 150, 45, _T("Edge Detection")},
		{635, 330, 150, 45, _T("Sharpening")}
	};

	// 4. วาดปุ่มและรูปต้นฉบับ
	for (int i = 0; i < 4; i++) draw_button(btns[i]);
	show_image(original_data, w, h, 30, 50, 24);

	// 5. วนลูปตรวจจับการคลิกเมาส์
	ExMessage msg;
	while (true) {
		if (peekmessage(&msg, EM_MOUSE)) {
			if (msg.message == WM_LBUTTONDOWN) {
				// คลิกปุ่ม 1: รูปต้นฉบับ
				if (msg.x >= btns[0].x && msg.x <= btns[0].x + btns[0].w && msg.y >= btns[0].y && msg.y <= btns[0].y + btns[0].h) {
					show_image(original_data, w, h, 30, 50, 24);
				}
				// คลิกปุ่ม 2: แปลงขาวดำ
				else if (msg.x >= btns[1].x && msg.x <= btns[1].x + btns[1].w && msg.y >= btns[1].y && msg.y <= btns[1].y + btns[1].h) {
					unsigned char* gray = convert_to_gray(original_data, w, h);
					show_image(gray, w, h, 30, 50, 8);
					free(gray);
				}
				// คลิกปุ่ม 3: ตรวจจับขอบ
				else if (msg.x >= btns[2].x && msg.x <= btns[2].x + btns[2].w && msg.y >= btns[2].y && msg.y <= btns[2].y + btns[2].h) {
					unsigned char* gray = convert_to_gray(original_data, w, h);
					unsigned char* edge = sobel_edge(gray, w, h);
					show_image(edge, w, h, 30, 50, 8);
					free(gray); free(edge);
				}
				// คลิกปุ่ม 4: เพิ่มความคมชัด
				else if (msg.x >= btns[3].x && msg.x <= btns[3].x + btns[3].w && msg.y >= btns[3].y && msg.y <= btns[3].y + btns[3].h) {
					unsigned char* sharp = sharpen_image(original_data, w, h);
					show_image(sharp, w, h, 30, 50, 24);
					free(sharp);
				}
			}
		}
	}

	free(original_data);
	closegraph();
	return 0;
}