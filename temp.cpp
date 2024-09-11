#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;  // For synchronization of shared resources
int thread_count = 0;
const int MAX_THREADS = 8;

int minimax(int mm_depth, int alpha, int beta, Color color, Board board) {
    minimax_searched++;

    Movelist moves;
    movegen::legalmoves(moves, board);
    std::sort(moves.begin(), moves.end(), compare_moves);

    if (board.isGameOver().second == GameResult::DRAW) {
        return 0;
    }

    if (board.isGameOver().first == GameResultReason::CHECKMATE) {
        return board.sideToMove() == Color::BLACK ? MAX_SCORE : -MAX_SCORE;
    }

    // Threaded part
    if (color == Color::WHITE) {
        int max_eval = -MAX_SCORE;
        std::vector<std::thread> threads;

        for (int i = 0; i < moves.size(); i++) {
            const auto move = moves[i];
            Board new_board = board;  // Copy board
            new_board.makeMove(move);

            if (thread_count < MAX_THREADS) {
                thread_count++;
                threads.emplace_back([&]() {
                    int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, new_board);
                    new_board.unmakeMove(move);

                    std::lock_guard<std::mutex> lock(mtx);
                    max_eval = std::max(max_eval, eval);
                    alpha = std::max(alpha, max_eval);
                    thread_count--;
                });
            } else {
                int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, new_board);
                new_board.unmakeMove(move);

                max_eval = std::max(max_eval, eval);
                alpha = std::max(alpha, max_eval);
            }

            if (beta <= alpha) break;
        }

        for (auto& th : threads) {
            th.join();
        }

        return max_eval;
    } else {
        int min_eval = MAX_SCORE;
        std::vector<std::thread> threads;

        for (int i = 0; i < moves.size(); i++) {
            const auto move = moves[i];
            Board new_board = board;  // Copy board
            new_board.makeMove(move);

            if (thread_count < MAX_THREADS) {
                thread_count++;
                threads.emplace_back([&]() {
                    int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, new_board);
                    new_board.unmakeMove(move);

                    std::lock_guard<std::mutex> lock(mtx);
                    min_eval = std::min(min_eval, eval);
                    beta = std::min(beta, min_eval);
                    thread_count--;
                });
            } else {
                int eval = minimax(mm_depth - 1, alpha, beta, 1 - color, new_board);
                new_board.unmakeMove(move);

                min_eval = std::min(min_eval, eval);
                beta = std::min(beta, min_eval);
            }

            if (beta <= alpha) break;
        }

        for (auto& th : threads) {
            th.join();
        }

        return min_eval;
    }
}



#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <semaphore>

constexpr int MAX_THREAD = 8;  // Limit the maximum number of concurrent threads
std::counting_semaphore<MAX_THREAD> thread_limit(MAX_THREAD);  // Initialize the semaphore with MAX_THREAD permits

void fs(int depth) {
    if (depth <= 0) return;  // Termination condition

    std::cout << "Depth: " << depth << " - Thread: " << std::this_thread::get_id() << std::endl;

    // Simulate some work (e.g., branching in a search)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Spawn child threads (if within thread limit)
    std::vector<std::thread> children;
    for (int i = 0; i < 2; ++i) {  // Let's say each node has 2 branches
        thread_limit.acquire();  // Acquire a thread slot (decrement the semaphore)
        children.emplace_back([depth]() {
            fs(depth - 1);    // Recurse with reduced depth
            thread_limit.release();  // Release the thread slot (increment the semaphore)
        });
    }

    // Join all children threads
    for (auto& child : children) {
        if (child.joinable())
            child.join();
    }
}

int main() {
    // Start the search with the root depth (e.g., 3)
    fs(3);

    return 0;
}
