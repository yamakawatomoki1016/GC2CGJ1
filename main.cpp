#include <Novice.h>
#include <cstring>

const char kWindowTitle[] = "GJ1";

// マップサイズ
const int MAP_WIDTH = 40;
const int MAP_HEIGHT = 23;
const int CHIP_SIZE = 32;

// マップチップの種類
enum TileType {
    TILE_EMPTY = 0,
    TILE_RED,
    TILE_BLUE,
};

// チュートリアル用マップ
// 0=空白, 1=赤ブロック, 2=青ブロック
int map[MAP_HEIGHT][MAP_WIDTH] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // 赤い床
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}, // 青い床
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

// プレイヤー構造体
struct Player {
    float x, y;
    float vx, vy;
    int size;
    bool isRed; // true=赤, false=青
} player;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Novice::Initialize(kWindowTitle, 1280, 736);

    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    // プレイヤー初期化
    player.x = 100;
    player.y = 100;
    player.vx = 0;
    player.vy = 0;
    player.size = 28;
    player.isRed = true;

    const float gravity = 0.5f;
    const float jumpPower = -10.0f;
    const float moveSpeed = 4.0f;

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();

        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        ///
        /// ↓更新処理
        ///

        // 左右移動
        player.vx = 0;
        if (keys[DIK_A]) {
            player.vx = -moveSpeed;
        }
        if (keys[DIK_D]) {
            player.vx = moveSpeed;
        }

        // 色切り替え
        if (!preKeys[DIK_RETURN] && keys[DIK_RETURN]) {
            player.isRed = !player.isRed;
        }

        // 重力
        player.vy += gravity;

        // ジャンプ（足場に乗っているときだけ）
        if (!preKeys[DIK_SPACE] && keys[DIK_SPACE]) {
            // 足場チェック
            int footX = static_cast<int>((player.x + player.size / 2) / CHIP_SIZE);
            int footY = static_cast<int>((player.y + player.size) / CHIP_SIZE);
            if (footY < MAP_HEIGHT) {
                int tile = map[footY][footX];
                if ((tile == TILE_RED && player.isRed) || (tile == TILE_BLUE && !player.isRed)) {
                    player.vy = jumpPower;
                }
            }
        }

        // 位置更新
        player.x += player.vx;
        player.y += player.vy;

        // 足場との当たり判定
        int footX = static_cast<int>((player.x + player.size / 2) / CHIP_SIZE);
        int footY = static_cast<int>((player.y + player.size) / CHIP_SIZE);
        if (footX >= 0 && footX < MAP_WIDTH && footY >= 0 && footY < MAP_HEIGHT) {
            int tile = map[footY][footX];
            if ((tile == TILE_RED && player.isRed) || (tile == TILE_BLUE && !player.isRed)) {
                // 床の上に乗る
                player.y = footY * (float)CHIP_SIZE - player.size;
                player.vy = 0;
            }
        }

        ///
        /// ↓描画処理
        ///

        // マップを描画
        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                int tile = map[y][x];
                if (tile == TILE_EMPTY) continue;

                unsigned int color = 0xFFFFFFFF;
                if (tile == TILE_RED) {
                    color = 0xFF0000FF; // 赤
                }
                else if (tile == TILE_BLUE) {
                    color = 0x0000FFFF; // 青
                }

                int px = x * CHIP_SIZE;
                int py = y * CHIP_SIZE;
                Novice::DrawBox(px, py, CHIP_SIZE, CHIP_SIZE, 0.0f, color, kFillModeSolid);
            }
        }

        // プレイヤーを描画
        unsigned int pColor = player.isRed ? 0xFF0000FF : 0x0000FFFF;
        Novice::DrawBox((int)player.x, (int)player.y, player.size, player.size, 0.0f, pColor, kFillModeSolid);

        ///
        /// ↑描画処理ここまで
        ///

        Novice::EndFrame();

        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }

    Novice::Finalize();
    return 0;
}
