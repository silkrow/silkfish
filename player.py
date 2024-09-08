import sys
import time
import threading
import subprocess

from enum import Enum, auto

class PlayerType(Enum):
    HUMAN = auto()
    ENGINE = auto()

class Player:
    """
    A class for the player in a game, it stores all the configuration for the 
    player, including settings for an engine, the color it is using, etc.
    """

    def __init__ (self, type, mm_depth, q_depth, time_limit):
        self.type = type
        self.color = None
        self.nodes_searched = 0
        self.mm_depth = mm_depth
        self.q_depth = q_depth
        self.time_limit = time_limit
    
    def assign_color(self, color):
        """
        Assuming the given color input is valid.
        """
        self.color = color

    def spinner(self):
        while not task_done:
            for symbol in ['/', '|', '\\', '-']:
                sys.stdout.write('\rsilkfish is thinking ' + symbol)
                sys.stdout.flush()
                time.sleep(0.2)
    def run_engine(self, command):
        global output
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        output = result.stdout
    
    def get_move(self, board):
        """
        Returns:
            chess.Move: A move to be made given the board.
        """
        if self.type == PlayerType.HUMAN:
            while True:
                user_input = input("Enter your move in SAN format (e.g. Nf3): ")
                
                # Try to parse the input move
                try:
                    move = board.parse_san(user_input)  # Parse Standard Algebraic Notation (SAN)
                except ValueError:
                    print("Invalid move format. Please try again.")
                    continue
                
                # Check if the move is legal
                if move in board.legal_moves:
                    return move
                else:
                    print("Illegal move. Please try again.")

        elif self.type == PlayerType.ENGINE:
            command = ['./silkfish', '-md', str(self.mm_depth), '-qd', str(self.q_depth), '-t', str(self.time_limit), '-fen', board.fen()]

            global task_done, output
            task_done = False
            output = None

            spinner_thread = threading.Thread(target=self.spinner)
            spinner_thread.start()

            run_engine_thread = threading.Thread(target=self.run_engine, args=(command,))
            run_engine_thread.start()
            run_engine_thread.join()

            task_done = True
            spinner_thread.join()

            sys.stdout.write('\r' + ' ' * 10 + '\r')
            sys.stdout.flush()

            best_move = output.strip().split('\n')[-1]

            print(f"silkfish with minimax depth {self.mm_depth} q depth {self.q_depth} time limit {self.time_limit}s makes move: {best_move}")
            move = board.parse_san(best_move)
            return move

        else:
            raise ValueError("Error: Player type invalid.")
