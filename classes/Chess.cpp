#include "Chess.h"
#include "Bit.h"
#include "Bitboard.h"
#include "ChessSquare.h"
#include "MagicBitboards.h"
#include <cstdint>
#include <limits>
#include <cmath>
#include <cctype>

Chess::Chess()
{
    _grid = new Grid(8, 8);
    for (int i = 0; i < 64; i++) {
        _knightBitboards[i] = generateKnightMoveBitBoard(i);
        _kingBitboards[i] = generateKingMoveBitBoard(i);
    }
    initMagicBitboards(); 

    for (int i = 0; i < 128; i++) {_bitboardLookup[i] = 0; }
    _bitboardLookup['P'] = WHITE_PAWNS;
    _bitboardLookup['N'] = WHITE_KNIGHTS;
    _bitboardLookup['B'] = WHITE_BISHOPS;
    _bitboardLookup['R'] = WHITE_ROOKS;
    _bitboardLookup['Q'] = WHITE_QUEENS;
    _bitboardLookup['K'] = WHITE_KING;
    _bitboardLookup['p'] = BLACK_PAWNS;
    _bitboardLookup['n'] = BLACK_KNIGHTS;
    _bitboardLookup['b'] = BLACK_BISHOPS;
    _bitboardLookup['r'] = BLACK_ROOKS;
    _bitboardLookup['q'] = BLACK_QUEENS;
    _bitboardLookup['k'] = BLACK_KING;
    _bitboardLookup['0'] = EMPTY_SQUARES;

}

Chess::~Chess()
{
    cleanupMagicBitboards();
    delete _grid;
}

char Chess::pieceNotation(int x, int y) const
{
    const char *wpieces = { "0PNBRQK" };
    const char *bpieces = { "0pnbrqk" };
    Bit *bit = _grid->getSquare(x, y)->bit();
    char notation = '0';
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag()-128];
    }
    return notation;
}

Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;

    _grid->initializeChessSquares(pieceSize, "boardsquare.png");
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    //FENtoBoard("p2k2n1/8/8/8/8/8/P2K2N1");
    //FENtoBoard("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR");
    
    _currentPlayer = WHITE;
    _moves = generateAllMoves();

    startGame();
}

void Chess::FENtoBoard(const std::string& fen) {
    // convert a FEN string to a board
    // FEN is a space delimited string with 6 fields
    // 1: piece placement (from white's perspective)
    // NOT PART OF THIS ASSIGNMENT BUT OTHER THINGS THAT CAN BE IN A FEN STRING
    // ARE BELOW
    // 2: active color (W or B)
    // 3: castling availability (KQkq or -)
    // 4: en passant target square (in algebraic notation, or -)
    // 5: halfmove clock (number of halfmoves since the last capture or pawn advance)

    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->setBit(nullptr);
    });

    int row = 7;
    int col = 0;

    for (char c : fen) {
        // if extra end bit then exit
        if (c == ' ') break;
        // if / then next line
        if (c == '/') {
            row--;
            col = 0;
        // if number then skip columns
        } else if (std::isdigit(c)) {
            col += c - '0';
        } else {
            ChessPiece piece;
            if (std::toupper(c) == 'R') {
                piece = Rook;
            } else if (std::toupper(c) == 'N') {
                piece = Knight;
            } else if (std::toupper(c) == 'B') {
                piece = Bishop;
            } else if (std::toupper(c) == 'Q') {
                piece = Queen;
            } else if (std::toupper(c) == 'K') {
                piece = King;
            } else {
                piece = Pawn;
            }

            // if uppercase white, else black
            Bit* bit = PieceForPlayer(std::isupper(c) ? 0 : 1, piece);
            ChessSquare* square = _grid->getSquare(col, row);
            bit->setPosition(square->getPosition());
            bit->setParent(square);
            bit->setGameTag(std::isupper(c) ? piece : (piece + 128));
            square->setBit(bit);

            col++;
        }
    }
}

bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    // need to implement friendly/unfriendly in bit so for now this hack
    int currentPlayer = getCurrentPlayer()->playerNumber() * 128;
    int pieceColor = bit.gameTag() & 128;
    
    if (pieceColor == currentPlayer) return true;
    return false;

    if (pieceColor != currentPlayer) return false;
    
    /*
    clearBoardHighlights();
    ChessSquare* srcSquare = (ChessSquare*)&src;
    int fromSquareIndex = srcSquare->getSquareIndex();
    for (const auto& move : _moves) {
        if (move.from == fromSquareIndex) {
            int file = move.to % 8;
            int rank = move.to / 8;
            ChessSquare* destSquare = _grid->getSquare(file, rank);
            destSquare->setHighlighted(true);
        }
    }
    return true;
    */
}

