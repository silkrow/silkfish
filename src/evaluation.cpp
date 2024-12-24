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
	int evaluation;

	if (board.isGameOver().second == chess::GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == chess::GameResultReason::CHECKMATE) {
		return board.sideToMove() == chess::Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

    // Define endgame: 
    // At least one side has less than 13 points of materials
	int mg_white = 0, mg_black = 0, eg_white = 0, eg_black = 0;
	int white_points = 0, black_points = 0;

	uint64_t mask = board.all().getBits();

	for (int i = 0; i < BOARD_SIZE; i++) {
		if (mask & 1ULL << i) {
			chess::Piece piece = board.at<chess::Piece>(chess::Square((uint8_t)i));
			if (piece.color() == chess::Color::WHITE) {
				mg_white += PESTO_POSITION[0][piece.type()][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[0][piece.type()];
				eg_white += PESTO_POSITION[1][piece.type()][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)] + PESTO_VALUE[1][piece.type()];
				white_points += PIECE_VAL[piece.type()];
			} else {
				mg_black += (PESTO_POSITION[0][piece.type()][i] + PESTO_VALUE[0][piece.type()]);
				eg_black += (PESTO_POSITION[1][piece.type()][i] + PESTO_VALUE[1][piece.type()]);
				black_points += PIECE_VAL[piece.type()];
			}
		}
	}

	evaluation = (white_points <= 13 || black_points <= 13)?eg_white - eg_black : mg_white - mg_black;

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