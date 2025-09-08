#include <Novice.h>
#include <math.h>
#include <cstdlib> // rand, srand
#include <ctime>   // time

const char kWindowTitle[] = "6044_\n";

// ボール構造体
struct Ball {
    float x, y;
    int radius;
    unsigned int color;
    float speed;     // 重力による落下速度
    float vx, vy;    // スライド用の速度
    bool isFixed;    // 固定されているか
    bool grabbed;    // 掴まれたことがあるか
    bool exploded;   // 爆発したかどうか
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 720);

    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    int openHand = Novice::LoadTexture("./Resources./openHand.png");
    int closeHand = Novice::LoadTexture("./Resources./closeHand.png");

    srand((unsigned int)time(nullptr)); // 乱数の種

    // ボール（15個生成）
    const int ballCount = 15;
    Ball balls[ballCount];
    for (int i = 0; i < ballCount; i++) {
        balls[i].x = 640.0f;                       // 画面中央
        balls[i].y = -i * 150.0f;                  // 上からずらして配置
        balls[i].radius = 30;
        balls[i].color = (rand() % 2 == 0) ? BLACK : WHITE;
        balls[i].speed = float(3 + rand() % 3);    // 落下速度ランダム（3〜5）
        balls[i].vx = 0.0f;
        balls[i].vy = 0.0f;
        balls[i].isFixed = false;                  // 落下スタート
        balls[i].grabbed = false;                  // まだ掴まれてない
        balls[i].exploded = false;                 // 爆発してない
    }

    // マウス
    int mousePosX = 0;
    int mousePosY = 0;
    int prevMouseX = 0;
    int prevMouseY = 0;
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

        /// 更新処理ここから ///

        // マウス座標
        Novice::GetMousePosition(&mousePosX, &mousePosY);
        Novice::SetMouseCursorVisibility(visibility);

        // ボールの落下 & 移動処理
        for (int i = 0; i < ballCount; i++) {
            if (!balls[i].isFixed && grabbingIndex != i && !balls[i].exploded) {
                // 重力で落下
                balls[i].y += balls[i].speed;

                // スライド移動
                balls[i].x += balls[i].vx;
                balls[i].y += balls[i].vy;

                // 摩擦で減速
                balls[i].vx *= 0.95f;
                balls[i].vy *= 0.95f;

                // 下まで行ったら止まる or 爆発
                if (balls[i].y + balls[i].radius > 720) {
                    balls[i].y = 720.0f - (float)balls[i].radius;

                    if (!balls[i].grabbed) {
                        balls[i].exploded = true;   // 掴まれなかった → 爆発
                    }
                    else {
                        balls[i].isFixed = true;    // 掴まれた → 固定
                    }

                    balls[i].vx = balls[i].vy = 0;
                }

                // 左右の壁で跳ね返る
                if (balls[i].x - balls[i].radius < 0) {
                    balls[i].x = (float)balls[i].radius;
                    balls[i].vx = -balls[i].vx * 0.5f;
                }
                if (balls[i].x + balls[i].radius > 1280) {
                    balls[i].x = 1280 - (float)balls[i].radius;
                    balls[i].vx = -balls[i].vx * 0.5f;
                }
            }
        }

        // 掴む処理
        if (grabbingIndex == -1 && Novice::IsPressMouse(0)) {
            for (int i = 0; i < ballCount; i++) {
                if (balls[i].exploded) continue; // 爆発済みは掴めない

                float dx = balls[i].x - mousePosX;
                float dy = balls[i].y - mousePosY;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist <= balls[i].radius + mouseRadius) {
                    grabbingIndex = i;
                    balls[i].grabbed = true; // 掴まれた記録
                    break;
                }
            }
        }

        // 掴んでいる間
        if (grabbingIndex != -1) {
            Ball& b = balls[grabbingIndex];

            // マウス移動量から速度を計算
            b.vx = float(mousePosX - prevMouseX);
            b.vy = float(mousePosY - prevMouseY);

            // ボールをマウス位置に追従
            b.x = (float)mousePosX;
            b.y = (float)mousePosY;

            // 離したらスライド開始
            if (!Novice::IsPressMouse(0)) {
                b.isFixed = false;
                grabbingIndex = -1;
            }
        }

        // マウス座標を保存
        prevMouseX = mousePosX;
        prevMouseY = mousePosY;

        /// 更新処理ここまで ///

        /// 描画処理ここから ///

        // 境界線
        Novice::DrawLine(kyoukaisenStartPosX, kyoukaisenStartPosY,
            kyoukaisenEndPosX, kyoukaisenEndPosY, RED);

        // ボール描画
        for (int i = 0; i < ballCount; i++) {
            if (balls[i].exploded) {
                // 爆発エフェクト（赤い大きな円を一瞬描画）
                Novice::DrawEllipse(
                    (int)balls[i].x, (int)balls[i].y,
                    balls[i].radius * 2, balls[i].radius * 2,
                    0.0f, RED, kFillModeSolid
                );
                continue;
            }

            Novice::DrawEllipse(
                (int)balls[i].x, (int)balls[i].y,
                balls[i].radius, balls[i].radius,
                0.0f, balls[i].color, kFillModeSolid
            );
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

        /// 描画処理ここまで ///

        Novice::EndFrame();

        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }

    Novice::Finalize();
    return 0;
}
