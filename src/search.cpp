#include <chrono>
#include "chess.hpp"
#include "search.hpp"
#include "evaluation.hpp"
#include "constants.hpp"

using namespace chess;
using namespace std;


std::vector<std::thread> threads;
std::queue<std::function<void()>> tasks;
std::mutex queue_mutex;
std::condition_variable condition;
std::atomic<bool> stop;
std::atomic<int> active_tasks{0};
LennyPOOL::LennyPOOL(int max_threads) : max_threads(max_threads), stop(false) {
    for (int i = 0; i < max_threads; ++i) {
        threads.emplace_back([this, i] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock, [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) {
                        // std::cout << "Thread " << i << " exiting." << std::endl;
                        return;
                    }
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                active_tasks++;
                task();
                active_tasks--;
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    condition.notify_all();
                }
            }
        });
    }
}

template<class F>
void LennyPOOL::run(F&& f) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(std::forward<F>(f));
    }
    condition.notify_one();
}

void LennyPOOL::wait_all() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    while (true) {
        if (tasks.empty() && active_tasks == 0) break;
        condition.wait(lock);
    }
}
LennyPOOL::~LennyPOOL() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all(); 
    for (std::thread& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

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


int quiescence_search (int q_depth, int alpha, int beta, Color color, Board board) {
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
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

    if (board.isGameOver().second == GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

    sort_moves(moves, board);

	if (mm_depth == 0) {
		if (appear_quiet(board)) {
			return evaluation(board);
		} else {
			// if (color == Color::WHITE) {
			// 	int max_eval = -MAX_SCORE;
			// 	for (int i = 0; i < moves.size(); i++) {
			// 		const auto move = moves[i];
			// 		board.makeMove(move);
			// 		int eval = quiescence_search(quiescence_depth, alpha, beta, 1 - color, board);
			// 		board.unmakeMove(move);

			// 		max_eval = eval > max_eval ? eval:max_eval;
			// 		alpha = alpha > max_eval ? alpha:max_eval;

			// 		if (beta <= alpha) break;
			// 	}
			// 	if (max_eval < B_WIN_THRE) return max_eval + 1;
			// 	else if (max_eval > W_WIN_THRE) return max_eval - 1;
			// 	return max_eval;
			// } else {
			// 	int min_eval = MAX_SCORE;
			// 	for (int i = 0; i < moves.size(); i++) {
			// 		const auto move = moves[i];
			// 		board.makeMove(move);
			// 		int eval = quiescence_search(quiescence_depth, alpha, beta, 1 - color, board);
			// 		board.unmakeMove(move);

			// 		min_eval = eval < min_eval ? eval:min_eval;
			// 		beta = beta < min_eval ? beta:min_eval;

			// 		if (beta <= alpha) break;
			// 	}
			// 	if (min_eval < B_WIN_THRE) return min_eval + 1;
			// 	else if (min_eval > W_WIN_THRE) return min_eval - 1;
			// 	return min_eval;
			// }
            int eval = quiescence_search(4, alpha, beta, color, board);
            return eval;
		}
	}

    int best_score = color == chess::Color::WHITE ? -MAX_SCORE : MAX_SCORE;

    for (const auto& move : moves) {
        board.makeMove(move);
        int score = minimax(mm_depth - 1, alpha, beta, chess::Color(1 - int(color)), board);
        board.unmakeMove(move);

        if (color == chess::Color::WHITE) {
            best_score = std::max(best_score, score);
            alpha = std::max(alpha, best_score);
        } else {
            best_score = std::min(best_score, score);
            beta = std::min(beta, best_score);
        }

        if (beta <= alpha) {
            break;
        }
    }

    if (best_score < B_WIN_THRE) return best_score + 1;
    else if (best_score > W_WIN_THRE) return best_score - 1;
    return best_score;
}

chess::Move findBestMove(chess::Board& board, int depth, int max_threads) {
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);
    if (moves.empty()) {
        std::cout << "No legal moves available." << std::endl;
        return chess::Move();
    }

    sort_moves(moves, board);
    std::vector<int> evals(moves.size());
    std::atomic<int> alpha(-MAX_SCORE);
    std::atomic<int> beta(MAX_SCORE);
    LennyPOOL lenny_pool(max_threads);

    chess::Color current_turn = board.sideToMove();
    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < moves.size(); ++i) {
        lenny_pool.run([&, i]() {
            chess::Board board_copy = board;
            board_copy.makeMove(moves[i]);
            evals[i] = minimax(depth - 1, alpha.load(), beta.load(), 
                               chess::Color(1 - int(current_turn)), board_copy);
            if (current_turn == chess::Color::WHITE) {
                int current_alpha = alpha.load();
                while (evals[i] > current_alpha && 
                       !alpha.compare_exchange_weak(current_alpha, evals[i])) {
                }
            } else {
                int current_beta = beta.load();
                while (evals[i] < current_beta && 
                       !beta.compare_exchange_weak(current_beta, evals[i])) {
                }
            }
        });
    }
    lenny_pool.wait_all();
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
    return moves[best_index];
}