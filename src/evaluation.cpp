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
	int white_points = 0, black_points = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
		auto piece = board.at(chess::Square((uint8_t)i));
		if (piece != chess::Piece::NONE) {
			if (piece.color() == chess::Color::WHITE) {
				white_points += PIECE_VAL[piece.type()];
			} else {
				black_points += PIECE_VAL[piece.type()];
			}
		} 
	}    

    int offset = (white_points <= 13 && black_points <= 13)? 1:0; // 1 for eg, 0 for mg

	for (int i = 0; i < BOARD_SIZE; i++) {
		auto piece = board.at(chess::Square((uint8_t)i));
		if (piece != chess::Piece::NONE) {
			if (piece.color() == chess::Color::WHITE) {
				evaluation += PESTO_POSITION[offset][piece.type()][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[offset][piece.type()];
			} else {
				evaluation -= (PESTO_POSITION[offset][piece.type()][i] + PESTO_VALUE[offset][piece.type()]);
			}
		} 
	}

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
		board.makeMove(move);
		if (is_capture_move(board, move)) {
			int attacker_val = PIECE_VAL[(int)((board.at<chess::Piece>(move.from())).type())];
			int victim_val = PIECE_VAL[(int)((board.at<chess::Piece>(move.to())).type())];
			if (attacker_val < victim_val) {
				board.unmakeMove(move);
				return false;
			}
		}
		board.unmakeMove(move);
	}

	return true;
}