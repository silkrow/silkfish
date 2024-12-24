#include "evaluation.hpp"
#include "constants.hpp"

 std::map<std::pair<int, int>, int> capture_score = {
    {{5, 4}, 50},
    {{4, 4}, 51},
    {{3, 4}, 52},
    {{2, 4}, 53},
    {{1, 4}, 54},
    {{0, 4}, 55},

    {{5, 3}, 40},
    {{4, 3}, 41},
    {{3, 3}, 42},
    {{2, 3}, 43},
    {{1, 3}, 44},
    {{0, 3}, 45},

    {{5, 2}, 30},
    {{4, 2}, 31},
    {{3, 2}, 32},
    {{2, 2}, 33},
    {{1, 2}, 34},
    {{0, 2}, 35},

    {{5, 1}, 20},
    {{4, 1}, 21},
    {{3, 1}, 22},
    {{2, 1}, 23},
    {{1, 1}, 24},
    {{0, 1}, 25},

    {{5, 0}, 10},
    {{4, 0}, 11},
    {{3, 0}, 12},
    {{2, 0}, 13},
    {{1, 0}, 14},
    {{0, 0}, 15}
};

int evaluation(chess::Board& board) {
	int evaluation = 0;

	if (board.isGameOver().second == chess::GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == chess::GameResultReason::CHECKMATE) {
		return board.sideToMove() == chess::Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

    // Define endgame: 
    // Botj sides have less than 13 points of materials
	// int white_points = 0, black_points = 0;
    // for (int i = 0; i < BOARD_SIZE; i++) {
	// 	auto piece = board.at(chess::Square((uint8_t)i));
	// 	if (piece != chess::Piece::NONE) {
	// 		if (piece.color() == chess::Color::WHITE) {
	// 			white_points += PIECE_VAL[piece.type()];
	// 		} else {
	// 			black_points += PIECE_VAL[piece.type()];
	// 		}
	// 	} 
	// }    

    // int offset = (white_points <= 13 && black_points <= 13)? 1:0; // 1 for eg, 0 for mg
	// int offset = 0;

	// std::cout << board.pieces(chess::PieceType::ROOK, chess::Color::WHITE) << std::endl;
	// std::cout << board.us(chess::Color::BLACK) << std::endl;

	// for (int i = 0; i < BOARD_SIZE; i++) {
	// 	chess::PieceType piecetype = board.at<chess::PieceType>(chess::Square((uint8_t)i));
	// 	if (piecetype != chess::PieceType::NONE) {
	// 		if (board.at(chess::Square((uint8_t)i)).color() == chess::Color::WHITE) {
	// 			evaluation += PESTO_POSITION[offset][piecetype][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[offset][piecetype];
	// 		} else {
	// 			evaluation -= (PESTO_POSITION[offset][piecetype][i] + PESTO_VALUE[offset][piecetype]);
	// 		}
	// 	} 
	// }


	int mg_white = 0, mg_black = 0, eg_white = 0, eg_black = 0;
	uint64_t mask;
	// PAWN
	mask = board.pieces(chess::PieceType::PAWN, chess::Color::WHITE).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_white += PESTO_POSITION[0][0][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[0][0];
			eg_white += PESTO_POSITION[1][0][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[1][0];
        }
		mask = mask >> 1;
    }
	// KNIGHT
	mask = board.pieces(chess::PieceType::KNIGHT, chess::Color::WHITE).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_white += PESTO_POSITION[0][1][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[0][1];
			eg_white += PESTO_POSITION[1][1][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[1][1];
        }
		mask = mask >> 1;
    }
	// BISHOP
	mask = board.pieces(chess::PieceType::BISHOP, chess::Color::WHITE).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_white += PESTO_POSITION[0][2][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[0][2];
			eg_white += PESTO_POSITION[1][2][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[1][2];
        }
		mask = mask >> 1;
    }
	// ROOK
	mask = board.pieces(chess::PieceType::ROOK, chess::Color::WHITE).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_white += PESTO_POSITION[0][3][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[0][3];
			eg_white += PESTO_POSITION[1][3][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[1][3];
        }
		mask = mask >> 1;
    }
	// QUEEN
	mask = board.pieces(chess::PieceType::QUEEN, chess::Color::WHITE).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_white += PESTO_POSITION[0][4][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[0][4];
			eg_white += PESTO_POSITION[1][4][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[1][4];
        }
		mask = mask >> 1;
    }
	// KING
	mask = board.pieces(chess::PieceType::KING, chess::Color::WHITE).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_white += PESTO_POSITION[0][5][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[0][5];
			eg_white += PESTO_POSITION[1][5][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[1][5];
        }
		mask = mask >> 1;
    }

	// PAWN
	mask = board.pieces(chess::PieceType::PAWN, chess::Color::BLACK).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_black += PESTO_POSITION[0][0][i] + PESTO_VALUE[0][0];
			eg_black += PESTO_POSITION[1][0][i] + PESTO_VALUE[1][0];
        }
		mask = mask >> 1;
    }
	// KNIGHT
	mask = board.pieces(chess::PieceType::KNIGHT, chess::Color::BLACK).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_black += PESTO_POSITION[0][1][i] + PESTO_VALUE[0][1];
			eg_black += PESTO_POSITION[1][1][i] + PESTO_VALUE[1][1];
        }
		mask = mask >> 1;
    }
	// BISHOP
	mask = board.pieces(chess::PieceType::BISHOP, chess::Color::BLACK).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_black += PESTO_POSITION[0][2][i] + PESTO_VALUE[0][2];
			eg_black += PESTO_POSITION[1][2][i] + PESTO_VALUE[1][2];
        }
		mask = mask >> 1;
    }
	// ROOK
	mask = board.pieces(chess::PieceType::ROOK, chess::Color::BLACK).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_black += PESTO_POSITION[0][3][i] + PESTO_VALUE[0][3];
			eg_black += PESTO_POSITION[1][3][i] + PESTO_VALUE[1][3];
        }
		mask = mask >> 1;
    }
	// QUEEN
	mask = board.pieces(chess::PieceType::QUEEN, chess::Color::BLACK).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_black += PESTO_POSITION[0][4][i] + PESTO_VALUE[0][4];
			eg_black += PESTO_POSITION[1][4][i] + PESTO_VALUE[1][4];
        }
		mask = mask >> 1;
    }
	// KING
	mask = board.pieces(chess::PieceType::KING, chess::Color::BLACK).getBits();
	for (int i = 0; i < BOARD_SIZE; ++i) {
		if (mask == 0) break;
        if (mask & 1ULL) { 
            mg_black += PESTO_POSITION[0][5][i] + PESTO_VALUE[0][5];
			eg_black += PESTO_POSITION[1][5][i] + PESTO_VALUE[1][5];
        }
		mask = mask >> 1;
    }

	evaluation = mg_white - mg_black;
	return evaluation;
}

bool is_capture_move(const chess::Board& board, const chess::Move& move) {
    const chess::Piece& from_piece = board.at<chess::Piece>(move.from());
    const chess::Piece& to_piece = board.at<chess::Piece>(move.to());
    if (from_piece.type() == chess::PieceType::NONE) {
        return false;
    }
    if (to_piece.type() != chess::PieceType::NONE && to_piece.color() != from_piece.color()) {
        return true;
    }
    return false;
}

bool appear_quiet(chess::Board board) {
	if (board.inCheck())
		return false;

	chess::Movelist moves;
	chess::movegen::legalmoves(moves, board);
	for (int i = 0; i < moves.size(); i++) {
		const auto move = moves[i];
		if (is_capture_move(board, move)) {
			board.makeMove(move);
			int attacker_val = PIECE_VAL[(int)((board.at<chess::Piece>(move.from())).type())];
			int victim_val = PIECE_VAL[(int)((board.at<chess::Piece>(move.to())).type())];
			if (attacker_val <= victim_val) {
				board.unmakeMove(move);
				return false;
			}
			board.unmakeMove(move);
		}
	}

	return true;
}