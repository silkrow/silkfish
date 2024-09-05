import chess

class Evaluation:
    """
    A class to give an evaluation of a given position. Each 
    engine Player object has its own Evaluation object, 
    initialized with the configuration loaded by Configuration.
    """

    def __init__(self, conf) -> None:
        self.conf = conf

    def material_count(self, board):
        # Define some constants
        TOTAL_SQUARE = 64
        PIECE = {
            chess.ROOK: (
                500, 500, 500, 500, 500, 500, 500, 500,
                520, 520, 520, 520, 520, 520, 520, 520,
                500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 500, 500, 500, 500, 500,
                500, 500, 500, 510, 510, 505, 500, 500
            ), 
            chess.KING: (
                -10, -10, 0, 0, 0, 0, -10, -10,
                -10, -10, 0, 0, 0, 0, -10, -10,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0,
                10, 10, 0, 0, 0, 0, 10, 10,
            ),
            chess.BISHOP: (
                290, 300, 300, 300, 300, 300, 300, 290,
                300, 305, 305, 305, 305, 305, 305, 300,
                300, 305, 325, 325, 325, 325, 305, 300,
                300, 305, 325, 325, 325, 325, 305, 300,
                300, 305, 325, 325, 325, 325, 305, 300,
                300, 305, 320, 325, 325, 325, 305, 300,
                300, 305, 305, 305, 305, 305, 305, 300,
                290, 310, 300, 300, 300, 300, 310, 290
            ), 
            chess.KNIGHT: (
                290, 300, 300, 300, 300, 300, 300, 290,
                300, 305, 305, 305, 305, 305, 305, 300,
                300, 305, 325, 325, 325, 325, 305, 300,
                300, 305, 325, 325, 325, 325, 305, 300,
                300, 305, 325, 325, 325, 325, 305, 300,
                300, 305, 320, 325, 325, 325, 305, 300,
                300, 305, 305, 305, 305, 305, 305, 300,
                290, 310, 300, 300, 300, 300, 310, 290
            ), 
            chess.QUEEN: (
                900, 900, 900, 900, 900, 900, 900, 900,
                900, 900, 900, 900, 900, 900, 900, 900,
                900, 900, 900, 900, 900, 900, 900, 900,
                900, 900, 900, 900, 900, 900, 900, 900,
                900, 900, 900, 900, 900, 900, 900, 900,
                900, 900, 900, 900, 900, 900, 900, 900,
                900, 900, 900, 900, 900, 900, 900, 900,
                900, 900, 900, 900, 900, 900, 900, 900
            ),
            chess.PAWN: (
                100, 100, 100, 100, 100, 100, 100, 100,
                130, 130, 130, 130, 130, 130, 130, 130,
                110, 110, 110, 125, 125, 110, 110, 110,
                100, 100, 100, 120, 120, 100, 100, 100,
                100, 100, 100, 110, 110, 100, 100, 100,
                100, 100, 100, 100, 100, 100, 100, 100,
                100, 100, 100, 100, 100, 100, 100, 100,
                100, 100, 100, 100, 100, 100, 100, 100,
            )

        }

        white_material = 0
        black_material = 0

        # Iterate over all squares
        for square in chess.SQUARES:
            piece = board.piece_at(square)
            if piece:
                if piece.color == chess.WHITE:
                    white_material += PIECE[piece.piece_type][square]
                else:
                    black_material += PIECE[piece.piece_type][TOTAL_SQUARE - 1 - square]

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

        evaluation = white_material - black_material

        return evaluation
