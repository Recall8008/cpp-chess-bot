#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <vector>

struct Move {
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
    char promotion = '\0';
};

class GameState {
public:
    GameState() {
        board = {{
            {{'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}},
            {{'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'}},
            {{'.', '.', '.', '.', '.', '.', '.', '.'}},
            {{'.', '.', '.', '.', '.', '.', '.', '.'}},
            {{'.', '.', '.', '.', '.', '.', '.', '.'}},
            {{'.', '.', '.', '.', '.', '.', '.', '.'}},
            {{'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'}},
            {{'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}}
        }};
    }

    void printBoard() const {
        std::cout << "\n    a b c d e f g h\n";
        std::cout << "   -----------------\n";
        for (int row = 0; row < 8; ++row) {
            std::cout << 8 - row << " | ";
            for (int col = 0; col < 8; ++col) {
                std::cout << board[row][col] << ' ';
            }
            std::cout << "| " << 8 - row << '\n';
        }
        std::cout << "   -----------------\n";
        std::cout << "    a b c d e f g h\n\n";
        std::cout << (whiteToMove ? "White" : "Black") << " to move.\n";
    }

    std::vector<Move> generateLegalMoves() const {
        std::vector<Move> legalMoves;
        auto pseudoMoves = generatePseudoMoves(whiteToMove);

        for (const Move& move : pseudoMoves) {
            GameState next = *this;
            next.applyMove(move);
            if (!next.isInCheck(!next.whiteToMove)) {
                legalMoves.push_back(move);
            }
        }

        return legalMoves;
    }

    bool isInCheck(bool white) const {
        const char king = white ? 'K' : 'k';
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                if (board[row][col] == king) {
                    return isSquareAttacked(row, col, !white);
                }
            }
        }
        return false;
    }

    void applyMove(const Move& move) {
        char piece = board[move.fromRow][move.fromCol];
        char target = board[move.toRow][move.toCol];

        updateCastlingStateForMove(piece, move.fromRow, move.fromCol);
        updateCastlingStateForCapture(target, move.toRow, move.toCol);

        board[move.fromRow][move.fromCol] = '.';
        board[move.toRow][move.toCol] = piece;

        if ((piece == 'K' || piece == 'k') && std::abs(move.toCol - move.fromCol) == 2) {
            moveRookForCastle(move);
        }

        if (piece == 'P' && move.toRow == 0) {
            board[move.toRow][move.toCol] = 'Q';
        } else if (piece == 'p' && move.toRow == 7) {
            board[move.toRow][move.toCol] = 'q';
        } else if (move.promotion != '\0') {
            board[move.toRow][move.toCol] = move.promotion;
        }

        whiteToMove = !whiteToMove;
    }

    bool isWhiteToMove() const {
        return whiteToMove;
    }

    char pieceAt(int row, int col) const {
        return board[row][col];
    }

    static std::string moveToString(const Move& move) {
        std::string result;
        result += static_cast<char>('a' + move.fromCol);
        result += static_cast<char>('8' - move.fromRow);
        result += static_cast<char>('a' + move.toCol);
        result += static_cast<char>('8' - move.toRow);
        return result;
    }

private:
    std::array<std::array<char, 8>, 8> board{};
    bool whiteToMove = true;
    bool whiteKingMoved = false;
    bool blackKingMoved = false;
    bool whiteQueenRookMoved = false;
    bool whiteKingRookMoved = false;
    bool blackQueenRookMoved = false;
    bool blackKingRookMoved = false;

    static bool inBounds(int row, int col) {
        return row >= 0 && row < 8 && col >= 0 && col < 8;
    }

    static bool isWhitePiece(char piece) {
        return std::isupper(static_cast<unsigned char>(piece));
    }

    static bool isBlackPiece(char piece) {
        return std::islower(static_cast<unsigned char>(piece));
    }

    static bool isEmpty(char piece) {
        return piece == '.';
    }

    static bool sameSide(char a, char b) {
        if (isEmpty(a) || isEmpty(b)) {
            return false;
        }
        return (isWhitePiece(a) && isWhitePiece(b)) || (isBlackPiece(a) && isBlackPiece(b));
    }

    bool isSquareAttacked(int row, int col, bool byWhite) const {
        int pawnRow = byWhite ? row + 1 : row - 1;
        if (inBounds(pawnRow, col - 1) && board[pawnRow][col - 1] == (byWhite ? 'P' : 'p')) {
            return true;
        }
        if (inBounds(pawnRow, col + 1) && board[pawnRow][col + 1] == (byWhite ? 'P' : 'p')) {
            return true;
        }

        static const int knightOffsets[8][2] = {
            {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
            {1, -2}, {1, 2}, {2, -1}, {2, 1}
        };
        for (const auto& offset : knightOffsets) {
            int nextRow = row + offset[0];
            int nextCol = col + offset[1];
            if (inBounds(nextRow, nextCol) && board[nextRow][nextCol] == (byWhite ? 'N' : 'n')) {
                return true;
            }
        }

        static const int bishopDirections[4][2] = {
            {-1, -1}, {-1, 1}, {1, -1}, {1, 1}
        };
        for (const auto& direction : bishopDirections) {
            int nextRow = row + direction[0];
            int nextCol = col + direction[1];
            while (inBounds(nextRow, nextCol)) {
                char piece = board[nextRow][nextCol];
                if (!isEmpty(piece)) {
                    if (piece == (byWhite ? 'B' : 'b') || piece == (byWhite ? 'Q' : 'q')) {
                        return true;
                    }
                    break;
                }
                nextRow += direction[0];
                nextCol += direction[1];
            }
        }

        static const int rookDirections[4][2] = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}
        };
        for (const auto& direction : rookDirections) {
            int nextRow = row + direction[0];
            int nextCol = col + direction[1];
            while (inBounds(nextRow, nextCol)) {
                char piece = board[nextRow][nextCol];
                if (!isEmpty(piece)) {
                    if (piece == (byWhite ? 'R' : 'r') || piece == (byWhite ? 'Q' : 'q')) {
                        return true;
                    }
                    break;
                }
                nextRow += direction[0];
                nextCol += direction[1];
            }
        }

        for (int rowOffset = -1; rowOffset <= 1; ++rowOffset) {
            for (int colOffset = -1; colOffset <= 1; ++colOffset) {
                if (rowOffset == 0 && colOffset == 0) {
                    continue;
                }
                int nextRow = row + rowOffset;
                int nextCol = col + colOffset;
                if (inBounds(nextRow, nextCol) && board[nextRow][nextCol] == (byWhite ? 'K' : 'k')) {
                    return true;
                }
            }
        }

        return false;
    }

    std::vector<Move> generatePseudoMoves(bool white) const {
        std::vector<Move> moves;

        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                char piece = board[row][col];
                if (isEmpty(piece)) {
                    continue;
                }
                if (white && !isWhitePiece(piece)) {
                    continue;
                }
                if (!white && !isBlackPiece(piece)) {
                    continue;
                }

                switch (std::tolower(static_cast<unsigned char>(piece))) {
                    case 'p':
                        addPawnMoves(moves, row, col, white);
                        break;
                    case 'n':
                        addKnightMoves(moves, row, col, white);
                        break;
                    case 'b':
                        addBishopMoves(moves, row, col);
                        break;
                    case 'r':
                        addRookMoves(moves, row, col);
                        break;
                    case 'q':
                        addQueenMoves(moves, row, col);
                        break;
                    case 'k':
                        addKingMoves(moves, row, col, white);
                        break;
                }
            }
        }