bool Chess::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    ChessSquare* squareSrc = (ChessSquare *)&src;
    ChessSquare* squareDst = (ChessSquare *)&dst;

    int squareIndexSrc = squareSrc->getSquareIndex();
    int squareIndexDst = squareDst->getSquareIndex();
    for (auto move: _moves) {
        if (move.from == squareIndexSrc && move.to == squareIndexDst) {
            return true;
        }
    }
    return false;
}

void Chess::clearBoardHighlights() {
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->setHighlighted(false);
    });
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) {
    _currentPlayer = (_currentPlayer == WHITE ? BLACK : WHITE);
    _moves = generateAllMoves();
    clearBoardHighlights();
    endTurn();
}

void Chess::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

Player* Chess::ownerAt(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        return nullptr;
    }

    auto square = _grid->getSquare(x, y);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* Chess::checkForWinner()
{
    return nullptr;
}

bool Chess::checkForDraw()
{
    return false;
}

std::string Chess::initialStateString()
{
    return stateString();
}

std::string Chess::stateString()
{
    std::string s;
    s.reserve(64);
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
            s += pieceNotation( x, y );
        }
    );
    return s;}

void Chess::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 8 + x;
        char playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit(PieceForPlayer(playerNumber - 1, Pawn));
        } else {
            square->setBit(nullptr);
        }
    });
}

// ----------------------------------------------------------------------------------------------------------

BitboardElement Chess::generateKnightMoveBitBoard(int square) {
    BitboardElement bitboard = 0ULL;
    int file = square % 8;
    int rank = square / 8;
    
    // All possible L-shapes from the position
    std::pair<int, int> knightOffsets[] = {
        {2, 1}, {-2, 1}, {2, -1}, {-2, -1},
        {1, 2}, {-1, 2}, {1, -2}, {-1, -2}
    };

    constexpr uint64_t oneBit = 1;
    for (auto [dr, df] : knightOffsets) {
        int r = rank + dr, f = file + df;
        if (r >= 0 && r < 8 && f >=0 && f < 8) {
            bitboard |= oneBit << (r * 8 + f);
        }
    }

    return bitboard;
}

BitboardElement Chess::generateKingMoveBitBoard(int square) {
    BitboardElement bitboard = 0ULL;
    int file = square % 8;
    int rank = square / 8;
    
    std::pair<int, int> kingOffsets[] = {
        {1, 0}, {-1, 0},
        {0, 1}, {0, -1},
        {1, -1}, {-1, 1},
        {1, 1}, {-1, -1}
    };

    constexpr uint64_t oneBit = 1;
    for (auto [dr, df] : kingOffsets) {
        int r = rank + dr, f = file + df;
        if (r >= 0 && r < 8 && f >=0 && f < 8) {
            bitboard |= oneBit << (r * 8 + f);
        }
    }

    return bitboard;
}

void Chess::generateKnightMoves(std::vector<BitMove>& moves, BitboardElement knightBoard, uint64_t occupancy) {
    knightBoard.forEachBit([&](int fromSquare) {
        BitboardElement canMoveTo(_knightBitboards[fromSquare].getData() & occupancy);
        canMoveTo.forEachBit([fromSquare, &moves](int to) {
            moves.emplace_back(fromSquare, to, Knight);
        });
    });
}

void Chess::generateKingMoves(std::vector<BitMove>& moves, BitboardElement kingBoard, uint64_t occupancy) {
    kingBoard.forEachBit([&](int fromSquare) {
        BitboardElement canMoveTo(_kingBitboards[fromSquare].getData() & occupancy);
        canMoveTo.forEachBit([fromSquare, &moves](int to) {
            moves.emplace_back(fromSquare, to, Knight);
        });
    });
}

