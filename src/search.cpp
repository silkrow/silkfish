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
        int af = (int)((board.at<Piece>(a.from())).type());
        a_val = af;
		if (a == Move::CASTLING) a_val = CASTLE;
		else if (board.isCapture(a)) {
			int at = (int)((board.at<Piece>(a.to())).type());

			std::pair<int, int> key = std::make_pair(af, at % 6);
			a_val = capture_score[key];
		}

        int bf = (int)((board.at<Piece>(b.from())).type());
        b_val = bf;
		if (b == Move::CASTLING) b_val = CASTLE;
		else if (board.isCapture(b)) {
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

        if (depth >= mm_depth) return value;
    }

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    if (mm_depth > 0) sort_moves(moves, board);

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
		if (true) {
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

    int co = (board.sideToMove() == Color::WHITE)? 1:-1;
    int scores[moves.size()];
    int init_score = -1000;
    for (size_t i = 0; i < moves.size(); i++) {

        int a_val = 0;
        int af = (int)((board.at<Piece>(moves[i].from())).type());
        a_val = af;
		if (moves[i] == Move::CASTLING) a_val = CASTLE;
		else if (board.isCapture(moves[i])) {
			int at = (int)((board.at<Piece>(moves[i].to())).type());

			std::pair<int, int> key = std::make_pair(af, at % 6);
			a_val = capture_score[key];
		}

        board.makeMove(moves[i]);
        auto it = TTable.find(board.hash());
        if (it != TTable.end()) {
            auto [depth, value] = it -> second;
            scores[i] = value.first * co;
        } else {
            scores[i] = init_score + a_val;
        }
        board.unmakeMove(moves[i]);
    }

    int move_order[moves.size()];
    
    for (int i = 0; i < moves.size(); ++i) {
        move_order[i] = i;
    }

    auto comparator = [&](int i, int j) {
        return scores[i] > scores[j];
    };

    sort(move_order, move_order + moves.size(), comparator);

    std::vector<int> evals(moves.size());
    int alpha = -MAX_SCORE;
    int beta = MAX_SCORE;

    chess::Color current_turn = board.sideToMove();
    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < moves.size(); ++i) {
        int j = move_order[i];

        if (debug_mode) {
            cout << i << ": " << uci::moveToSan(board, moves[j]) << " " << scores[j] << endl;
        }

        board.makeMove(moves[j]);
        uint64_t board_hash = board.hash();

        int eval;
        string pv_move;

        std::tie(eval, pv_move) = minimax(depth - 1, alpha, beta, 
                            chess::Color(1 - int(current_turn)), board, board_hash);

        evals[j] = eval;
        board.unmakeMove(moves[j]);

        // cout << i << " " << uci::moveToSan(board, moves[i]) << " " << pv_move << endl;
        if (current_turn == chess::Color::WHITE) {
            alpha = (alpha > eval)?alpha:eval;
        } else {
            beta = (beta < eval)?beta:eval;
        }
    }
    size_t best_index = 0;
    int best_eval = current_turn == chess::Color::WHITE ? -MAX_SCORE : MAX_SCORE;

    int move_order_index = 0;
    for (size_t i = 0; i < moves.size(); ++i) {
        int j = move_order[i];
        if ((current_turn == chess::Color::WHITE && evals[j] > best_eval) ||
            (current_turn == chess::Color::BLACK && evals[j] < best_eval)) {
            best_eval = evals[j];
            best_index = j;
            move_order_index = i;
        }
    }
    std::cout << "Best move index: " << move_order_index << " with eval: " << best_eval << std::endl;
    
    if (debug_mode) {
        std::cout << "Called minimax#: " << mm_cnt << ", called Qsearch#: " << q_cnt << std::endl;
    }

    return moves[best_index];
}