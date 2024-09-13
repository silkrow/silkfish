#pragma once
#include <map>
#include <semaphore>
#include <thread>

const int MAX_SCORE = 100000;
const int W_WIN_THRE = MAX_SCORE - 50;
const int B_WIN_THRE = -W_WIN_THRE;

const int BOARD_SIZE = 64;

const int CASTLE = 10;

const int DEFAULT_DEPTH_MM = 6;
const int DEFAULT_DEPTH_Q = 3;

const int PIECE_VAL[6] = {1, 3, 3, 5, 9, 0};
const int RAND_MOVE_THRE = 10;
const int MAX_THREAD = std::thread::hardware_concurrency();

extern int quiescence_depth;
extern int mm_depth;
extern float time_limit;
extern bool debug_mode; // Not being used.
extern int evals[1000];


const int PST[6][BOARD_SIZE] = {{// PAWN
    0,  0,  0,  0,  0,  0,  0,  0,
    150, 150, 150, 150, 150, 150, 150, 150,
    110, 110, 120, 130, 130, 120, 110, 110,
    105,  105, 110, 125, 125, 110, 105, 105,
    100,  100,  100, 120, 120,  100, 100,  100,
    105, 95, 90, 100, 100, 90, 95, 105,
    105, 110, 110, 80, 80, 110, 110, 105,
    0,  0,  0,  0,  0,  0,  0,  0
}, { // KNIGHT
    290, 300, 300, 300, 300, 300, 300, 290,
    300, 305, 305, 305, 305, 305, 305, 300,
    300, 305, 325, 325, 325, 325, 305, 300,
    300, 305, 325, 325, 325, 325, 305, 300,
    300, 305, 325, 325, 325, 325, 305, 300,
    300, 305, 320, 325, 325, 325, 305, 300,
    300, 305, 305, 305, 305, 305, 305, 300,
    290, 310, 300, 300, 300, 300, 310, 290
}, {  // BISHOP
    290, 300, 300, 300, 300, 300, 300, 290,
    300, 305, 305, 305, 305, 305, 305, 300,
    300, 305, 325, 325, 325, 325, 305, 300,
    300, 305, 325, 325, 325, 325, 305, 300,
    300, 305, 325, 325, 325, 325, 305, 300,
    300, 305, 320, 325, 325, 325, 305, 300,
    300, 305, 305, 305, 305, 305, 305, 300,
    290, 310, 300, 300, 300, 300, 310, 290
}, { // ROOK
    500, 500, 500, 500, 500, 500, 500, 500,
    520, 520, 520, 520, 520, 520, 520, 520,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 500, 500, 500, 500, 500, 500,
    500, 500, 505, 510, 510, 505, 500, 500
}, { //QUEEN
    900, 900, 900, 900, 900, 900, 900, 900,
    900, 900, 900, 900, 900, 900, 900, 900,
    900, 900, 900, 900, 900, 900, 900, 900,
    900, 900, 900, 900, 900, 900, 900, 900,
    900, 900, 900, 900, 900, 900, 900, 900,
    900, 900, 900, 900, 900, 900, 900, 900,
    900, 900, 900, 900, 900, 900, 900, 900,
    900, 900, 900, 900, 900, 900, 900, 900
}, { //KING
    -10, -10, 0, 0, 0, 0, -10, -10,
    -10, -10, 0, 0, 0, 0, -10, -10,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, -10, -10, 0, 0, 0,
    10, 10, 0, -10, 0, -10, 10, 10,
}};

extern std::map<std::pair<int, int>, int> capture_score;