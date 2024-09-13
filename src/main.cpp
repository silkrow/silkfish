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
bool debug_mode = false; // Not being used.
int evals[1000];
std::counting_semaphore<MAX_THREAD> thread_limit(MAX_THREAD);

void usage_error() {
	std::cout << "Usage: ./silkrow <-flag1> <option1> <-flag2> <option2> ... <-fen> {fen_string}" << endl;
	return;
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

		Color turn = board.sideToMove();

		string game_pgn = "";
		int round = 1;
		
		LennyPOOL lenny_pool(MAX_THREAD);

		while (board.isGameOver().first == GameResultReason::NONE) {
			movegen::legalmoves(moves, board);
			Move picked_move;
			auto start = std::chrono::high_resolution_clock::now();
			picked_move = findBestMove(board, mm_depth, MAX_THREAD);
			auto end = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> duration = end - start;
			if (!mute) {
				std::cout << "Execution time: " << duration.count() << " seconds\n";
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
		Board board = Board(fen_string);
		Color turn = board.sideToMove();
		Movelist moves;
		movegen::legalmoves(moves, board);
		Move picked_move;
		auto start = std::chrono::high_resolution_clock::now();
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
