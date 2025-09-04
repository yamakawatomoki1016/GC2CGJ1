#include <Novice.h>
#include <math.h>
#include <cstdlib> // rand, srand
#include <ctime>   // time

const char kWindowTitle[] = "6044_\n";

// ボール構造体
struct Ball {
    int x, y;
    int radius;
    unsigned int color;
    int speed;
    bool isFixed;  // 固定されているか
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);

    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    int openHand = Novice::LoadTexture("./Resources./openHand.png");
    int closeHand = Novice::LoadTexture("./Resources./closeHand.png");

    srand((unsigned int)time(nullptr)); // 乱数の種

    // ボール（黒か白をランダムで10個生成）
    const int ballCount = 10;
    Ball balls[ballCount];
    for (int i = 0; i < ballCount; i++) {
        balls[i].x = 640;                       // 真ん中の線上
        balls[i].y = -i * 150;                  // 上からずらして配置
        balls[i].radius = 30;
        balls[i].color = (rand() % 2 == 0) ? BLACK : WHITE;
        balls[i].speed = 3 + rand() % 3;        // 落下速度ランダム（3〜5）
        balls[i].isFixed = false;               // 最初は落ちる
    }

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

        ///
        /// 更新処理ここから
        /// 

        // マウス座標
        Novice::GetMousePosition(&mousePosX, &mousePosY);
        Novice::SetMouseCursorVisibility(visibility);

        // ボールの落下処理（固定されていない && 掴まれていないボールだけ落ちる）
        for (int i = 0; i < ballCount; i++) {
            if (!balls[i].isFixed && grabbingIndex != i) {
                balls[i].y += balls[i].speed;

                // 下まで行ったら止まる（仮に画面下で固定）
                if (balls[i].y + balls[i].radius > 720) {
                    balls[i].y = 720 - balls[i].radius;
                    balls[i].isFixed = true;
                }
            }
        }

        // まだ掴んでいないときにクリックしたら、近いボールを掴む
        if (grabbingIndex == -1 && Novice::IsPressMouse(0)) {
            for (int i = 0; i < ballCount; i++) {
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

            // クリックを離したら掴むのをやめる → その場で固定
            if (!Novice::IsPressMouse(0)) {
                balls[grabbingIndex].isFixed = true;
                grabbingIndex = -1;
            }
        }

        ///
        /// 更新処理ここまで
        /// 

        ///
        /// 描画処理ここから
        /// 

        // 境界線
        Novice::DrawLine(kyoukaisenStartPosX, kyoukaisenStartPosY,
            kyoukaisenEndPosX, kyoukaisenEndPosY, RED);

        // ボール（10個）
        for (int i = 0; i < ballCount; i++) {
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

        ///
        /// 描画処理ここまで
        /// 

        Novice::EndFrame();

        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }

    Novice::Finalize();
    return 0;
}
