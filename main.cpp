#include "chess-library/include/chess.hpp"
#include "constants.hpp"

#include <chrono>

#include <thread>
#include <vector>
#include <semaphore>
#include <future>
#include <iostream>
#include <algorithm>
#include <queue>
#include <csignal>

using namespace chess;
using namespace std;

int quiescence_depth = DEFAULT_DEPTH_Q;
int mm_depth = DEFAULT_DEPTH_MM;
float time_limit = 15.0;
auto start = std::chrono::high_resolution_clock::now();
bool debug_mode = false; // Not being used.
int evals[1000];
std::string game_pgn = "GAME FINISHED YOOOOOOOOO BIIIIIIIIIIIITCH. SAY MY NAME!!!!!!";
std::counting_semaphore<MAX_THREAD> thread_limit(MAX_THREAD);

// InputParser source: https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c
class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }
        // @author iain
        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        // @author iain
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};

int evaluation(Board& board) {
	int evaluation = 0;

	if (board.isGameOver().second == GameResult::DRAW) {
		return 0;
	}

	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
		return board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
	}

	for (int i = 0; i < BOARD_SIZE; i++) {
		auto piece = board.at(Square((uint8_t)i));
		if (piece != Piece::NONE) {
			if (piece.color() == Color::WHITE) {
				evaluation += PST[piece.type()][BOARD_SIZE - 1 - (8*(i/8) + 7 - i%8)];
			} else {
				evaluation -= PST[piece.type()][i];
			}
		} 
	}

	return evaluation;
}

bool is_capture_move(const Board& board, const Move& move) {
    const Piece& from_piece = board.at<Piece>(move.from());
    const Piece& to_piece = board.at<Piece>(move.to());
    if (from_piece.type() == PieceType::NONE) {
        return false;
    }
    if (to_piece.type() != PieceType::NONE && to_piece.color() != from_piece.color()) {
        return true;
    }
    return false;
}

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "Best move index: " << game_pgn << std::endl;
        exit(0); 
    }
}


bool appear_quiet(Board board) {
	if (board.inCheck())
		return false;

	Movelist moves;
	movegen::legalmoves(moves, board);
	for (int i = 0; i < moves.size(); i++) {
		const auto move = moves[i];
		board.makeMove(move);
		if (is_capture_move(board, move)) {
			int attacker_val = PIECE_VAL[(int)((board.at<Piece>(move.from())).type())];
			int victim_val = PIECE_VAL[(int)((board.at<Piece>(move.to())).type())];
            cout << board.at<Piece>(move.to()).type() << endl;
			if (attacker_val < victim_val) {
				board.unmakeMove(move);
				return false;
			}
		}
		board.unmakeMove(move);
	}

	return true;
}

