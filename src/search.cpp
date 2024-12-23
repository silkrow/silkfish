#include "chess.hpp"
#include "search.hpp"
#include "evaluation.hpp"
#include "constants.hpp"
#include <unordered_map>
#include <string>

using namespace chess;
using namespace std;

unordered_map<uint64_t, std::pair<int, std::pair<int, string>>> TTable;

// For debugging and performance testing
long mm_cnt = 0;
long q_cnt = 0;

void sort_moves(chess::Movelist& moves, const chess::Board& board) {
    std::sort(moves.begin(), moves.end(), [&board](const chess::Move& a, const chess::Move& b) {
        int a_val = 0, b_val = 0;

		if (a == Move::CASTLING) a_val = CASTLE;
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
}

void table_insert(uint64_t hash, int depth, std::pair<int, string> pair, Color color) {
    if (TTable.size() >= TTABLE_MAX) {
        TTable.clear();
    }

    auto it = TTable.find(hash);
    if (it != TTable.end()) {
        auto [stored_depth, stored_value] = it -> second;
        if (stored_depth == depth) {
            if ((color == Color::WHITE && stored_value.first >= pair.first) || 
            (color == Color::BLACK && stored_value.first <= pair.first)) return;
        } else if (stored_depth > depth) return;
    }

    TTable[hash] = {depth, pair};
}

std::pair<int, std::string> quiescence_search (int q_depth, int alpha, int beta, Color color, Board board) {

    if (debug_mode) q_cnt++;

	if (q_depth == 0 || appear_quiet(board)) return {evaluation(board), ""};

	Movelist moves;
	movegen::legalmoves(moves, board);

	if (board.isGameOver().second == GameResult::DRAW) {
		return {0, ""};
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return {board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE, ""};
	}

    string best_move_str = "";

	if (color == Color::WHITE) {
		int max_eval = -MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			auto [eval, prev_move_str] = quiescence_search(q_depth-1, alpha, beta, 1 - color, board);
			board.unmakeMove(move);

			if (eval > max_eval) {
                max_eval = eval;
                best_move_str = "~" + uci::moveToSan(board, move) + " " + prev_move_str;  // Get the move as a string
            }
			alpha = alpha > max_eval ? alpha:max_eval;

			if (beta <= alpha) break;
		}

		if (max_eval < B_WIN_THRE) return {max_eval + 1, best_move_str};
		else if (max_eval > W_WIN_THRE) return {max_eval - 1, best_move_str};
		return {max_eval, best_move_str};
	} else {
		int min_eval = MAX_SCORE;
		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			auto [eval, prev_move_str] = quiescence_search(q_depth-1, alpha, beta, 1 - color, board);
			board.unmakeMove(move);

			if (eval < min_eval) {
                min_eval = eval;
                best_move_str = "~" + uci::moveToSan(board, move) + " " + prev_move_str;  // Get the move as a string
            }
			beta = beta < min_eval ? beta:min_eval;

			if (beta <= alpha) break;
		}

		if (min_eval < B_WIN_THRE) return {min_eval + 1, best_move_str};
		else if (min_eval > W_WIN_THRE) return {min_eval - 1, best_move_str};
		return {min_eval, best_move_str};
	}
}

