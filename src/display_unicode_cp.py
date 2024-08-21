import chess

def print_board(color, board):
    """
    A function to print a given board with pieces in UNICODE format, the direction of the board
    is decided by the color passed in. 
    """
    UNICODE_PIECE_SYMBOLS = {
        "r": "♜",
        "n": "♞",
        "b": "♝",
        "q": "♛",
        "k": "♚",
        "p": "♙",
    }

    side_color = 237

    def colored(text, text_color, background_color):
        text = " " + text # Padding to center the pieces
        return f"\x1b[48;5;{background_color}m\x1b[38;5;{text_color}m{text}"
                
    print()

    if color == chess.WHITE:            
        for rank in range(7, -1, -1):
            row = []
            for file in range(8):
                square = chess.square(file, rank)
                piece = board.piece_at(square)
                square_color = 214 if (rank + file) % 2 == 0 else 94
                piece_color = 231
                if piece == None:
                    piece_symbol = " "
                else:
                    piece_symbol = UNICODE_PIECE_SYMBOLS.get(piece.symbol().lower())
                    if piece.symbol().islower():
                        piece_color = 16
                square_content = colored(piece_symbol, piece_color, square_color)
                row.append(square_content)
            print(f"\x1b[0m {rank + 1}\x1b[48;5;{side_color}m {' '.join(row)} \x1b[48;5;{side_color}m \x1b[0m ")
        print("\x1b[0m    a  b  c  d  e  f  g  h")

    else:
        board = board.transform(chess.flip_vertical).transform(chess.flip_horizontal)
        for rank in range(7, -1, -1):
            row = []
            for file in range(8):
                square = chess.square(file, rank)
                piece = board.piece_at(square)
                square_color = 214 if (rank + file) % 2 == 0 else 94
                piece_color = 231
                if piece == None:
                    piece_symbol = " "
                else:
                    piece_symbol = UNICODE_PIECE_SYMBOLS.get(piece.symbol().lower())
                    if piece.symbol().islower():
                        piece_color = 16
                square_content = colored(piece_symbol, piece_color, square_color)
                row.append(square_content)
            print(f"\x1b[0m {8 - rank}\x1b[48;5;{side_color}m {' '.join(row)} \x1b[48;5;{side_color}m \x1b[0m ")
        print("\x1b[0m    h  g  f  e  d  c  b  a") 