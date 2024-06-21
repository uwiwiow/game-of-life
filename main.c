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

typedef struct Cell {
    bool alive;
    short neighbors;
} Cell;

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
        if (errno != 0 || *p != '\0' || TILE > HEIGHT || TILE > WIDTH || TILE < 1)
            handle_error("TILE not a number or too long\n")

    }


    const int W_TILES = WIDTH / TILE;
    const int H_TILES = HEIGHT / TILE;

    SetWindowState(FLAG_WINDOW_RESIZABLE);
    InitWindow(WIDTH, HEIGHT, "Game of Life");
    SetTargetFPS(60);

    // camera
    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    Vector2 relPos;

    // declare and initialize board
    Cell board[W_TILES][H_TILES];
    for (int i = 0; i < W_TILES; i++) {
        for (int j = 0; j < H_TILES; j++) {
            board[i][j].alive = false;
            board[i][j].neighbors = 0;
        }
    }
    // declare secondary board
    Cell buffBoard[W_TILES][H_TILES];

    Cell cleanBoard[W_TILES][H_TILES];
    memcpy(cleanBoard, board, sizeof board);



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
                board[dx][dy].alive = true;
            }
                // remove cell
            else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                board[dx][dy].alive = false;
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
            memcpy(buffBoard, cleanBoard, sizeof board);
            memcpy(buffBoard, board, sizeof board);
            for (int i = 0; i < W_TILES; i++) {
                for (int j = 0; j < H_TILES; j++) {

                    if (board[i][j].alive) {
                        total = 0;
                        for (int d = 0; d < 8; d++) {
                            int newX = i + directions[d][0];
                            int newY = j + directions[d][1];
                            if (newX >= 0 && newX < W_TILES && newY >= 0 && newY < H_TILES) {
                                if (board[newX][newY].alive) {
                                    total++;
                                } else {
                                    buffBoard[newX][newY].neighbors++;
                                }
                            }
                        }
                    }
                }
            }
            for (int i = 0; i < W_TILES; i++) {
                for (int j = 0; j < H_TILES; j++) {
                    if (!buffBoard[i][j].alive && buffBoard[i][j].neighbors == 3){
                        buffBoard[i][j].alive = true;
                        buffBoard[i][j].neighbors = 0;
                    }
                }
            }
            memcpy(board, buffBoard, sizeof board);
            memcpy(buffBoard, cleanBoard, sizeof board);
        }

        // reset
        if (IsKeyPressed(KEY_R)) {
            for (int i = 0; i < W_TILES; i++) {
                for (int j = 0; j < H_TILES; j++) {
                    board[i][j].alive = false;
                    board[i][j].neighbors = 0;
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
                if (board[i][j].alive) {
                    DrawRectangle(dx, dy, TILE, TILE, BLACK);
                } else {
                    DrawRectangle(dx, dy, TILE, TILE, WHITE);

                }
            }
        }

        dx = (int) relPos.x - ((int) relPos.x % TILE);
        dy = (int) relPos.y - ((int) relPos.y % TILE);

        DrawRectangle(dx, dy, TILE, TILE, GRAY);

        EndMode2D();
        char Text[32];

        sprintf(Text, "Zoom: %.2f", camera.zoom);
        DrawText(Text , 10, 10, 24, YELLOW);

        dx = (int) relPos.x / TILE;
        dy = (int) relPos.y / TILE;

        sprintf(Text, "Neighbors: %d", board[dx][dy].neighbors);
        DrawText(Text , 10, 30, 24, YELLOW);

        sprintf(Text, "Mouse rel: %f, %f", relPos.x, relPos.y);
        DrawText(Text , 10, 50, 24, YELLOW);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
