# -*- coding: utf-8 -*-
import numpy as np


def all_patterns(pent):

    temp_pattern_list = []
    temp_pent = np.copy(pent)

    for _ in range(4):
        # print(temp_pent)
        temp_pattern_list.append(temp_pent)
        temp_pent = np.rot90(temp_pent)
    
    temp_pent = np.flip(temp_pent, 0)

    for _ in range(4):
        # print(temp_pent)
        temp_pattern_list.append(temp_pent)
        temp_pent = np.rot90(temp_pent)

    # print(temp_pattern_list)
    
    unique_arrays = {}

    for pent in temp_pattern_list:
        pent_tuple = (pent.tobytes(), pent.shape)  
        if pent_tuple not in unique_arrays:
            unique_arrays[pent_tuple] = pent
            
    return(list(unique_arrays.values()))

def all_positions(board, pent):

    b_row, b_col = board.shape
    matrices = []
    info = []
    for cur in all_patterns(pent):

        # print(pent.shape)
        rows, cols = cur.shape
        for i in range(b_row - rows + 1):
            for j in range(b_col - cols + 1):
                cur_board = np.copy(board)
                # print(board_clip)
                fail = False
                for x in range(rows):
                    if fail:
                        break
                    for y in range(cols):
                        if cur[x][y] != 0 and cur_board[i+x][j+y] == 1:
                            fail = True
                            break
                        cur_board[i+x][j+y] = cur[x][y]
                if not fail:
                    matrices.append(cur_board)
                    info.append((cur, i, j))
    return matrices, info

def exact_cover(A, partial, original_r):
    row, col = A.shape
    if col == 0:
        return partial
    else:
        c = np.argmin(A.sum(axis=0))
        if A[:, c].sum() == 0:
            return None
        for r in range(row):
            if A[r, c] != 1:
                continue
            partial.append(original_r[r])
            cols_to_delete = np.where(A[r, :] == 1)[0]
            rows_to_delete = np.where(A[:, cols_to_delete].any(axis=1))[0]
            B = np.delete(A, rows_to_delete, axis=0)
            B = np.delete(B, cols_to_delete, axis=1)
            new_original_r = [original_r[i] for i in range(row) if i not in rows_to_delete]
            answer = exact_cover(B, partial.copy(), new_original_r)
            if answer is not None:
                return answer
            partial.pop()


def solve(board, pents):
    """
    This is the function you will implement. It will take in a numpy array of the board
    as well as a list of n tiles in the form of numpy arrays. The solution returned
    is of the form [(p1, (row1, col1))...(pn,  (rown, coln))]
    where pi is a tile (may be rotated or flipped), and (rowi, coli) is 
    the coordinate of the upper left corner of pi in the board (lowest row and column index 
    that the tile covers).

    -Use np.flip and np.rot90 to manipulate pentominos.

    -You can assume there will always be a solution.
    """
    board = 1 - board
    # print(board)
    flat_board = board.flatten()
    
    # Generate the list of indices to delete from the exact cover matrix
    list_to_delete = np.where(flat_board == 1)[0] + len(pents)

    matrix = []
    info_matrix = []
    for i in range(len(pents)):
        all_pos_list, all_pos_info = all_positions(board, pents[i])
        for pos in all_pos_list:
            pos = np.append(np.zeros(len(pents)), pos)
            pos = np.delete(pos, list_to_delete)
            pos[i] = 1
            matrix.append(pos)
        info_matrix.extend(all_pos_info)

    # matrix is the big 2000 * 72 for exact cover algorithm X
    matrix = np.array(matrix)
    # print("matrix row numbers:", len(matrix))

    matrix[matrix > 0] = 1
    result_indices = exact_cover(matrix, [], list(range(len(matrix))))
    
    # Generate the solution in the required format
    solution = [(info_matrix[i][0], (info_matrix[i][1], info_matrix[i][2])) for i in result_indices]
    
    return solution