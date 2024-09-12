#include "chess.hpp"
#include "constants.hpp"
#include "input_parser.hpp"
#include "evaluation.hpp"
#include "search.hpp"
#include "uci.hpp"

#include <chrono>

#include <thread>
#include <vector>
#include <semaphore>

using namespace chess;
using namespace std;

int quiescence_depth = DEFAULT_DEPTH_Q;
int mm_depth = DEFAULT_DEPTH_MM;
float time_limit = 15.0;
auto start = std::chrono::high_resolution_clock::now();
bool debug_mode = false; // Not being used.
int evals[1000];
std::counting_semaphore<MAX_THREAD> thread_limit(MAX_THREAD);

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

int main (int argc, char *argv[]) {
	if (argc == 1) {      // UCI mode if no argument passed in.
		handle_uci_command();
		return 0;
	}

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
            std::cout << "Running in demo mode with mm_depth " << mm_depth << ", q_depth " << quiescence_depth << ", time_limit " << time_limit << ", engine vs engine." << endl;
        }

		Board board = Board(fen_string);
		Movelist moves;

		Color turn = fen_player_color(fen_string);

		string game_pgn = "";
		int round = 1;
		mutex mtx;
		while (board.isGameOver().first == GameResultReason::NONE) {
			movegen::legalmoves(moves, board);
			Move picked_move;
			start = std::chrono::high_resolution_clock::now();

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

			if (turn == Color::WHITE) fill(evals, evals + moves.size(), -MAX_SCORE);
			else fill(evals, evals + moves.size(), MAX_SCORE);

			std::vector<std::thread> children;
			mutex mtx;
			int alpha = -MAX_SCORE;
			int beta = MAX_SCORE;

			for (int i = 0; i < moves.size(); i++) {
				const auto move = moves[i];
				Board board_cp = board;
				thread_limit.acquire(); 
				children.emplace_back([board_cp = std::move(board_cp), move = std::move(move), turn, i, &alpha, &beta, &mtx]() mutable {
					board_cp.makeMove(move);
					evals[i] = minimax(mm_depth, alpha, beta, 1 - turn, board_cp);

					if (turn == Color::WHITE && evals[i] > alpha) {
						mtx.lock();
						alpha = evals[i];
						mtx.unlock();
					} else if (turn == Color::BLACK && evals[i] < beta){
						mtx.lock();
						beta = evals[i];
						mtx.unlock();
					}
					thread_limit.release();
				});
			}

			for (auto& child : children) {
        		if (child.joinable()) {
            		child.join();
				}
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
			}
			string move_s = uci::moveToSan(board, picked_move);
			if (turn == Color::WHITE) {
				game_pgn = game_pgn + " " + to_string(round) + ". " + move_s;
			} else {
				game_pgn = game_pgn + " " + move_s;
				round++;
			}
			board.makeMove(picked_move);
			if (!mute) {
				std::cout << move_s << endl << endl;
			}
			turn = 1 - turn;
		} 

		std::cout << game_pgn << endl;
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
			evals[i] = minimax(mm_depth, -MAX_SCORE, MAX_SCORE, 1 - turn, board);	
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
		// board.makeMove(picked_move);
		std::cout << move_s << endl;
	}
    return 0;
}