        return moves;
    }

    void addPawnMoves(std::vector<Move>& moves, int row, int col, bool white) const {
        int direction = white ? -1 : 1;
        int startRow = white ? 6 : 1;
        int promotionRow = white ? 0 : 7;

        int nextRow = row + direction;
        if (inBounds(nextRow, col) && isEmpty(board[nextRow][col])) {
            moves.push_back({row, col, nextRow, col, nextRow == promotionRow ? (white ? 'Q' : 'q') : '\0'});
            int jumpRow = row + (2 * direction);
            if (row == startRow && inBounds(jumpRow, col) && isEmpty(board[jumpRow][col])) {
                moves.push_back({row, col, jumpRow, col});
            }
        }

        for (int colOffset : {-1, 1}) {
            int nextCol = col + colOffset;
            if (!inBounds(nextRow, nextCol)) {
                continue;
            }
            char target = board[nextRow][nextCol];
            if (!isEmpty(target) && !sameSide(board[row][col], target)) {
                moves.push_back({
                    row,
                    col,
                    nextRow,
                    nextCol,
                    nextRow == promotionRow ? (white ? 'Q' : 'q') : '\0'
                });
            }
        }
    }

    void addKnightMoves(std::vector<Move>& moves, int row, int col, bool white) const {
        (void)white;
        static const int offsets[8][2] = {
            {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
            {1, -2}, {1, 2}, {2, -1}, {2, 1}
        };

        for (const auto& offset : offsets) {
            int nextRow = row + offset[0];
            int nextCol = col + offset[1];
            if (!inBounds(nextRow, nextCol)) {
                continue;
            }
            char target = board[nextRow][nextCol];
            if (isEmpty(target) || !sameSide(board[row][col], target)) {
                moves.push_back({row, col, nextRow, nextCol});
            }
        }
    }

    void addBishopMoves(std::vector<Move>& moves, int row, int col) const {
        static constexpr std::array<std::array<int, 2>, 4> directions = {{
            {{-1, -1}}, {{-1, 1}}, {{1, -1}}, {{1, 1}}
        }};
        addSlidingMoves(moves, row, col, directions);
    }

    void addRookMoves(std::vector<Move>& moves, int row, int col) const {
        static constexpr std::array<std::array<int, 2>, 4> directions = {{
            {{-1, 0}}, {{1, 0}}, {{0, -1}}, {{0, 1}}
        }};
        addSlidingMoves(moves, row, col, directions);
    }

    void addQueenMoves(std::vector<Move>& moves, int row, int col) const {
        static constexpr std::array<std::array<int, 2>, 8> directions = {{
            {{-1, -1}}, {{-1, 1}}, {{1, -1}}, {{1, 1}},
            {{-1, 0}}, {{1, 0}}, {{0, -1}}, {{0, 1}}
        }};
        addSlidingMoves(moves, row, col, directions);


    template <std::size_t N>
    void addSlidingMoves(
        std::vector<Move>& moves,
        int row,
        int col,
        const std::array<std::array<int, 2>, N>& directions
    ) const {
        for (const auto& direction : directions) {
            int nextRow = row + direction[0];
            int nextCol = col + direction[1];
            while (inBounds(nextRow, nextCol)) {
                char target = board[nextRow][nextCol];
                if (isEmpty(target)) {
                    moves.push_back({row, col, nextRow, nextCol});
                } else {
                    if (!sameSide(board[row][col], target)) {
                        moves.push_back({row, col, nextRow, nextCol});
                    }
                    break;
                }
                nextRow += direction[0];
                nextCol += direction[1];
            }
        }
    }

    void addKingMoves(std::vector<Move>& moves, int row, int col, bool white) const {
        for (int rowOffset = -1; rowOffset <= 1; ++rowOffset) {
            for (int colOffset = -1; colOffset <= 1; ++colOffset) {
                if (rowOffset == 0 && colOffset == 0) {
                    continue;
                }
                int nextRow = row + rowOffset;
                int nextCol = col + colOffset;
                if (!inBounds(nextRow, nextCol)) {
                    continue;
                }
                char target = board[nextRow][nextCol];
                if (isEmpty(target) || !sameSide(board[row][col], target)) {
                    moves.push_back({row, col, nextRow, nextCol});
                }
            }
        }

        addCastlingMoves(moves, white);
    }

    void addCastlingMoves(std::vector<Move>& moves, bool white) const {
        if (white) {
            if (whiteKingMoved || board[7][4] != 'K' || isInCheck(true)) {
                return;
            }
            if (!whiteKingRookMoved &&
                board[7][7] == 'R' &&
                board[7][5] == '.' &&
                board[7][6] == '.' &&
                !isSquareAttacked(7, 5, false) &&
                !isSquareAttacked(7, 6, false)) {
                moves.push_back({7, 4, 7, 6});
            }
            if (!whiteQueenRookMoved &&
                board[7][0] == 'R' &&
                board[7][1] == '.' &&
                board[7][2] == '.' &&
                board[7][3] == '.' &&
                !isSquareAttacked(7, 3, false) &&
                !isSquareAttacked(7, 2, false)) {
                moves.push_back({7, 4, 7, 2});
            }
        } else {
            if (blackKingMoved || board[0][4] != 'k' || isInCheck(false)) {
                return;
            }
            if (!blackKingRookMoved &&
                board[0][7] == 'r' &&
                board[0][5] == '.' &&
                board[0][6] == '.' &&
                !isSquareAttacked(0, 5, true) &&
                !isSquareAttacked(0, 6, true)) {
                moves.push_back({0, 4, 0, 6});
            }
            if (!blackQueenRookMoved &&
                board[0][0] == 'r' &&
                board[0][1] == '.' &&
                board[0][2] == '.' &&
                board[0][3] == '.' &&
                !isSquareAttacked(0, 3, true) &&
                !isSquareAttacked(0, 2, true)) {
                moves.push_back({0, 4, 0, 2});
            }
        }
    }

    void updateCastlingStateForMove(char piece, int fromRow, int fromCol) {
        if (piece == 'K') {
            whiteKingMoved = true;
        } else if (piece == 'k') {
            blackKingMoved = true;
        } else if (piece == 'R' && fromRow == 7 && fromCol == 0) {
            whiteQueenRookMoved = true;
        } else if (piece == 'R' && fromRow == 7 && fromCol == 7) {
            whiteKingRookMoved = true;
        } else if (piece == 'r' && fromRow == 0 && fromCol == 0) {
            blackQueenRookMoved = true;
        } else if (piece == 'r' && fromRow == 0 && fromCol == 7) {
            blackKingRookMoved = true;
        }
    }

    void updateCastlingStateForCapture(char target, int toRow, int toCol) {
        if (target == 'R' && toRow == 7 && toCol == 0) {
            whiteQueenRookMoved = true;
        } else if (target == 'R' && toRow == 7 && toCol == 7) {
            whiteKingRookMoved = true;
        } else if (target == 'r' && toRow == 0 && toCol == 0) {
            blackQueenRookMoved = true;
        } else if (target == 'r' && toRow == 0 && toCol == 7) {
            blackKingRookMoved = true;
        }
    }

    void moveRookForCastle(const Move& move) {
        if (move.toCol == 6) {
            board[move.toRow][5] = board[move.toRow][7];
            board[move.toRow][7] = '.';
            if (move.toRow == 7) {
                whiteKingRookMoved = true;
            } else {
                blackKingRookMoved = true;
            }
        } else if (move.toCol == 2) {
            board[move.toRow][3] = board[move.toRow][0];
            board[move.toRow][0] = '.';
            if (move.toRow == 7) {
                whiteQueenRookMoved = true;
            } else {
                blackQueenRookMoved = true;
            }
        }
    }
};

class ChessBot {
public:
    Move chooseMove(const GameState& state, int depth) const {
        auto legalMoves = state.generateLegalMoves();
        Move bestMove = legalMoves.front();
        int bestScore = std::numeric_limits<int>::min();

        for (const Move& move : legalMoves) {
            GameState next = state;
            next.applyMove(move);
            int score = minimax(next, depth - 1, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }

        return bestMove;
    }

private:
    static int materialValue(char piece) {
        switch (std::tolower(static_cast<unsigned char>(piece))) {
            case 'p':
                return 100;
            case 'n':
                return 320;
            case 'b':
                return 330;
            case 'r':
                return 500;
            case 'q':
                return 900;
            case 'k':
                return 20000;
            default:
                return 0;
        }
    }

    int evaluate(const GameState& state) const {
        int score = 0;
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                char piece = state.pieceAt(row, col);
                if (piece == '.') {
                    continue;
                }
                int value = materialValue(piece);
                if (std::isupper(static_cast<unsigned char>(piece))) {
                    score -= value;
                } else {
                    score += value;
                }

                if (piece == 'p' || piece == 'P') {
                    int advance = std::abs((piece == 'P' ? 6 : 1) - row) * 4;
                    score += piece == 'p' ? advance : -advance;
                }
            }
        }
        return score;
    }

    int minimax(const GameState& state, int depth, int alpha, int beta) const {
        auto legalMoves = state.generateLegalMoves();
        if (depth == 0 || legalMoves.empty()) {
            if (legalMoves.empty()) {
                if (state.isInCheck(state.isWhiteToMove())) {
                    return state.isWhiteToMove() ? 100000 + depth : -100000 - depth;
                }
                return 0;
            }
            return evaluate(state);
        }

        if (state.isWhiteToMove()) {
            int bestScore = std::numeric_limits<int>::max();
            for (const Move& move : legalMoves) {
                GameState next = state;
                next.applyMove(move);
                bestScore = std::min(bestScore, minimax(next, depth - 1, alpha, beta));
                beta = std::min(beta, bestScore);
                if (beta <= alpha) {
                    break;
                }
            }
            return bestScore;
        }

        int bestScore = std::numeric_limits<int>::min();
        for (const Move& move : legalMoves) {
            GameState next = state;
            next.applyMove(move);
            bestScore = std::max(bestScore, minimax(next, depth - 1, alpha, beta));
            alpha = std::max(alpha, bestScore);
            if (beta <= alpha) {
                break;
            }
        }
        return bestScore;
    }
};

