# Add setup code to your chess game
I used the lecture code to help me with the board.

# Chess Movement

## Part 1:
I used the lecture code to help with movement of the knight, king, and pawn.
- The knight checks where it can move with a list of L shaped vectors in generateKnightMoveBitBoard.
- The king checks where it can move with a list of adjacent and diagonals in generateKingMoveBitBoard.
- The pawn checks where it can move by checking if it is the first step (it can move 2 squares), and then checks the forward diagonals for any occupied enemies.

These check for whether there is an occupied space and whether it is an enemy or not.

## Part 2:
- I used the provided MagicBitboards.h to help with rook, bishop, and queen movement.
- The pieces use similar logic to the previously implemented pieces, where it checks what moves are valid. 