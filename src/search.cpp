#include <chrono>
#include "chess.hpp"
#include "search.hpp"
#include "evaluation.hpp"
#include "constants.hpp"

using namespace chess;
using namespace std;

int quiescence_search (int q_depth, int alpha, int beta, Color color, Board board) {
	// Force quit if time is up
	// if (time_limit > 0) {
	// 	auto end = std::chrono::high_resolution_clock::now();
	// 	chrono::duration<double> duration = end - start;
	// 	if (duration.count() > time_limit) {
	// 		return color == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	// 	}
	// }

	if (q_depth == 0 || appear_quiet(board)) return evaluation(board);

	Movelist moves;
	movegen::legalmoves(moves, board);

	if (board.isGameOver().second == GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

	if (color == Color::WHITE) {
		int max_eval = -MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = quiescence_search(q_depth, alpha, beta, 1 - color, board);
			board.unmakeMove(move);

			max_eval = eval > max_eval ? eval:max_eval;
			alpha = alpha > max_eval ? alpha:max_eval;

			if (beta <= alpha) break;
		}

		if (max_eval < B_WIN_THRE) return max_eval + 1;
		else if (max_eval > W_WIN_THRE) return max_eval - 1;
		return max_eval;
	} else {
		int min_eval = MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = quiescence_search(q_depth, alpha, beta, 1 - color, board);
			board.unmakeMove(move);

			min_eval = eval < min_eval ? eval:min_eval;
			beta = beta < min_eval ? beta:min_eval;

			if (beta <= alpha) break;
		}

		if (min_eval < B_WIN_THRE) return min_eval + 1;
		else if (min_eval > W_WIN_THRE) return min_eval - 1;
		return min_eval;
	}
}

int minimax (int mm_depth, int alpha, int beta, Color color, Board board) {
	// Force quit if time is up
	// if (time_limit > 0) {
	// 	auto end = std::chrono::high_resolution_clock::now();
	// 	chrono::duration<double> duration = end - start;
	// 	if (duration.count() > time_limit) {
	// 		return color == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	// 	}
	// }

	Movelist moves;
	movegen::legalmoves(moves, board);
	sort(moves.begin(), moves.end(), [board] (Move a, Move b) {
	int a_val = 0, b_val = 0;

	if (a == Move::CASTLING) a_val = CASTLE; // Castle is encoded as king capturing rook in the library!
	else if (board.isCapture(a)) {
		int af = (int)((board.at<Piece>(a.from())).type());
		int at = (int)((board.at<Piece>(a.to())).type());
		
		std::pair<int, int> key = std::make_pair(af, at % 6);
		a_val = capture_score[key];
	}

	if (b == Move::CASTLING) b_val = CASTLE;
	else if (board.isCapture(b)) {
		int bf = (int)((board.at<Piece>(b.from())).type());
		int bt = (int)((board.at<Piece>(b.to())).type());
		
		std::pair<int, int> key = std::make_pair(bf, bt % 6);
		b_val = capture_score[key];
	}

    return a_val > b_val;
	});

	if (mm_depth == 0) {
		if (appear_quiet(board)) {
			return evaluation(board);
		} else {
			if (color == Color::WHITE) {
				int max_eval = -MAX_SCORE;
				for (int i = 0; i < moves.size(); i++) {
					const auto move = moves[i];
					board.makeMove(move);
					int eval = quiescence_search(DEFAULT_DEPTH_Q, alpha, beta, 1 - color, board);
					board.unmakeMove(move);

					max_eval = eval > max_eval ? eval:max_eval;
					alpha = alpha > max_eval ? alpha:max_eval;

					if (beta <= alpha) break;
				}
				if (max_eval < B_WIN_THRE) return max_eval + 1;
				else if (max_eval > W_WIN_THRE) return max_eval - 1;
				return max_eval;
			} else {
				int min_eval = MAX_SCORE;
				for (int i = 0; i < moves.size(); i++) {
					const auto move = moves[i];
					board.makeMove(move);
					int eval = quiescence_search(quiescence_depth, alpha, beta, 1 - color, board);
					board.unmakeMove(move);

					min_eval = eval < min_eval ? eval:min_eval;
					beta = beta < min_eval ? beta:min_eval;

					if (beta <= alpha) break;
				}
				if (min_eval < B_WIN_THRE) return min_eval + 1;
				else if (min_eval > W_WIN_THRE) return min_eval - 1;
				return min_eval;
			}
		}
	}

	if (board.isGameOver().second == GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

	if (color == Color::WHITE) {
		int max_eval = -MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, board);
			board.unmakeMove(move);

			max_eval = eval > max_eval ? eval:max_eval;
			alpha = alpha > max_eval ? alpha:max_eval;

			if (beta <= alpha) break;
		}

		if (max_eval < B_WIN_THRE) return max_eval + 1;
		else if (max_eval > W_WIN_THRE) return max_eval - 1;
		return max_eval;
	} else {
		int min_eval = MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, board);
			board.unmakeMove(move);

			min_eval = eval < min_eval ? eval:min_eval;
			beta = beta < min_eval ? beta:min_eval;

			if (beta <= alpha) break;
		}

		if (min_eval < B_WIN_THRE) return min_eval + 1;
		else if (min_eval > W_WIN_THRE) return min_eval - 1;
		return min_eval;
	}
}