std::pair<int, std::string> minimax (int mm_depth, int alpha, int beta, Color color, Board board, uint64_t board_hash) {
    if (debug_mode) mm_cnt++;

    if (board.isGameOver().second == GameResult::DRAW) {
		return {0, ""};
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return {board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE, ""};
	}

    string hash_move = "";

    // Check in transposition table
    auto it = TTable.find(board_hash);
    if (it != TTable.end()) {
        auto [depth, value] = it -> second;
        size_t spacePos = value.second.find(' ');

    
        hash_move = (spacePos != std::string::npos) ? value.second.substr(0, spacePos) : value.second;

        if (depth > mm_depth) return value;
    }

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    sort_moves(moves, board);

    // Prioritize the hash move
    if (hash_move.size() != 0) {
        for (int i = 0; i < moves.size(); i++) {
            if (hash_move == uci::moveToSan(board, moves[i])) {
                Move swap = moves[0];
                moves[0] = moves[i];
                moves[i] = swap;
                break;
            }
        }
    }

    std::string best_move_str = "";
    std::string prev_move_str = "";

	if (mm_depth == 0) {
		if (appear_quiet(board)) {
            table_insert(board_hash, mm_depth, {evaluation(board), ""}, color);
			return {evaluation(board), ""};
		} else {
			if (color == Color::WHITE) {
				int max_eval = -MAX_SCORE;
				for (int i = 0; i < moves.size(); i++) {
					const auto move = moves[i];
					board.makeMove(move);
					auto [eval, prev_move_str] = quiescence_search(quiescence_depth, alpha, beta, 1 - color, board);
					board.unmakeMove(move);

					if (eval > max_eval) {
                        max_eval = eval;
                        best_move_str = uci::moveToSan(board, move) + " " + prev_move_str;  // Get the move as a string
                    }
					alpha = alpha > max_eval ? alpha:max_eval;

					if (beta <= alpha) break;
				}
				if (max_eval < B_WIN_THRE) {
                    table_insert(board_hash, mm_depth, {max_eval + 1, best_move_str}, color);
                    return {max_eval + 1, best_move_str};
                }
				else if (max_eval > W_WIN_THRE) {
                    table_insert(board_hash, mm_depth, {max_eval - 1, best_move_str}, color);
                    return {max_eval - 1, best_move_str};
                }
                table_insert(board_hash, mm_depth, {max_eval, best_move_str}, color);
				return {max_eval, best_move_str};
			} else {
				int min_eval = MAX_SCORE;
				for (int i = 0; i < moves.size(); i++) {
					const auto move = moves[i];
					board.makeMove(move);
					auto [eval, prev_move_str] = quiescence_search(quiescence_depth, alpha, beta, 1 - color, board);
					board.unmakeMove(move);

					if (eval < min_eval) {
                        min_eval = eval;
                        best_move_str = uci::moveToSan(board, move) + " " + prev_move_str;  // Get the move as a string
                    }
					beta = beta < min_eval ? beta:min_eval;

					if (beta <= alpha) break;
				}
				if (min_eval < B_WIN_THRE) {
                    table_insert(board_hash, mm_depth, {min_eval + 1, best_move_str}, color);
                    return {min_eval + 1, best_move_str};
                }
				else if (min_eval > W_WIN_THRE) {
                    table_insert(board_hash, mm_depth, {min_eval - 1, best_move_str}, color);
                    return {min_eval - 1, best_move_str};
                }
                table_insert(board_hash, mm_depth, {min_eval, best_move_str}, color);
				return {min_eval, best_move_str};
			}
		}
	}

    int best_score = color == chess::Color::WHITE ? -MAX_SCORE : MAX_SCORE;

    for (const auto& move : moves) {
        board.makeMove(move);
        uint64_t new_hash = board.hash();
        auto [score, prev_move_str] = minimax(mm_depth - 1, alpha, beta, chess::Color(1 - int(color)), board, new_hash);
        board.unmakeMove(move);

        if (color == chess::Color::WHITE) {
            if (score > best_score) {
                best_score = score;
                best_move_str = uci::moveToSan(board, move) + " " + prev_move_str;  // Record the best move
            }
            alpha = std::max(alpha, best_score);
        } else {
            if (score < best_score) {
                best_score = score;
                best_move_str = uci::moveToSan(board, move) + " " + prev_move_str;  // Record the best move
            }
            beta = std::min(beta, best_score);
        }

        if (beta <= alpha) {
            break;
        }
    }

    if (best_score < B_WIN_THRE) {
        table_insert(board_hash, mm_depth, {best_score + 1, best_move_str}, color);
        return {best_score + 1, best_move_str};
    }
    else if (best_score > W_WIN_THRE) {
        table_insert(board_hash, mm_depth, {best_score - 1, best_move_str}, color);
        return {best_score - 1, best_move_str};
    }
    table_insert(board_hash, mm_depth, {best_score, best_move_str}, color);
    return {best_score, best_move_str};
}

chess::Move findBestMove(chess::Board& board, int depth) {
    if (debug_mode) {
        mm_cnt = 0;
        q_cnt = 0;
    }

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);
    if (moves.empty()) {
        std::cout << "No legal moves available." << std::endl;
        return chess::Move();
    }

    sort_moves(moves, board);
    std::vector<int> evals(moves.size());
    int alpha = -MAX_SCORE;
    int beta = MAX_SCORE;

    chess::Color current_turn = board.sideToMove();
    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < moves.size(); ++i) {
        board.makeMove(moves[i]);
        uint64_t board_hash = board.hash();
        auto[eval, pv_move] = minimax(depth - 1, alpha, beta, 
                            chess::Color(1 - int(current_turn)), board, board_hash);

        evals[i] = eval;
        board.unmakeMove(moves[i]);

        // cout << i << " " << uci::moveToSan(board, moves[i]) << " " << pv_move << endl;
        if (current_turn == chess::Color::WHITE) {
            alpha = (alpha > eval)?alpha:eval;
        } else {
            beta = (beta < eval)?beta:eval;
        }
    }
    size_t best_index = 0;
    int best_eval = current_turn == chess::Color::WHITE ? -MAX_SCORE : MAX_SCORE;

    for (size_t i = 0; i < moves.size(); ++i) {
        if ((current_turn == chess::Color::WHITE && evals[i] > best_eval) ||
            (current_turn == chess::Color::BLACK && evals[i] < best_eval)) {
            best_eval = evals[i];
            best_index = i;
        }
    }
    std::cout << "Best move index: " << best_index << " with eval: " << best_eval << std::endl;
    
    if (debug_mode) {
        // Check in transposition table
        auto it = TTable.find(board.hash());
        if (it != TTable.end()) {
            auto [depth, value] = it -> second;
            cout << depth << " " << value.first << " " << value.second << endl;
        } else {
            cout << "Not in Transposition Table!" << endl;
        }
        std::cout << "Called minimax#: " << mm_cnt << ", called Qsearch#: " << q_cnt << std::endl;
    }

    return moves[best_index];
}