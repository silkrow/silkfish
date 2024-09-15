import threading
import sys
import subprocess

# Function to run the silkfish engine
def run_engine(command):
    global output
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = process.communicate()
    output = out.decode()

# Spinner function to show while engine is thinking
def spinner():
    while not task_done:
        for symbol in "|/-\\":
            sys.stdout.write(f'\r{symbol}')
            sys.stdout.flush()
            threading.Event().wait(0.1)

def run_test(fen, best_move):
    board_fen = fen.strip()

    mm_depth = 8
    time_limit = 15
    command = ['./silkfish', '-md', str(mm_depth), '-t', str(time_limit), '-fen', board_fen]

    global task_done, output
    task_done = False
    output = None

    spinner_thread = threading.Thread(target=spinner)
    spinner_thread.start()

    run_engine_thread = threading.Thread(target=run_engine, args=(command,))
    run_engine_thread.start()
    run_engine_thread.join()

    task_done = True
    spinner_thread.join()

    sys.stdout.write('\r' + ' ' * 10 + '\r')
    sys.stdout.flush()

    engine_best_move = output.strip().split('\n')[-1]

    print(f"Expected best move: {best_move}, Engine best move: {engine_best_move}")
    if engine_best_move == best_move:
        print("Test passed!")
        return True
    else:
        print("Test failed.")
        return False

def parse_eret_tests(filename):
    tests = []
    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if line and (' bm ' in line or ' am ' in line):  # Check if the line contains "bm" or "am"
                try:
                    # Split using either " bm " or " am "
                    if ' bm ' in line:
                        fen_part, move_part = line.split(' bm ', 1)
                    else:
                        fen_part, move_part = line.split(' am ', 1)

                    fen = fen_part.strip()
                    best_move = move_part.split(';')[0].strip()  # Extract the move
                    tests.append((fen, best_move))
                except ValueError:
                    print(f"Skipping malformed line: {line}")
            else:
                print(f"Skipping line without 'bm' or 'am': {line}")
    return tests

if __name__ == "__main__":
    eret_tests = parse_eret_tests('tests/ERET/eret.txt')

    total_tests = len(eret_tests)
    correct_tests = 0

    for idx, (fen, best_move) in enumerate(eret_tests, start=1):
        print(f"Running test {idx}: FEN = {fen}, Best move = {best_move}")
        if run_test(fen, best_move):
            correct_tests += 1

    print(f"\nTotal tests: {total_tests}")
    print(f"Correct tests: {correct_tests}")
    print(f"Accuracy: {correct_tests / total_tests * 100:.2f}%")