static std::string normalizeInput(std::string input) {
    input.erase(
        std::remove_if(input.begin(), input.end(), [](unsigned char ch) {
            return std::isspace(ch);
        }),
        input.end()
    );

    for (char& ch : input) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return input;
}

static std::optional<Move> parseMoveInput(const std::string& input, const std::vector<Move>& legalMoves) {
    if (input.size() < 4) {
        return std::nullopt;
    }

    auto fileToCol = [](char file) -> int {
        return file >= 'a' && file <= 'h' ? file - 'a' : -1;
    };
    auto rankToRow = [](char rank) -> int {
        return rank >= '1' && rank <= '8' ? 8 - (rank - '0') : -1;
    };

    int fromCol = fileToCol(input[0]);
    int fromRow = rankToRow(input[1]);
    int toCol = fileToCol(input[2]);
    int toRow = rankToRow(input[3]);

    if (fromCol == -1 || fromRow == -1 || toCol == -1 || toRow == -1) {
        return std::nullopt;
    }

    for (const Move& move : legalMoves) {
        if (move.fromRow == fromRow &&
            move.fromCol == fromCol &&
            move.toRow == toRow &&
            move.toCol == toCol) {
            return move;
        }
    }

    return std::nullopt;
}

static void printInstructions() {
    std::cout << "Mac Terminal Chess\n";
    std::cout << "Play as White against the bot.\n";
    std::cout << "Enter moves like e2e4, g1f3, or e1g1 for castling.\n";
    std::cout << "Type 'quit' to exit.\n";
}