int quiescence_search (int q_depth, int alpha, int beta, Color color, Board board) {
	// Force quit if time is up
	if (time_limit > 0) {
		auto end = std::chrono::high_resolution_clock::now();
		chrono::duration<double> duration = end - start;
		if (duration.count() > time_limit) {
			return color == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
		}
	}

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

// int minimax (int mm_depth, int alpha, int beta, Color color, Board board) {
// 	// Force quit if time is up
// 	if (time_limit > 0) {
// 		auto end = std::chrono::high_resolution_clock::now();
// 		chrono::duration<double> duration = end - start;
// 		if (duration.count() > time_limit) {
// 			return color == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
// 		}
// 	}

// 	Movelist moves;
// 	movegen::legalmoves(moves, board);
// 	sort(moves.begin(), moves.end(), [board] (Move a, Move b) {
// 	int a_val = 0, b_val = 0;

// 	if (a == Move::CASTLING) a_val = CASTLE; // Castle is encoded as king capturing rook in the library!
// 	else if (board.isCapture(a)) {
// 		int af = (int)((board.at<Piece>(a.from())).type());
// 		int at = (int)((board.at<Piece>(a.to())).type());
		
// 		std::pair<int, int> key = std::make_pair(af, at % 6);
// 		a_val = capture_score[key];
// 	}

// 	if (b == Move::CASTLING) b_val = CASTLE;
// 	else if (board.isCapture(b)) {
// 		int bf = (int)((board.at<Piece>(b.from())).type());
// 		int bt = (int)((board.at<Piece>(b.to())).type());
		
// 		std::pair<int, int> key = std::make_pair(bf, bt % 6);
// 		b_val = capture_score[key];
// 	}

//     return a_val > b_val;
// 	});

// 	if (mm_depth == 0) {
// 		if (appear_quiet(board)) {
// 			return evaluation(board);
// 		} else {
// 			if (color == Color::WHITE) {
// 				int max_eval = -MAX_SCORE;
// 				for (int i = 0; i < moves.size(); i++) {
// 					const auto move = moves[i];
// 					board.makeMove(move);
// 					int eval = quiescence_search(DEFAULT_DEPTH_Q, alpha, beta, 1 - color, board);
// 					board.unmakeMove(move);

// 					max_eval = eval > max_eval ? eval:max_eval;
// 					alpha = alpha > max_eval ? alpha:max_eval;

// 					if (beta <= alpha) break;
// 				}
// 				if (max_eval < B_WIN_THRE) return max_eval + 1;
// 				else if (max_eval > W_WIN_THRE) return max_eval - 1;
// 				return max_eval;
// 			} else {
// 				int min_eval = MAX_SCORE;
// 				for (int i = 0; i < moves.size(); i++) {
// 					const auto move = moves[i];
// 					board.makeMove(move);
// 					int eval = quiescence_search(quiescence_depth, alpha, beta, 1 - color, board);
// 					board.unmakeMove(move);

// 					min_eval = eval < min_eval ? eval:min_eval;
// 					beta = beta < min_eval ? beta:min_eval;

// 					if (beta <= alpha) break;
// 				}
// 				if (min_eval < B_WIN_THRE) return min_eval + 1;
// 				else if (min_eval > W_WIN_THRE) return min_eval - 1;
// 				return min_eval;
// 			}
// 		}
// 	}

// 	if (board.isGameOver().second == GameResult::DRAW) {
// 		return 0;
// 	}

// 	if (board.isGameOver().first == GameResultReason::CHECKMATE) {
// 		return board.sideToMove() == Color::BLACK ? MAX_SCORE:-MAX_SCORE;
// 	}

// 	if (color == Color::WHITE) {
// 		int max_eval = -MAX_SCORE;
// 		for (int i = 0; i < moves.size(); i++) {
// 			const auto move = moves[i];
// 			board.makeMove(move);
// 			int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, board);
// 			board.unmakeMove(move);

// 			max_eval = eval > max_eval ? eval:max_eval;
// 			alpha = alpha > max_eval ? alpha:max_eval;

// 			if (beta <= alpha) break;
// 		}

// 		if (max_eval < B_WIN_THRE) return max_eval + 1;
// 		else if (max_eval > W_WIN_THRE) return max_eval - 1;
// 		return max_eval;
// 	} else {
// 		int min_eval = MAX_SCORE;
// 		for (int i = 0; i < moves.size(); i++) {
// 			const auto move = moves[i];
// 			board.makeMove(move);
// 			int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, board);
// 			board.unmakeMove(move);

// 			min_eval = eval < min_eval ? eval:min_eval;
// 			beta = beta < min_eval ? beta:min_eval;

// 			if (beta <= alpha) break;
// 		}

// 		if (min_eval < B_WIN_THRE) return min_eval + 1;
// 		else if (min_eval > W_WIN_THRE) return min_eval - 1;
// 		return min_eval;
// 	}
// }
const int MIN_DEPTH_FOR_PARALLELISM = 2;



class LennyPOOL {
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<int> active_tasks{0};
    int max_threads;

public:
    LennyPOOL(int max_threads) : max_threads(max_threads), stop(false) {
        for (int i = 0; i < max_threads; ++i) {
            threads.emplace_back([this, i] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) {
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
    void run(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    void wait_all() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        while (true) {
            if (tasks.empty() && active_tasks == 0) break;
            condition.wait(lock);
        }
    }
    ~LennyPOOL() {
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
};

#include <map>

std::map<chess::PieceType, int> piece_value_map = {
    {chess::PieceType::PAWN, 1},
    {chess::PieceType::KNIGHT, 3},
    {chess::PieceType::BISHOP, 3},
    {chess::PieceType::ROOK, 5},
    {chess::PieceType::QUEEN, 9},
    {chess::PieceType::KING, 1000} 
};


int piece_value(chess::PieceType piece_type) {
    auto it = piece_value_map.find(piece_type);
    if (it != piece_value_map.end()) {
        return it->second;
    }
    return 0;
}


void sort_moves(chess::Movelist& moves, const chess::Board& board) {
    std::sort(moves.begin(), moves.end(), [&board](const chess::Move& a, const chess::Move& b) {
        int a_val = 0, b_val = 0;

        if (a == chess::Move::CASTLING) {
            a_val = CASTLE;
        } else if (is_capture_move(board, a)) {
            chess::PieceType attacker_type_a = board.at<chess::Piece>(a.from()).type();
            chess::PieceType victim_type_a = board.at<chess::Piece>(a.to()).type();
            a_val = 10 * piece_value(victim_type_a) - piece_value(attacker_type_a);
        }

        if (b == chess::Move::CASTLING) {
            b_val = CASTLE;
        } else if (is_capture_move(board, b)) {
            chess::PieceType attacker_type_b = board.at<chess::Piece>(b.from()).type();
            chess::PieceType victim_type_b = board.at<chess::Piece>(b.to()).type();
            b_val = 10 * piece_value(victim_type_b) - piece_value(attacker_type_b);
        }

        return a_val > b_val;
    });
}

int minimax(int depth, int alpha, int beta, chess::Color color, chess::Board& board, 
            const std::chrono::time_point<std::chrono::high_resolution_clock>& start_time, double time_limit) {
    auto current_time = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration<double>(current_time - start_time).count() > time_limit && time_limit > 0) {
        std::cout << "Time limit exceeded for color: " << (color == chess::Color::WHITE ? "WHITE" : "BLACK") << std::endl;
        return color == chess::Color::WHITE ? -MAX_SCORE : MAX_SCORE;
    }

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);

	sort_moves(moves, board);

    if (depth == 0) {
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

    int best_score = color == chess::Color::WHITE ? -MAX_SCORE : MAX_SCORE;

    for (const auto& move : moves) {
        board.makeMove(move);
        int score = minimax(depth - 1, alpha, beta, chess::Color(1 - int(color)), board, start_time, time_limit);
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
	if (best_score < B_WIN_THRE) return best_score + 5;
	else if (best_score > W_WIN_THRE) return best_score - 5;
    return best_score;
}


void usage_error() {
	std::cout << "Usage: ./silkrow <-flag1> <option1> <-flag2> <option2> ... <-fen> {fen_string}" << endl;
	return;
}

Color fen_player_color(string& fen) {
	size_t spacePos = fen.find(' ');
    if (spacePos != string::npos) {
        char toMove = fen[spacePos + 1];
        if (toMove == 'w') {
            return Color::WHITE;
        } else if (toMove == 'b') {
            return Color::BLACK;
        }
    }

    throw std::invalid_argument("Invalid FEN string format");
}

void handle_uci_command();


chess::Move findBestMove(chess::Board& board, int depth, int max_threads, double time_limit) {
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
                               chess::Color(1 - int(current_turn)), board_copy, start_time, time_limit);
            if (current_turn == chess::Color::WHITE) {
                int current_alpha = alpha.load();
                while (evals[i] > current_alpha && 
                       !alpha.compare_exchange_weak(current_alpha, evals[i]));
            } else {
                int current_beta = beta.load();
                while (evals[i] < current_beta && 
                       !beta.compare_exchange_weak(current_beta, evals[i]));
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



int main (int argc, char *argv[]) {
	if (argc == 1) {      // UCI mode if no argument passed in.
		handle_uci_command();
		return 0;
	}
    std::signal(SIGINT, signal_handler);

	string fen_string = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; 
	// fen_string = "4k3/8/6K1/8/3Q4/8/8/8 w - - 0 1"; // for testing purpose
	bool demo_mode = false;
	bool mute = false;

	InputParser input(argc, argv);

	if (input.cmdOptionExists("-m")) {
		mute = true;
	} 

	if (input.cmdOptionExists("-demo")) {
		demo_mode = true;
	} 

	const string &mm_depth_s = input.getCmdOption("-md");
	if (!mm_depth_s.empty()) {
		try {
			size_t pos;
			mm_depth = stoi(mm_depth_s, &pos);
			if (pos != mm_depth_s.length()) {
				usage_error();
				return 1;
			}
		} catch (const std::invalid_argument&) {
			usage_error();
			return 1;
    	} catch (const std::out_of_range&) {
        	usage_error();
			return 1;
		}
	}

	const string &q_depth_s = input.getCmdOption("-qd");
	if (!q_depth_s.empty()) {
		try {
			size_t pos;
			quiescence_depth = stoi(q_depth_s, &pos);
			if (pos != q_depth_s.length()) {
				usage_error();
				return 1;
			}
		} catch (const std::invalid_argument&) {
			usage_error();
			return 1;
    	} catch (const std::out_of_range&) {
        	usage_error();
			return 1;
		}
	}

	const string &time_limit_s = input.getCmdOption("-t");
	if (!time_limit_s.empty()) {
		try {
			size_t pos;
			time_limit = (double) stoi(time_limit_s, &pos);
			if (pos != time_limit_s.length()) {
				usage_error();
				return 1;
			}
		} catch (const std::invalid_argument&) {
			usage_error();
			return 1;
    	} catch (const std::out_of_range&) {
        	usage_error();
			return 1;
		}
	}

	if (!demo_mode) {
		int arg_start = 0;
		for (int i = 0; i < argc - 1; i++) {
			if (string(argv[i]) == "-fen") {
				arg_start = i + 1;
				break;
			}
		}

		fen_string = "";
		for (int j = arg_start; j < argc; ++j) {
        	fen_string += argv[j];
        	if (j < argc - 1) {
            	fen_string += " "; // Add a space between arguments
        	}
    	}
	}

	// Main logic
	if (demo_mode) {
		if (!mute) {
			std::cout << "Running in demo mode with mm_depth " << mm_depth << ", q_depth " << quiescence_depth 
					<< ", time_limit " << time_limit << ", engine vs engine." << std::endl;
		}

		chess::Board board(fen_string);
		chess::Movelist moves;

		chess::Color turn = board.sideToMove();

		
		int round = 1;
		LennyPOOL lenny_pool(MAX_THREAD);

		while (board.isGameOver().first == chess::GameResultReason::NONE) {
			chess::movegen::legalmoves(moves, board);
			chess::Move picked_move;
			auto start = std::chrono::high_resolution_clock::now();
			picked_move = findBestMove(board, mm_depth, MAX_THREAD, time_limit);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = end - start;

			if (!mute) {
				std::cout << "Execution time: " << duration.count() << " seconds\n";
			}
			std::string move_s = chess::uci::moveToSan(board, picked_move);
			if (turn == chess::Color::WHITE) {
				game_pgn += " " + std::to_string(round) + ". " + move_s;
			} else {
				game_pgn += " " + move_s;
				round++;
			}
			board.makeMove(picked_move);

			if (!mute) {
				std::cout << move_s << std::endl << std::endl;
			}

			turn = chess::Color(1 - int(turn));
		}

		std::cout << game_pgn << std::endl;
	} else { // engine mode
		Color turn = fen_player_color(fen_string);
		Board board = Board(fen_string);
		Movelist moves;
		movegen::legalmoves(moves, board);
		Move picked_move;
		start = std::chrono::high_resolution_clock::now();
		int evals[moves.size()];

		if (turn == Color::WHITE) fill(evals, evals + moves.size(), -MAX_SCORE);
		else fill(evals, evals + moves.size(), MAX_SCORE);

		for (int i = 0; i < moves.size(); i++) {
			const auto move = moves[i];
			board.makeMove(move);
			// evals[i] = minimax(mm_depth, -MAX_SCORE, MAX_SCORE, 1 - turn, board);	
			board.unmakeMove(move);
		}

		int eval = evals[0];
		picked_move = moves[0];
		for (int i = 0; i < moves.size(); i++) {
			if ((turn == Color::WHITE && eval < evals[i]) ||
			(turn == Color::BLACK && eval > evals[i])) {
				eval = evals[i];
				picked_move = moves[i];
			}
		}
		auto end = std::chrono::high_resolution_clock::now();

		chrono::duration<double> duration = end - start;
		if (!mute) {
			std::cout << "Execution time: " << duration.count() << " seconds\n";
			printf("eval: %d\n", eval);
			std::cout << "Engine mm_depth: " << mm_depth  << ", q_depth: " << quiescence_depth << ", time_limit: " << time_limit << "s" << endl;
		}
		string move_s = uci::moveToSan(board, picked_move);
		board.makeMove(picked_move);
		std::cout << move_s << endl;
	}
    return 0;
}

///////////////// UCI implementation //////////////////

int move_overhead = 0; // Default value

void send_uci_info() {
    std::cout << "id name silkrow" << endl;
    std::cout << "id author Erkai Yu" << endl;
    std::cout << "uciok" << endl;
}

void send_ready_ok() {
    std::cout << "readyok" << endl;
}

void send_best_move(const Move& best_move) {
    std::cout << "bestmove " << uci::moveToUci(best_move) << endl;
}

// Function to handle UCI options like "Move Overhead"
void set_option(const std::string& name, const std::string& value) {
    if (name == "Move Overhead") {
        move_overhead = stoi(value);  // Convert value to integer and set move_overhead
        std::cout << "info string Set Move Overhead to " << move_overhead << " ms" << endl;
    } else {
        // For unsupported options, ignore or log a message
        std::cout << "info string Unsupported option: " << name << endl;
    }
}

void handle_uci_command() {
	time_limit = 0;
    string command;
	Board board;
    while (getline(cin, command)) {
        if (command == "uci") {
            send_uci_info();
        } else if (command == "isready") {
            send_ready_ok();
        } else if (command.rfind("position", 0) == 0) {
            // Handle "position" command
            size_t startpos_start = command.find("startpos");
            size_t fen_start = command.find("fen");
            if (startpos_start != string::npos) {
                // If "startpos" is given, set up the board with the initial position
                board = Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
            } else if (fen_start != string::npos) {
                // Handle FEN string if present
                string fen_string = command.substr(fen_start + 4);
                board = Board(fen_string);
            }

            // Handle moves after "position" command
            size_t moves_start = command.find("moves");
            if (moves_start != string::npos) {
                string moves_string = command.substr(moves_start + 6);
                istringstream move_stream(moves_string);
                string move_str;
                while (move_stream >> move_str) {
                    Move move = uci::uciToMove(board, move_str);
                    board.makeMove(move);
                }
            }
        } else if (command.rfind("go", 0) == 0) {
			size_t btime_pos = command.find("btime");
            size_t wtime_pos = command.find("wtime");

			int time_left;

			if (board.sideToMove() == Color::WHITE && wtime_pos != string::npos) {
                time_left = stoi(command.substr(wtime_pos + 6));
            } else if (board.sideToMove() == Color::BLACK && btime_pos != string::npos) {
                time_left = stoi(command.substr(btime_pos + 6));
            }

			if (time_left > 20*60*1000) mm_depth = 8;
			else if (time_left > 5*60*1000) mm_depth = 7;
			else if (time_left > 1*60*1000) mm_depth = 6;
			else mm_depth = 5;

            // Run search algorithm
            Movelist moves;
            movegen::legalmoves(moves, board);
			LennyPOOL lenny_pool(MAX_THREAD);
			start = std::chrono::high_resolution_clock::now();

			Move picked_move = findBestMove(board, mm_depth, MAX_THREAD, time_limit);
			auto end = std::chrono::high_resolution_clock::now();
			chrono::duration<double> duration = end - start;

            send_best_move(picked_move);
        } else if (command == "stop") {
            // Stop the search and return the best move found so far
        } else if (command == "quit") {
            break;
        } else if (command.rfind("setoption", 0) == 0) {
            // Handle "setoption" command for setting engine options
            size_t name_start = command.find("name");
            size_t value_start = command.find("value");
            if (name_start != string::npos && value_start != string::npos) {
                string option_name = command.substr(name_start + 5, value_start - name_start - 6);
                string option_value = command.substr(value_start + 6);
                set_option(option_name, option_value);
            }
        } else if (command == "debug on") {
            debug_mode = true;
        } else if (command == "debug off") {
            debug_mode = false;
        }
    }
}

////////////// End of UCI imlementation /////////////