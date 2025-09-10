#define NOMINMAX 
#include <Novice.h>
#include <cmath>
#include <cstdlib> 
#include <ctime> 
#include <algorithm>

#define YELLOW 0xFFFFFF00
#define ORANGE 0xFFFFA500

const char kWindowTitle[] = "白黒ボムパニック！";

// 画面サイズ
const int SCREEN_W = 1280;
const int SCREEN_H = 720;

// ボール構造体
struct Ball {
    float x, y;
    float radius;
    unsigned int color;
    int image;
    float speed;     // 自動落下速度
    float vx, vy;    // 慣性速度
    bool isFixed;    // 地面で固定されているか
    bool beingHeld;  // 今掴んでいるか
    bool touched;    // 一度でも掴まれたか（掴まれたことがあるなら true）
    bool exploded;   // 爆発処理を行ったか
    bool active;     // 存在するか（爆発で消したい場合 false にする）
};

// パーティクル（爆発の破片）
struct Particle {
    float x, y;
    float vx, vy;
    float size;
    int life;        // 残フレーム
    int lifeMax;
    unsigned int color;
    bool active;
};

// パーティクル生成関数（外に出しておく）
void SpawnExplosion(Particle particles[], int maxParticles, float cx, float cy) {
    const int spawnCount = 24; // 1爆発あたりの破片数
    for (int n = 0; n < spawnCount; n++) {
        int slot = -1;
        for (int i = 0; i < maxParticles; i++) {
            if (!particles[i].active) {
                slot = i;
                break;
            }
        }
        if (slot == -1) return;

        // ランダムな角度と速度
        float angle = (float)(rand() % 360) * (3.14159265f / 180.0f);
        float speed = (float)(rand() % 80) / 10.0f + 2.5f; // 2.5〜10.4
        float vx = cosf(angle) * speed;
        float vy = sinf(angle) * speed;

        // ライフやサイズ
        int life = rand() % 30 + 30; // 30〜59 フレーム
        float size = (float)(rand() % 4 + 2); // 2〜5

        // 色（赤・黄・オレンジっぽいのをランダムに選ぶ）
        unsigned int color;
        int c = rand() % 3;
        if (c == 0) color = RED;
        else if (c == 1) color = YELLOW;
        else color = 0xFFFFA500;

        particles[slot].x = cx;
        particles[slot].y = cy;
        particles[slot].vx = vx;
        particles[slot].vy = vy * -1.0f; // 画面Yは下方向に増えるので反転して上方向にも飛ばす
        particles[slot].size = size;
        particles[slot].life = life;
        particles[slot].lifeMax = life;
        particles[slot].color = color;
        particles[slot].active = true;
    }
}

// シーン列挙型
enum Scene {
    TITLE,
    GAME,
    SCORE
};

// ----------------------------
// 初期化処理関数
// ----------------------------

int whiteBallGH;
int blackBallGH;

void InitGame(Ball balls[], int ballCount, Particle particles[], int maxParticles, int& missCount) {
    // ボール初期化
    for (int i = 0; i < ballCount; i++) {
        balls[i].x = SCREEN_W / 2.0f;
        balls[i].y = -i * 150.0f;
        balls[i].radius = 30.0f;
        if (rand() % 2 == 0) {
            balls[i].color = WHITE;          // ← 判定用
            balls[i].image = whiteBallGH;    // ← 描画用
        }
        else {
            balls[i].color = BLACK;          // ← 判定用
            balls[i].image = blackBallGH;    // ← 描画用
        }
        balls[i].speed = float(3 + rand() % 3); // 3〜5
        balls[i].vx = 0.0f;
        balls[i].vy = 0.0f;
        balls[i].isFixed = false;
        balls[i].beingHeld = false;
        balls[i].touched = false;
        balls[i].exploded = false;
        balls[i].active = true;
    }
    // パーティクル初期化
    for (int i = 0; i < maxParticles; i++) {
        particles[i].active = false;
    }
    // ミス回数リセット
    missCount = 0;
}

