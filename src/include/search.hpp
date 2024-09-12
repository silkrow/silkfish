#pragma once
#include "chess.hpp"

int quiescence_search(int q_depth, int alpha, int beta, chess::Color color, chess::Board board);
int minimax(int mm_depth, int alpha, int beta, chess::Color color, chess::Board board);
