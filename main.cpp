#include <Novice.h>
#include <math.h>

const char kWindowTitle[] = "6044_\n";

// ボール構造体
struct Ball {
    int x, y;
    int radius;
    unsigned int color;
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);

    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    int openHand = Novice::LoadTexture("./Resources./openHand.png");
    int closeHand = Novice::LoadTexture("./Resources./closeHand.png");

    // ボール（黒と白）
    Ball balls[2] = {
        {640, 250, 30, BLACK},  // 黒
        {640, 550, 30, WHITE}   // 白
    };

    // マウス
    int mousePosX = 0;
    int mousePosY = 0;
    int mouseRadius = 15;

    // 境界線
    int kyoukaisenStartPosX = 640;
    int kyoukaisenStartPosY = 0;
    int kyoukaisenEndPosX = 640;
    int kyoukaisenEndPosY = 720;

    int visibility = 0;  // マウスカーソル非表示

    // 掴んでいるかどうか（どのボールか -1 なら掴んでない）
    int grabbingIndex = -1;

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();

        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        // マウス座標
        Novice::GetMousePosition(&mousePosX, &mousePosY);
        Novice::SetMouseCursorVisibility(visibility);

        // まだ掴んでいないときにクリックしたら、近いボールを掴む
        if (grabbingIndex == -1 && Novice::IsPressMouse(0)) {
            for (int i = 0; i < 2; i++) {
                float dx = float(balls[i].x - mousePosX);
                float dy = float(balls[i].y - mousePosY);
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist <= balls[i].radius + mouseRadius) {
                    grabbingIndex = i; // このボールを掴む
                    break;
                }
            }
        }

        // 掴んでいる間はボールをマウスに追従
        if (grabbingIndex != -1) {
            balls[grabbingIndex].x = mousePosX;
            balls[grabbingIndex].y = mousePosY;

            // クリックを離したら掴むのをやめる
            if (!Novice::IsPressMouse(0)) {
                grabbingIndex = -1;
            }
        }

        // === 描画 ===
        // 境界線
        Novice::DrawLine(kyoukaisenStartPosX, kyoukaisenStartPosY,
            kyoukaisenEndPosX, kyoukaisenEndPosY, RED);

        // ボール（2つ）
        for (int i = 0; i < 2; i++) {
            Novice::DrawEllipse(balls[i].x, balls[i].y, balls[i].radius, balls[i].radius,
                0.0f, balls[i].color, kFillModeSolid);
        }

        // 手の描画
        if (Novice::IsPressMouse(0)) {
            Novice::DrawSprite(mousePosX - mouseRadius, mousePosY - mouseRadius,
                closeHand, 1.0f, 1.0f, 0.0f, WHITE);
        }
        else {
            Novice::DrawSprite(mousePosX - mouseRadius, mousePosY - mouseRadius,
                openHand, 1.0f, 1.0f, 0.0f, WHITE);
        }

        Novice::EndFrame();

        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }

    Novice::Finalize();
    return 0;
}