void SplitScoreToDigits(int score, int digits[], int maxDigits) {
    for (int i = maxDigits - 1; i >= 0; i--) {
        digits[i] = score % 10;
        score /= 10;
    }
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // ライブラリの初期化
    Novice::Initialize(kWindowTitle, 1280, 720);
    // 乱数のシードを初期化
    srand((unsigned int)time(NULL));
    // フルスクリーンにする
    HWND hwnd = GetForegroundWindow(); // 現在のウィンドウのハンドルを取得
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW); // ウィンドウのスタイルを変更
    SetWindowPos(hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED | SWP_NOZORDER | SWP_SHOWWINDOW); // ウィンドウ位置とサイズをフルスクリーンに設定
    ShowWindow(hwnd, SW_MAXIMIZE); // ウィンドウを最大化

    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    // 手の画像（あるなら）
    int openHand = Novice::LoadTexture("./Resources./openHand.png");
    int closeHand = Novice::LoadTexture("./Resources./closeHand.png");
    int title = Novice::LoadTexture("./Resources./title.png");
    int gameScene = Novice::LoadTexture("./Resources./gameScene.png");
    int result = Novice::LoadTexture("./Resources./result.png");
    whiteBallGH = Novice::LoadTexture("./Resources/whiteBomb.png");//白
    blackBallGH = Novice::LoadTexture("./Resources/blackBomb.png");//黒
    int lifeIcon = Novice::LoadTexture("./Resources/hp.png");//残機

    int numGH[10] = {};
    numGH[0] = Novice::LoadTexture("./Resources./0.png");
    numGH[1] = Novice::LoadTexture("./Resources./1.png");
    numGH[2] = Novice::LoadTexture("./Resources./2.png");
    numGH[3] = Novice::LoadTexture("./Resources./3.png");
    numGH[4] = Novice::LoadTexture("./Resources./4.png");
    numGH[5] = Novice::LoadTexture("./Resources./5.png");
    numGH[6] = Novice::LoadTexture("./Resources./6.png");
    numGH[7] = Novice::LoadTexture("./Resources./7.png");
    numGH[8] = Novice::LoadTexture("./Resources./8.png");
    numGH[9] = Novice::LoadTexture("./Resources./9.png");

    int gamaSceneBGM = Novice::LoadAudio("./Sound./gamaSceneBGM.mp3");

    srand((unsigned int)time(nullptr));

    // ボール数
    const int ballCount = 100;
    Ball balls[ballCount];

    // パーティクル配列（十分な数）
    const int maxParticles = 500;
    static Particle particles[maxParticles];
    for (int i = 0; i < maxParticles; i++) {
        particles[i].active = false;
    }

    // ボール初期化（上から落ちてくるイメージ）
    for (int i = 0; i < ballCount; i++) {
        balls[i].x = SCREEN_W / 2.0f;
        balls[i].y = -i * 150.0f;
        balls[i].radius = 30.0f;

        if (rand() % 2 == 0) {
            balls[i].color = WHITE;          // ← 判定用
            balls[i].image = whiteBallGH;    // ← 描画用
        }
        else {
            balls[i].color = BLACK;          // ← 判定用
            balls[i].image = blackBallGH;    // ← 描画用
        }

        balls[i].speed = float(3 + rand() % 3); // 3〜5
        balls[i].vx = 0.0f;
        balls[i].vy = 0.0f;
        balls[i].isFixed = false;
        balls[i].beingHeld = false;
        balls[i].touched = false; // 一度も掴まれていない
        balls[i].exploded = false;
        balls[i].active = true;
    }
    // マウス
    int mousePosX = 0;
    int mousePosY = 0;
    int prevMouseX = 0;
    int prevMouseY = 0;
    bool prevMouseDown = false;
    int mouseRadius = 15;
    // 掴んでいるボールのインデックス
    int grabbingIndex = -1;
    // 物理パラメータ
    const float gravity = 0.05f; // パーティクルや慣性に作用する重力っぽい値
    // --- シーン管理用変数 ---
    Scene scene = TITLE;
    int score = 0;
    int missCount = 0;   // ミスした回数
    const int maxMiss = 3;
    // ゲーム開始時のカウントダウン用
    int gameTimer = 0;
    bool gameStart = false;
    int playHandle = 1;
    //残機
    int lives;
    int numArray[3] = {};
    int hiScore = 0;
    // --- グローバル変数 ---
    int highScores[3] = { 0, 0, 0 }; // 上位3位のスコア
    InitGame(balls, ballCount, particles, maxParticles, missCount);

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();

        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        // マウス情報
        Novice::GetMousePosition(&mousePosX, &mousePosY);
        bool mouseDown = Novice::IsPressMouse(0) != 0;
        Novice::SetMouseCursorVisibility(0);

        // --- 更新処理 ------------------------------------------------

        switch (scene)
        {
        case TITLE:///////////////////////////////////////////////////////////////////
            if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {
                InitGame(balls, ballCount, particles, maxParticles, missCount);
                gameTimer = 0;
                gameStart = false;
                scene = GAME;
                if (!Novice::IsPlayingAudio(playHandle)) {
                    playHandle = Novice::PlayAudio(gamaSceneBGM, true, 1);
                }
            }
            break;
        case GAME:///////////////////////////////////////////////////////////////////
            //ゲーム開始タイマー（３秒）
            gameTimer++;
            if (gameTimer >= 180) {
                gameStart = true;
            }
            //ゲーム開始
            if (gameStart == true) {
                // 1) ボールの更新
                for (int i = 0; i < ballCount; i++) {
                    if (!balls[i].active) continue;

                    // 掴まれているボールはマウスに追従
                    if (balls[i].beingHeld) {
                        float dx = (float)(mousePosX - prevMouseX);
                        float dy = (float)(mousePosY - prevMouseY);
                        balls[i].vx = dx;
                        balls[i].vy = dy;

                        balls[i].x = (float)mousePosX;
                        balls[i].y = (float)mousePosY;
                    }
                    else if (!balls[i].isFixed && !balls[i].exploded) {

                        balls[i].vy += gravity * 0.16f;
                        balls[i].y += balls[i].speed + balls[i].vy;
                        balls[i].x += balls[i].vx;

                        // 減速（摩擦）
                        balls[i].vx *= 0.98f;
                        balls[i].vy *= 0.99f;

                        // 地面に着いたら判定
                        if (balls[i].y + balls[i].radius >= SCREEN_H) {
                            balls[i].y = SCREEN_H - balls[i].radius;

                            if (!balls[i].touched) {
                                // 一度も掴まれていなければ爆発
                                balls[i].exploded = true;
                                balls[i].active = false;
                                SpawnExplosion(particles, maxParticles, balls[i].x, balls[i].y);
                                missCount++;
                                if (missCount >= maxMiss) {
                                    Novice::StopAudio(playHandle);
                                    scene = SCORE; // 3回ミスで終了
                                }
                            }
                            else {
                                // --- 仕分け判定 ---
                                bool correct = false;
                                if (balls[i].color == WHITE && balls[i].x > SCREEN_W / 2) {
                                    correct = true; // 白は右
                                }
                                if (balls[i].color == BLACK && balls[i].x < SCREEN_W / 2) {
                                    correct = true; // 黒は左
                                }

                                if (correct) {
                                    score++; // 正解ならスコア加算
                                }
                                else {
                                    missCount++;
                                    if (missCount >= maxMiss) {
                                        Novice::StopAudio(playHandle);
                                        scene = SCORE;
                                    }
                                }

                                // 正解でも不正解でも床で止める
                                balls[i].isFixed = true;
                                balls[i].vx = balls[i].vy = 0.0f;
                            }
                        }

                        // 壁の当たり判定（左右）
                        if (balls[i].x - balls[i].radius < 0.0f) {
                            balls[i].x = balls[i].radius;
                            balls[i].vx = -balls[i].vx * 0.5f;
                        }
                        if (balls[i].x + balls[i].radius > SCREEN_W) {
                            balls[i].x = SCREEN_W - balls[i].radius;
                            balls[i].vx = -balls[i].vx * 0.5f;
                        }
                    }
                }

                // 2) マウス入力（掴む/離すの判定）
                if (!prevMouseDown && mouseDown) {
                    for (int i = ballCount - 1; i >= 0; i--) {
                        if (!balls[i].active || balls[i].exploded || balls[i].touched) continue;
                        float dx = balls[i].x - (float)mousePosX;
                        float dy = balls[i].y - (float)mousePosY;
                        float dist2 = dx * dx + dy * dy;
                        if (dist2 <= (balls[i].radius + mouseRadius) * (balls[i].radius + mouseRadius)) {
                            grabbingIndex = i;
                            balls[i].beingHeld = true;
                            balls[i].touched = true;
                            prevMouseX = mousePosX;
                            prevMouseY = mousePosY;
                            break;
                        }
                    }
                }
                else if (prevMouseDown && !mouseDown) {
                    if (grabbingIndex != -1) {
                        Ball& b = balls[grabbingIndex];
                        b.beingHeld = false;
                        b.vx *= 1.2f;
                        b.vy *= 1.2f;
                        grabbingIndex = -1;
                    }
                }

                // 3) パーティクル更新
                for (int i = 0; i < maxParticles; i++) {
                    if (!particles[i].active) continue;
                    // 重力
                    particles[i].vy += gravity;
                    particles[i].x += particles[i].vx;
                    particles[i].y += particles[i].vy;

                    // 徐々に減速させる
                    particles[i].vx *= 0.995f;
                    particles[i].vy *= 0.995f;

                    // 寿命を減らす
                    particles[i].life--;
                    if (particles[i].life <= 0) {
                        particles[i].active = false;
                    }

                    // 床に当たったら跳ねて減速させる（簡易）
                    if (particles[i].y + particles[i].size >= SCREEN_H) {
                        particles[i].y = SCREEN_H - particles[i].size;
                        particles[i].vy *= -0.4f;
                        particles[i].vx *= 0.6f;
                    }
                }
            }
            break;
        case SCORE:///////////////////////////////////////////////////////////////////
            if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {
                // 初期化してタイトルに戻る
                InitGame(balls, ballCount, particles, maxParticles, missCount);
                scene = TITLE;
            }
        }


        // --- 更新処理 ------------------------------------------------
        

        // --- 描画処理 ------------------------------------------------

        switch (scene)
        {
        case TITLE:
            Novice::DrawSprite(0, 0, title, 1.0f, 1.0f, 0.0f, WHITE);

            score = 0;
            break;
        case GAME:
            Novice::DrawSprite(0, 0, gameScene, 1.0f, 1.0f, 0.0f, WHITE);
            // 境界線（任意）
            Novice::DrawLine(SCREEN_W / 2, 0, SCREEN_W / 2, SCREEN_H, RED);

            for (int i = 0; i < maxParticles; i++) {
                if (!particles[i].active) continue;
                float t = particles[i].life / (float)particles[i].lifeMax;
                int drawSize = (int)std::max(1.0f, particles[i].size * t);
                Novice::DrawEllipse((int)particles[i].x, (int)particles[i].y,
                    drawSize, drawSize, 0.0f, particles[i].color, kFillModeSolid);
            }

            // ボールを描画
            for (int i = 0; i < ballCount; i++) {
                // ボール描画
                if (balls[i].active) {
                    Novice::DrawSprite(
                        int(balls[i].x - balls[i].radius), // 左上X
                        int(balls[i].y - balls[i].radius), // 左上Y
                        balls[i].image,
                        1.0f, 1.0f, 0.0f, WHITE
                    );
                }
            }

            // 手の描画
            if (mouseDown) {
                Novice::DrawSprite(mousePosX - mouseRadius, mousePosY - mouseRadius,
                    closeHand, 1.0f, 1.0f, 0.0f, WHITE);
            }
            else {
                Novice::DrawSprite(mousePosX - mouseRadius, mousePosY - mouseRadius,
                    openHand, 1.0f, 1.0f, 0.0f, WHITE);
            }

            lives = maxMiss - missCount;
            for (int i = 0; i < lives; i++) {
                Novice::DrawSprite(
                    20 + i * 40,   // X座標（横に並べる）
                    20,            // Y座標（固定）
                    lifeIcon,      // 残機画像
                    1.0f, 1.0f,    // 拡大率
                    0.0f,          // 回転
                    WHITE          // 色
                );
            }

            if (gameTimer >= 0 && gameTimer < 60) {
                Novice::DrawSprite(550, 50, numGH[3], 2.0f, 2.0f, 0.0f, WHITE);
            }
            else if (gameTimer >= 60 && gameTimer < 120) {
                Novice::DrawSprite(550, 50, numGH[2], 2.0f, 2.0f, 0.0f, WHITE);
            }
            else if (gameTimer >= 120 && gameTimer < 180) {
                Novice::DrawSprite(550, 50, numGH[1], 2.0f, 2.0f, 0.0f, WHITE);
            }

            if (score <= hiScore) {
                hiScore = score;
            }

            break;
        case SCORE:
        {
            int temp[4] = { highScores[0], highScores[1], highScores[2], score };

            std::sort(temp, temp + 4, std::greater<int>());

            // 重複を除いて上位3位に反映
            int idx = 0;
            int lastScore = -1;
            for (int i = 0; i < 4 && idx < 3; ++i) {
                if (temp[i] != lastScore) {
                    highScores[idx++] = temp[i];
                    lastScore = temp[i];
                }
            }

            // 空いた順位は0にしておく
            for (; idx < 3; ++idx) {
                highScores[idx] = 0;
            }

            Novice::DrawSprite(0, 0, result, 1.0f, 1.0f, 0.0f, WHITE);

            // 現スコアを描画
            SplitScoreToDigits(score, numArray, 3);
            for (int i = 0; i < 3; ++i) {
                Novice::DrawSprite(450 + i * 85, 360, numGH[numArray[i]], 2.5f, 2.5f, 0.0f, WHITE);
            }

            // 左側にランキング表示
            for (int rank = 0; rank < 3; ++rank) {
                if (highScores[rank] == 0) continue; // 空白なら描画しない
                int digits[3];
                SplitScoreToDigits(highScores[rank], digits, 3);
                for (int i = 0; i < 3; ++i) {
                    Novice::DrawSprite(50 + i * 60, 120 + rank * 100, numGH[digits[i]], 2.0f, 2.0f, 0.0f, WHITE);
                }
            }
        }
        break;
        }

        // --- 描画処理 ------------------------------------------------

        Novice::EndFrame();

        // マウスの前フレーム情報を保持
        prevMouseX = mousePosX;
        prevMouseY = mousePosY;
        prevMouseDown = mouseDown;

        // ESCで終了
        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }

    Novice::Finalize();
    return 0;
}
