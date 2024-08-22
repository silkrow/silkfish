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
        piece_values = {
            chess.PAWN: 1,
            chess.KNIGHT: 3,
            chess.BISHOP: 3,
            chess.ROOK: 5,
            chess.QUEEN: 9
        }

        center_squares = (chess.E4, chess.E5, chess.D4, chess.D5)
        second_center = (chess.C3, chess.D3, chess.E3, chess.F3, chess.F4, chess.F5, chess.F6, chess.E6, chess.D6, chess.C6, chess.C5, chess.C4)
        corner_squares = (chess.A1, chess.A8, chess.H1, chess.H8)
        side_squares = (chess.A2, chess.A3, chess.A4, chess.A5, chess.A6, chess.A7, \
                        chess.H2, chess.H3, chess.H4, chess.H5, chess.H6, chess.H7, \
                        chess.B1, chess.C1, chess.D1, chess.E1, chess.F1, chess.G1, \
                        chess.B8, chess.C8, chess.D8, chess.E8, chess.F8, chess.G8)
        
        white_material = 0
        black_material = 0

        # Iterate over all squares
        for square in chess.SQUARES:
            piece = board.piece_at(square)
            if piece:
                piece_val = piece_values.get(piece.piece_type, 0)
                if piece.piece_type is not chess.KING:
                    if square in center_squares:
                        piece_val += 0.04
                    elif square in second_center:
                        piece_val += 0.02
                if piece.piece_type in (chess.KNIGHT, chess.BISHOP):
                    if square in corner_squares:
                        piece_val -= 0.05
                    elif square in side_squares:
                        piece_val -= 0.03

                if piece.color == chess.WHITE:
                    white_material += piece_val
                else:
                    black_material += piece_val

        return white_material, black_material

    def castling_bonus(self, board):
        evaluation = 0
        castling_right = 0.3
        castled = 0.1
        in_center = -0.3

        # Check for castling
        if board.has_kingside_castling_rights(chess.WHITE) or board.has_queenside_castling_rights(chess.WHITE):
            evaluation += castling_right  
        if board.has_kingside_castling_rights(chess.BLACK) or board.has_queenside_castling_rights(chess.BLACK):
            evaluation -= castling_right 

        # Reward castling
        white_castled = (
            board.piece_at(chess.G1) == chess.KING and
            board.piece_at(chess.F1) == chess.ROOK
        ) or (
            board.piece_at(chess.C1) == chess.KING and
            board.piece_at(chess.D1) == chess.ROOK
        )

        black_castled = (
            board.piece_at(chess.G8) == chess.KING and
            board.piece_at(chess.F8) == chess.ROOK
        ) or (
            board.piece_at(chess.C8) == chess.KING and
            board.piece_at(chess.D8) == chess.ROOK
        )

        if white_castled:
            evaluation += castled
        if black_castled:
            evaluation -= castled

        # Penalize if king is still in the center
        if not white_castled:
            if board.king(chess.WHITE) in [chess.E1, chess.D1]:
                evaluation -= in_center
        if not black_castled:
            if board.king(chess.BLACK) in [chess.E8, chess.D8]:
                evaluation += in_center
        
        return evaluation

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
        evaluation += self.castling_bonus(board)

        return evaluation
