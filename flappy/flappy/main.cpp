#define _CRT_SECURE_NO_WARNINGS
#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// โครงสร้างปุ่มกด (ใช้ LPCTSTR รองรับระบบสากลชัวร์ 100%)
typedef struct {
    int x, y, width, height;
    LPCTSTR text;
    COLORREF color;
} Button;

// สถานะเกม
int gameState = 0;    // 0: Menu, 1: Playing, 2: Game Over
int score = 0;
int highScore = 0;

// ตัวแปรสำหรับนก
float birdY = 240;
float birdVelocity = 0;
const float gravity = 0.6f;
const float jumpStrength = -9.0f;
const int birdX = 150;
const int birdRadius = 15;

// ตัวแปรสำหรับท่อ (Obstacles)
int pipeX = 720;
int pipeGap = 150;
int pipeWidth = 60;
int topPipeHeight = 150;
const float pipeSpeed = 5.0f;

// ฟังก์ชันวาดปุ่ม (แก้ไขเอา _M ออกเรียบร้อยแล้วครับ)
void drawBtn(Button b) {
    setfillcolor(b.color);
    setlinecolor(BLACK);
    fillrectangle(b.x, b.y, b.x + b.width, b.y + b.height);
    settextcolor(WHITE);
    setbkmode(TRANSPARENT);
    settextstyle(16, 0, _T("Tahoma"));

    // ใช้ฟังก์ชันดั้งเดิมของ EasyX วัดขนาดตัวหนังสือได้กึ่งกลางปุ่มพอดี ไม่ฟ้อง Error
    outtextxy(b.x + (b.width - textwidth(b.text)) / 2, b.y + (b.height - textheight(b.text)) / 2, b.text);
}

// เช็กคลิกปุ่ม
bool isClick(int mx, int my, int x, int y, int w, int h) {
    return (mx >= x && mx <= x + w && my >= y && my <= y + h);
}

// รีเซ็ตค่าเพื่อเริ่มเกมใหม่
void resetGame() {
    birdY = 240;
    birdVelocity = 0;
    pipeX = 720;
    topPipeHeight = 100 + rand() % 180;
    score = 0;
}

// 0. หน้าเมนูหลัก
void drawMenuScreen() {
    cleardevice();
    settextcolor(BLACK);
    settextstyle(40, 0, _T("Tahoma"));
    outtextxy(260, 100, _T("Flappy Bird"));

    settextstyle(18, 0, _T("Tahoma"));
    settextcolor(BLACK);
    outtextxy(260, 160, _T("Press SPACEBAR to Fly!"));

    drawBtn({ 260, 230, 200, 45, _T("Start Game"), GREEN });
    drawBtn({ 260, 290, 200, 45, _T("Exit"), RED });
}

// 1. หน้าฉากเล่นเกม (ตัวนกมีปีก มีปาก มีตา)
void drawGameplayScreen() {
    cleardevice();

    // วาดพื้นหลัง (ท้องฟ้าสีฟ้าอ่อน)
    setfillcolor(0xE6F2FF);
    solidrectangle(0, 0, 720, 480);

    // วาดท่อบน (สีเขียว)
    setfillcolor(GREEN);
    setlinecolor(BLACK);
    fillrectangle(pipeX, 0, pipeX + pipeWidth, topPipeHeight);

    // วาดท่อล่าง (สีเขียว)
    int bottomPipeY = topPipeHeight + pipeGap;
    fillrectangle(pipeX, bottomPipeY, pipeX + pipeWidth, 480);

    // --- ส่วนประกอบการวาดรูปตัวนก ---
    int bx = birdX;
    int by = (int)birdY;
    setlinecolor(BLACK);

    // 1. ตัวหลัก (วงกลมสีเหลือง)
    setfillcolor(YELLOW);
    fillcircle(bx, by, birdRadius);

    // 2. ปีกนก (วงรีสีเหลืองเข้ม)
    setfillcolor(0xFFE066);
    fillellipse(bx - 10, by - 2, bx, by + 8);

    // 3. ตานก (ตาขาว + ตาดำ)
    setfillcolor(WHITE);
    fillcircle(bx + 6, by - 6, 4);
    setfillcolor(BLACK);
    fillcircle(bx + 7, by - 6, 1.5);

    // 4. ปากนก (สามเหลี่ยมสีส้มยื่นไปข้างหน้า ระบบสี Hex ใน EasyX เป็นแบบ BGR)
    setfillcolor(0x0066FF);
    POINT pts[3] = {
        { bx + 12, by + 1 },
        { bx + 22, by + 4 },
        { bx + 11, by + 7 }
    };
    solidpolygon(pts, 3);
    polygon(pts, 3);

    // แสดงคะแนนปัจจุบัน
    settextcolor(BLACK);
    settextstyle(24, 0, _T("Tahoma"));
    TCHAR txtScore[30];
    _stprintf_s(txtScore, _T("Score: %d"), score);
    outtextxy(30, 30, txtScore);
}

