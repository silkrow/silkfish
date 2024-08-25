import random
import chess

from enum import Enum, auto
from configure import Configure
from evaluation import Evaluation

class PlayerType(Enum):
    HUMAN = auto()
    ENGINE = auto()

class Player:
    """
    A class for the player in a game, it stores all the configuration for the 
    player, including settings for an engine, the color it is using, etc.
    """

    def __init__ (self, type, conf_file=None):
        self.type = type
        self.color = None
        self.nodes_searched = 0
    
        # Load configuration    
        if self.type == PlayerType.ENGINE:
            if conf_file == None:
                raise ValueError("Error: Engine player missing configuration file.")
            self.conf = Configure(conf_file)
            self.evaluation = Evaluation(self.conf)
        elif self.type == PlayerType.HUMAN:
            self.conf = None
            self.evaluation = None
        else:
            raise ValueError("Error: Player type invalid.")
    
    def assign_color(self, color):
        """
        Assuming the given color input is valid.
        """
        self.color = color
    
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
            self.nodes_searched = 0
            best_move = None
            max_eval = float('-inf')
            min_eval = float('inf')
            alpha = float('-inf')
            beta = float('inf')
            depth = self.conf.config["minimax_depth"]

            legal_moves = list(board.legal_moves)
            random.shuffle(legal_moves)
            for move in legal_moves:
                board.push(move)
                if self.color == chess.WHITE:
                    eval = self.minimax(board, depth - 1, alpha, beta, chess.BLACK)
                else:
                    eval = self.minimax(board, depth - 1, alpha, beta, chess.WHITE)
                board.pop()
                if self.color == chess.WHITE and eval > max_eval:
                    max_eval = eval
                    best_move = move
                if self.color == chess.BLACK and eval < min_eval:
                    min_eval = eval
                    best_move = move
            if best_move == None:
                best_move = legal_moves[0]
            
            print(f"Minimax nodes searched: {self.nodes_searched}")
            if self.color == chess.WHITE:
                print(f"White Engine eval: {max_eval}")
            else:
                print(f"Black Engine eval: {min_eval}")
            return best_move

        else:
            raise ValueError("Error: Player type invalid.")

    ########## KEY LOGIC OF SEARCHING IN GAME TREE ##########
    def minimax(self, board, depth, alpha, beta, maximizing_color, capture_square=None):
        self.nodes_searched += 1
        
        if depth == 0 or board.is_game_over():
            return self.evaluation.evaluate(board)

        if maximizing_color == chess.WHITE:
            max_eval = float('-inf')
            legal_moves = list(board.legal_moves)
            random.shuffle(legal_moves)
            for move in legal_moves:
                board.push(move)
                if depth == 1:
                    if capture_square is None:
                        if board.is_capture(move):
                            eval = self.minimax(board, depth, alpha, beta, chess.BLACK, move.to_square)
                        else:
                            eval = self.minimax(board, depth - 1, alpha, beta, chess.BLACK) # Regular depth = 1 situation
                    else:
                        if move.to_square == capture_square:
                            eval = self.minimax(board, depth, alpha, beta, chess.BLACK, move.to_square)
                        else:
                            board.pop()
                            continue
                else:
                    eval = self.minimax(board, depth - 1, alpha, beta, chess.BLACK)
                board.pop()
                max_eval = max(max_eval, eval)
                alpha = max(alpha, max_eval)
                if beta <= alpha:
                    break
            if depth == 1 and capture_square is not None and max_eval == float('-inf'):
                return self.evaluation.evaluate(board)
            return max_eval

        else:
            min_eval = float('inf')
            legal_moves = list(board.legal_moves)
            random.shuffle(legal_moves)
            for move in legal_moves:
                board.push(move)
                if depth == 1:
                    if capture_square is None:
                        if board.is_capture(move):
                            eval = self.minimax(board, depth, alpha, beta, chess.WHITE, move.to_square)
                        else:
                            eval = self.minimax(board, depth - 1, alpha, beta, chess.WHITE) # Regular depth = 1 situation
                    else:
                        if move.to_square == capture_square:
                            eval = self.minimax(board, depth, alpha, beta, chess.WHITE, move.to_square)
                        else:
                            board.pop()
                            continue
                else:
                    eval = self.minimax(board, depth - 1, alpha, beta, chess.WHITE)
                board.pop()
                min_eval = min(min_eval, eval)
                beta = min(beta, min_eval)
                if beta <= alpha:
                    break
            if depth == 1 and capture_square is not None and min_eval == float('inf'):
                return self.evaluation.evaluate(board)
            return min_eval

