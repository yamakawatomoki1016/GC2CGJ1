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

int map[MAP_HEIGHT][MAP_WIDTH] = {
    // 0=空白, 1=赤ブロック, 2=青ブロック
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,0,0,0,2,0,0,0,1,0,0,0,2,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {2,2,2,2,2,2,0,0,0,1,0,0,0,2,0,0,0,1,0,0,0,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
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

// ブロックが「プレイヤーと同じ色かどうか」判定
bool IsSolid(int tile, bool isRed) {
    return (tile == TILE_RED && isRed) || (tile == TILE_BLUE && !isRed);
}

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

        ////////////// ↓更新処理 //////////////

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

        // --- 横方向の当たり判定 ---
        float nextX = player.x + player.vx;
        int left = (int)(nextX / CHIP_SIZE);
        int right = (int)((nextX + player.size) / CHIP_SIZE);
        int top = (int)(player.y / CHIP_SIZE);
        int bottom = (int)((player.y + player.size - 1) / CHIP_SIZE);

        bool hitWall = false;
        if (player.vx > 0) { // 右移動
            for (int ty = top; ty <= bottom; ty++) {
                if (right < MAP_WIDTH && IsSolid(map[ty][right], player.isRed)) {
                    player.x = right * CHIP_SIZE - player.size - 0.01f;
                    player.vx = 0;
                    hitWall = true;
                    break;
                }
            }
        }
        else if (player.vx < 0) { // 左移動
            for (int ty = top; ty <= bottom; ty++) {
                if (left >= 0 && IsSolid(map[ty][left], player.isRed)) {
                    player.x = (left + 1) * CHIP_SIZE + 0.01f;
                    player.vx = 0;
                    hitWall = true;
                    break;
                }
            }
        }
        if (!hitWall) {
            player.x += player.vx;
        }

        // --- 縦方向の当たり判定（床・天井） ---
        float nextY = player.y + player.vy;
        left = (int)(player.x / CHIP_SIZE);
        right = (int)((player.x + player.size - 1) / CHIP_SIZE);
        int footY = (int)((nextY + player.size) / CHIP_SIZE);
        int headY = (int)(nextY / CHIP_SIZE);

        bool onGround = false;
        if (player.vy >= 0) { // 下方向
            for (int tx = left; tx <= right; tx++) {
                if (footY < MAP_HEIGHT && IsSolid(map[footY][tx], player.isRed)) {
                    player.y = static_cast<float>(footY) * CHIP_SIZE - player.size;
                    player.vy = 0;
                    onGround = true;
                    break;
                }
            }
        }
        else { // 上方向
            for (int tx = left; tx <= right; tx++) {
                if (headY >= 0 && IsSolid(map[headY][tx], player.isRed)) {
                    player.y = (static_cast<float>(headY) + 1.0f) * CHIP_SIZE;
                    player.vy = 0;
                    break;
                }
            }
        }
        if (!onGround) {
            player.y = nextY;
        }

        // ジャンプ
        if (!preKeys[DIK_SPACE] && keys[DIK_SPACE] && onGround) {
            player.vy = jumpPower;
        }
        ////////////// 更新処理 //////////////

        ////////////// ↓描画処理 //////////////

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

        ////////////// 描画処理 //////////////

        Novice::EndFrame();

        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }

    Novice::Finalize();
    return 0;
}
