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

struct Ball {
    float x, y;
    float radius;
    int color;  
    int image;   // ← 追加（描画用）
    float speed;
    float vx, vy;
    bool isFixed;
    bool beingHeld;
    bool touched;
    bool exploded;
    bool active;
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
        balls[i].speed = float(3 + rand() % 3);

        if (rand() % 2 == 0) {
            balls[i].color = WHITE;          // ← 判定用
            balls[i].image = whiteBallGH;    // ← 描画用
        }
        else {
            balls[i].color = BLACK;          // ← 判定用
            balls[i].image = blackBallGH;    // ← 描画用
        }

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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, SCREEN_W, SCREEN_H);

    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    // 手の画像（あるなら）
    int openHand = Novice::LoadTexture("./Resources./openHand.png");
    int closeHand = Novice::LoadTexture("./Resources./closeHand.png");
    whiteBallGH = Novice::LoadTexture("./Resources/whiteBomb1.png");//白
    blackBallGH = Novice::LoadTexture("./Resources/blackBomb1.png");//黒
    int lifeIcon = Novice::LoadTexture("./Resources/blackBomb1.png");//残機

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

    // ボール初期化
    for (int i = 0; i < ballCount; i++) {
        balls[i].x = SCREEN_W / 2.0f;
        balls[i].y = -i * 150.0f;
        balls[i].radius = 30.0f;
        balls[i].speed = float(3 + rand() % 3);

        if (rand() % 2 == 0) {
            balls[i].color = WHITE;          // ← 判定用
            balls[i].image = whiteBallGH;    // ← 描画用
        }
        else {
            balls[i].color = BLACK;          // ← 判定用
            balls[i].image = blackBallGH;    // ← 描画用
        }

        balls[i].vx = 0.0f;
        balls[i].vy = 0.0f;
        balls[i].isFixed = false;
        balls[i].beingHeld = false;
        balls[i].touched = false;
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
    //int score = 0;
    int missCount = 0;   // ミスした回数
    const int maxMiss = 3;
    // ゲーム開始時のカウントダウン用
    int gameTimer = 0;
    bool gameStart = false;

    int lives;

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
                                    scene = SCORE; // 3回ミスで終了
                                }
                            }
                            else {
                                // --- 仕分け判定を追加 ---
                                bool correct = false;
                                if (balls[i].color == WHITE && balls[i].x > SCREEN_W / 2) {
                                    correct = true; // 白は右
                                }
                                if (balls[i].color == BLACK && balls[i].x < SCREEN_W / 2) {
                                    correct = true; // 黒は左
                                }

                                if (!correct) {
                                    // ミス → カウントを増やす
                                    missCount++;
                                    if (missCount >= maxMiss) {
                                        // 3回ミスで終了
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
            break;
        case GAME:
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
                        2.0f, 2.0f, 0.0f, WHITE
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

            break;
        case SCORE:
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