int main() {
    GameState state;
    ChessBot bot;

    printInstructions();

    while (true) {
        state.printBoard();
        auto legalMoves = state.generateLegalMoves();

        if (legalMoves.empty()) {
            if (state.isInCheck(state.isWhiteToMove())) {
                std::cout << (state.isWhiteToMove() ? "Checkmate. Black wins.\n" : "Checkmate. White wins.\n");
            } else {
                std::cout << "Stalemate.\n";
            }
            break;
        }

        if (state.isInCheck(state.isWhiteToMove())) {
            std::cout << "Check.\n";
        }

        if (state.isWhiteToMove()) {
            std::cout << "Your move: ";
            std::string input;
            std::getline(std::cin, input);
            input = normalizeInput(input);

            if (input == "quit" || input == "exit") {
                std::cout << "Goodbye.\n";
                break;
            }

            auto chosenMove = parseMoveInput(input, legalMoves);
            if (!chosenMove.has_value()) {
                std::cout << "Invalid move. Try again using format like e2e4.\n";
                continue;
            }

            state.applyMove(*chosenMove);
            continue;
        }

        std::cout << "Bot is thinking...\n";
        Move botMove = bot.chooseMove(state, 3);
        std::cout << "Bot plays " << GameState::moveToString(botMove) << "\n";
        state.applyMove(botMove);
    }

    return 0;
}
