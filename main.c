#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

#define handle_error(msg) \
    do { \
        fprintf( stderr,msg); \
        exit(-1); \
    } while (0);

int main(int argc, char **argv) {
    SetTraceLogLevel(LOG_WARNING);

    int WIDTH = 800;
    int HEIGHT = 800;
    int TILE = 4;

    if (argc >= 4) {
        char *p;
        errno = 0;

        WIDTH = (int) strtol(argv[1], &p, 10);
        if (errno != 0 || *p != '\0' || WIDTH > INT_MAX || WIDTH < INT_MIN)
            handle_error("WIDTH not a number or too long\n")

        HEIGHT = (int) strtol(argv[2], &p, 10);
        if (errno != 0 || *p != '\0' || HEIGHT > INT_MAX || HEIGHT < INT_MIN)
            handle_error("HEIGHT not a number or too long\n")

        TILE = (int) strtol(argv[3], &p, 10);
        if (errno != 0 || *p != '\0' || TILE > INT_MAX || TILE < INT_MIN)
            handle_error("TILE not a number or too long\n")

    }


    const int W_TILES = WIDTH / TILE;
    const int H_TILES = HEIGHT / TILE;

    InitWindow(WIDTH, HEIGHT, "Game of Life");
    SetTargetFPS(60);

    // camera
    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    Vector2 relPos;

    // declare and initialize board
    bool board[W_TILES][H_TILES];
    for (int i = 0; i < W_TILES; i++) {
        for (int j = 0; j < H_TILES; j++) {
            board[i][j] = false;
        }
    }
    // declare secondary board
    bool buffBoard[W_TILES][H_TILES];

    Vector2 mousePos;
    int dx, dy;

    int total;
    int directions[8][2] = {
            {-1, -1}, {-1, 0}, {-1, 1},
            { 0, -1},          { 0, 1},
            { 1, -1}, { 1, 0}, { 1, 1}
    };

    int running = false;

    while (!WindowShouldClose()) {

        //----------------------------------------------------------------------------------
        // mouse input
        mousePos = GetMousePosition();
        relPos = GetScreenToWorld2D(mousePos, camera);
        dx = (int) relPos.x / TILE;
        dy = (int) relPos.y / TILE;

        // Translate based on mouse click
        if (IsKeyDown(KEY_SPACE)) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 delta = GetMouseDelta();
                delta = Vector2Scale(delta, -1.0f/camera.zoom);
                camera.target = Vector2Add(camera.target, delta);
            }
        } else {
            // place a cell
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                board[dx][dy] = true;
            }
                // remove cell
            else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                board[dx][dy] = false;
            }
        }

        // Zoom based on mouse wheel
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            // Get the world point that is under the mouse
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            // Set the offset to where the mouse is
            camera.offset = GetMousePosition();

            // Set the target to match, so that the camera maps the world space point
            // under the cursor to the screen space point under the cursor at any zoom
            camera.target = mouseWorldPos;

            // Zoom increment
            float scaleFactor = 1.0f + (0.1f*fabsf(wheel));
            if (wheel < 0) scaleFactor = 1.0f/scaleFactor;
            camera.zoom = Clamp(camera.zoom*scaleFactor, 0.125f, 64.0f);
        }

        //----------------------------------------------------------------------------------
        // key input

        // start/stop running
        if (IsKeyPressed(KEY_W)) {
            running = !running;
        }

        // hold to run
        if (IsKeyDown(KEY_E) || running) {
            memcpy(buffBoard, board, sizeof board);
            for (int i = 0; i < W_TILES; i++) {
                for (int j = 0; j < H_TILES; j++) {
                    total = 0;
                    for (int d = 0; d < 8; d++) {
                        int newX = i + directions[d][0];
                        int newY = j + directions[d][1];
                        if (newX >= 0 && newX < W_TILES && newY >= 0 && newY < H_TILES) {
                            if (board[newX][newY]) {
                                total++;
                            }
                        }
                    }
                    if (total < 2 || total > 3) {
                        buffBoard[i][j] = false;
                    }
                    if (total == 3 && !board[i][j]) {
                        buffBoard[i][j] = true;
                    }

                }
            }
            memcpy(board, buffBoard, sizeof board);
        }

        // reset
        if (IsKeyPressed(KEY_R)) {
            for (int i = 0; i < W_TILES; i++) {
                for (int j = 0; j < H_TILES; j++) {
                    board[i][j] = false;
                }
            }
        }

        //----------------------------------------------------------------------------------
        // draw

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);

        for (int i = 0; i < W_TILES; i++) {
            dx = i * TILE;
            for (int j = 0; j < H_TILES; j++) {
                dy = j * TILE;
                if (board[i][j]) {
                    DrawRectangle(dx, dy, TILE, TILE, BLACK);
                } else {
                    DrawRectangle(dx, dy, TILE, TILE, WHITE);

                }
            }
        }

        EndMode2D();
        char zoomText[32];
        sprintf(zoomText, "Zoom: %.2f", camera.zoom);

        DrawText(zoomText , 10, 10, 24, YELLOW);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
