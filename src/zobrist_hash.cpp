#include "zobrist_hash.hpp"
#include <random>
#include <array>
#include "chess.hpp"
#include <iostream>

// Define constants
const int BOARD_SIZE = 64;       // Number of squares on a chessboard
const int PIECE_TYPES = 12;      // 6 pieces × 2 colors
const int CASTLING_RIGHTS = 16;  // Possible castling states
const int EN_PASSANT_FILES = 8;  // Files for en passant targets

using namespace chess;

// Zobrist table
std::array<std::array<uint64_t, BOARD_SIZE>, PIECE_TYPES> piece_hash;
std::array<uint64_t, CASTLING_RIGHTS> castling_hash;
std::array<uint64_t, EN_PASSANT_FILES> en_passant_hash;
uint64_t side_to_move_hash;

// Random number generator
std::mt19937_64 rng(12345); // Seed for reproducibility
std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

void initialize_zobrist() {
    for (int piece = 0; piece < PIECE_TYPES; ++piece) {
        for (int square = 0; square < BOARD_SIZE; ++square) {
            piece_hash[piece][square] = dist(rng);
        }
    }
    for (int i = 0; i < CASTLING_RIGHTS; ++i) {
        castling_hash[i] = dist(rng);
    }
    for (int i = 0; i < EN_PASSANT_FILES; ++i) {
        en_passant_hash[i] = dist(rng);
    }
    side_to_move_hash = dist(rng);
}

uint64_t compute_zobrist_hash(const Board& board) {
    uint64_t hash = 0;

    // Add pieces
    for (int square = 0; square < BOARD_SIZE; ++square) {

        auto piece = board.at(chess::Square((uint8_t)square));
		if (piece != chess::Piece::NONE) {
            int piece_val;
            if (piece.color() == chess::Color::WHITE){
                piece_val = piece.type();
            } else {
                piece_val = piece.type() + 6;
            }
            hash ^= piece_hash[piece_val][square];
        }

    }

    // TODO 
    // Implement the actual hashing with castling and en passant info

    // Add castling rights
    // int castling = board.getCastlingRights(); // Encode castling state (e.g., 0-15)
    // hash ^= castling_hash[castling];

    // Add en passant target
    // int ep_file = board.getEnPassantFile(); // File index or -1 if no target
    // if (ep_file != -1) {
    //     hash ^= en_passant_hash[ep_file];
    // }

    // Add side to move
    if (board.sideToMove() == Color::WHITE) {
        hash ^= side_to_move_hash;
    }


    return hash;
}

void update_zobrist_hash(uint64_t& hash, int piece, int from_square, int to_square, int captured_piece, int castling, int new_castling, int ep_file, int new_ep_file, bool side_to_move) {
    // Remove the piece from the starting square
    hash ^= piece_hash[piece][from_square];

    // Add the piece to the destination square
    hash ^= piece_hash[piece][to_square];

    // Remove the captured piece if any
    if (captured_piece != -1) {
        hash ^= piece_hash[captured_piece][to_square];
    }

    // TODO 
    // Implement the actual hashing with castling and en passant info
    // Update castling rights
    // hash ^= castling_hash[castling];
    // hash ^= castling_hash[new_castling];

    // // Update en passant file
    // if (ep_file != -1) {
    //     hash ^= en_passant_hash[ep_file];
    // }
    // if (new_ep_file != -1) {
    //     hash ^= en_passant_hash[new_ep_file];
    // }

    // Flip side to move
    hash ^= side_to_move_hash;
}