void Chess::generatePawnMoves(std::vector<BitMove>& moves, BitboardElement pawnBoard, uint64_t occupancy, uint64_t opp_occupancy) {
    pawnBoard.forEachBit([&](int fromSquare) {
        int file = fromSquare % 8;
        int rank = fromSquare / 8;

        bool isWhite = (_currentPlayer == WHITE);
        int direction = isWhite ? 1 : -1;

        int oneStepRank = rank + direction;
        if (oneStepRank >= 0 && oneStepRank < 8) {
            int oneStepSquare = oneStepRank * 8 + file;

            if (occupancy & (1ULL << oneStepSquare)) {
                moves.emplace_back(fromSquare, oneStepSquare, Pawn);

                int startRank = isWhite ? 1 : 6;
                if (rank == startRank) {
                    int twoStepRank = rank + 2 * direction;
                    int twoStepSquare = twoStepRank * 8 + file;

                    if (occupancy & (1ULL << twoStepSquare)) {
                        moves.emplace_back(fromSquare, twoStepSquare, Pawn);
                    }
                }
            }
        }

        // Diagonals
        int captureFiles[2] = { file - 1, file + 1 };
        for (int cf : captureFiles) {
            int captureRank = rank + direction;
            if (cf >= 0 && cf < 8 && captureRank >= 0 && captureRank < 8) {
                int captureSquare = captureRank * 8 + cf;

                if (opp_occupancy & (1ULL << captureSquare)) {
                    moves.emplace_back(fromSquare, captureSquare, Pawn);
                }
            }
        }
    });
}

void Chess::generateRookMoves(std::vector<BitMove>& moves, BitboardElement rookBoard, uint64_t occupancy, uint64_t self_occupancy) {
    rookBoard.forEachBit([&](int from) {
        BitboardElement canMoveTo(getRookAttacks(from, occupancy) & ~self_occupancy);
        canMoveTo.forEachBit([from, &moves](int to) {
            moves.emplace_back(from, to, Rook);
        });
    });
}

void Chess::generateBishopMoves(std::vector<BitMove>& moves, BitboardElement bishopBoard, uint64_t occupancy, uint64_t self_occupancy) {
    bishopBoard.forEachBit([&](int from) {
        BitboardElement canMoveTo(getBishopAttacks(from, occupancy) & ~self_occupancy);
        canMoveTo.forEachBit([from, &moves](int to) {
            moves.emplace_back(from, to, Bishop);
        });
    });
}

void Chess::generateQueenMoves(std::vector<BitMove>& moves, BitboardElement queenBoard, uint64_t occupancy, uint64_t self_occupancy) {
    queenBoard.forEachBit([&](int from) {
        BitboardElement canMoveTo(getQueenAttacks(from, occupancy) & ~self_occupancy);
        canMoveTo.forEachBit([from, &moves](int to) {
            moves.emplace_back(from, to, Queen);
        });
    });
}

std::vector<BitMove> Chess::generateAllMoves() {
    std::vector<BitMove> moves;
    moves.reserve(32);
    std::string state = stateString();

    for (int i = 0; i < e_numBitBoards; i++) {
        _bitboards[i] = 0;
    }

    for (int i = 0; i < 64; i++) {
        int bitIndex = _bitboardLookup[state[i]];
        _bitboards[bitIndex] |= 1ULL << i;
        if (state[i] != '0') {
            _bitboards[OCCUPANCY] |= 1ULL << i;
            _bitboards[isupper(state[i]) ? WHITE_ALL_PIECES : BLACK_ALL_PIECES] |= 1ULL << i;
        }
    }

    int bitIndex = _currentPlayer == WHITE ? WHITE_PAWNS : BLACK_PAWNS;
    int oppBitIndex = _currentPlayer == WHITE ? BLACK_PAWNS : WHITE_PAWNS;
    
    int selfOccupancyIndex = _currentPlayer == WHITE ? WHITE_ALL_PIECES : BLACK_ALL_PIECES;
    int oppOccupancyIndex = _currentPlayer == WHITE ? BLACK_ALL_PIECES : WHITE_ALL_PIECES;

    generateKnightMoves(moves, _bitboards[WHITE_KNIGHTS + bitIndex], ~_bitboards[selfOccupancyIndex].getData());
    generateKingMoves(moves, _bitboards[WHITE_KING + bitIndex], ~_bitboards[selfOccupancyIndex].getData());
    generatePawnMoves(moves, _bitboards[WHITE_PAWNS + bitIndex], ~_bitboards[OCCUPANCY].getData(), _bitboards[oppOccupancyIndex].getData());

    generateBishopMoves(moves, _bitboards[WHITE_BISHOPS + bitIndex], _bitboards[OCCUPANCY].getData(), _bitboards[selfOccupancyIndex].getData());
    generateRookMoves(moves, _bitboards[WHITE_ROOKS + bitIndex], _bitboards[OCCUPANCY].getData(), _bitboards[selfOccupancyIndex].getData());
    generateQueenMoves(moves, _bitboards[WHITE_QUEENS + bitIndex], _bitboards[OCCUPANCY].getData(), _bitboards[selfOccupancyIndex].getData());
    
    return moves;
}