// 2. หน้าจอ Game Over
void drawGameOverScreen() {
    cleardevice();
    settextcolor(RED);
    settextstyle(45, 0, _T("Tahoma"));
    outtextxy(240, 100, _T("Game Over"));

    settextcolor(BLACK);
    settextstyle(20, 0, _T("Tahoma"));

    TCHAR txtFinal[30];
    _stprintf_s(txtFinal, _T("Your Score: %d"), score);
    outtextxy(290, 170, txtFinal);

    TCHAR txtHigh[30];
    _stprintf_s(txtHigh, _T("High Score: %d"), highScore);
    outtextxy(290, 200, txtHigh);

    drawBtn({ 260, 270, 200, 45, _T("Play Again"), GREEN });
    drawBtn({ 260, 330, 200, 45, _T("Main Menu"), 0x00A5FF });
}

// ฟังก์ชันหลักคุม Loop ระบบทั้งหมด
int main() {
    srand((unsigned int)time(NULL));
    initgraph(720, 480);
    setbkcolor(WHITE);
    BeginBatchDraw();

    ExMessage m;

    while (true) {
        if (gameState == 0) {
            drawMenuScreen();
        }
        else if (gameState == 1) {
            birdVelocity += gravity;
            birdY += birdVelocity;
            pipeX -= (int)pipeSpeed;

            if (pipeX + pipeWidth < 0) {
                pipeX = 720;
                topPipeHeight = 100 + rand() % 180;
                score++;
                if (score > highScore) {
                    highScore = score;
                }
            }

            if (birdY - birdRadius < 0 || birdY + birdRadius > 480) {
                gameState = 2;
            }

            if (birdX + birdRadius > pipeX && birdX - birdRadius < pipeX + pipeWidth) {
                if (birdY - birdRadius < topPipeHeight || birdY + birdRadius > topPipeHeight + pipeGap) {
                    gameState = 2;
                }
            }

            drawGameplayScreen();
            Sleep(20);
        }
        else if (gameState == 2) {
            drawGameOverScreen();
        }

        FlushBatchDraw();

        if (peekmessage(&m)) {
            if (m.message == WM_KEYDOWN && m.vkcode == VK_SPACE && gameState == 1) {
                birdVelocity = jumpStrength;
            }

            if (m.message == WM_LBUTTONDOWN) {
                if (gameState == 0) {
                    if (isClick(m.x, m.y, 260, 230, 200, 45)) { resetGame(); gameState = 1; }
                    if (isClick(m.x, m.y, 260, 290, 200, 45)) break;
                }
                else if (gameState == 2) {
                    if (isClick(m.x, m.y, 260, 270, 200, 45)) { resetGame(); gameState = 1; }
                    if (isClick(m.x, m.y, 260, 330, 200, 45)) gameState = 0;
                }
            }
            if (m.message == WM_KEYDOWN && m.vkcode == VK_ESCAPE) break;
        }
    }

    EndBatchDraw();
    closegraph();
    return 0;
}