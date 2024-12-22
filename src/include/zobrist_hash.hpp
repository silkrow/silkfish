#pragma once
#include "chess.hpp"
void initialize_zobrist();
uint64_t compute_zobrist_hash(const chess::Board& board);
void update_zobrist_hash(uint64_t& hash, int piece, int from_square, int to_square, int captured_piece, int castling, int new_castling, int ep_file, int new_ep_file, bool side_to_move);