import chess

class Evaluation:
    """
    A class to give an evaluation of a given position. Each 
    engine Player object has its own Evaluation object, 
    initialized with the configuration loaded by Configuration.
    """

    def __init__(self, conf) -> None:
        """
        Load the configuration of the engine, define some
        default constants.
        """
        self.conf = conf

    def material_count(self, board):
        """
        Returns:
            int, int: material count for white and black.
        """
        piece_values = {
            chess.PAWN: 1,
            chess.KNIGHT: 3,
            chess.BISHOP: 3,
            chess.ROOK: 5,
            chess.QUEEN: 9
        }
        
        white_material = 0
        black_material = 0

        # Iterate over all squares
        for square in chess.SQUARES:
            piece = board.piece_at(square)
            if piece:
                # Add piece value to the corresponding side's material count
                if piece.color == chess.WHITE:
                    white_material += piece_values.get(piece.piece_type, 0)
                else:
                    black_material += piece_values.get(piece.piece_type, 0)

        return white_material, black_material



    def evaluate(self, board):
        """
        Returns:
            float: the evaluation of the current board.
        """
        # Check for terminal states (checkmate, stalemate)
        if board.is_checkmate():
            if board.turn == chess.WHITE:
                return float('-inf')  # Black wins
            else:
                return float('inf')   # White wins
        elif board.is_stalemate() or board.is_insufficient_material():
            return 0  # Draw

        # Material count evaluation
        white_material, black_material = self.material_count(board)

        # Basic evaluation based on material difference
        return white_material - black_material
