#include "./chess_bot.h"

void ChessBot::reset_tt() { tt_array->fill(Move{}); }

int ChessBot::eval(Board& board) {
  int mg[2] = {};
  int eg[2] = {};
  int gamePhase = 0;
  for (int i = 0; i < 64; i++) {
    Piece piece = board[i];
    // Calculate game phase for 'Tampered Eval'.
    gamePhase += piece.get_game_phase_value();
    // Add PSQT values + material value.
    switch (piece.piece_type) {
      case WP:
        mg[0] += mg_pawn_table[i ^ 56] + piece.get_material_value(false);
        eg[0] += eg_pawn_table[i ^ 56] + piece.get_material_value(true);
        break;
      case WN:
        mg[0] += mg_knight_table[i ^ 56] + piece.get_material_value(false);
        eg[0] += eg_knight_table[i ^ 56] + piece.get_material_value(true);
        break;
      case WB:
        mg[0] += mg_bishop_table[i ^ 56] + piece.get_material_value(false);
        eg[0] += eg_bishop_table[i ^ 56] + piece.get_material_value(true);
        break;
      case WR:
        mg[0] += mg_rook_table[i ^ 56] + piece.get_material_value(false);
        eg[0] += eg_rook_table[i ^ 56] + piece.get_material_value(true);
        break;
      case WQ:
        mg[0] += mg_queen_table[i ^ 56] + piece.get_material_value(false);
        eg[0] += eg_queen_table[i ^ 56] + piece.get_material_value(true);
        break;
      case WK:
        mg[0] += mg_king_table[i ^ 56] + piece.get_material_value(false);
        eg[0] += eg_king_table[i ^ 56] + piece.get_material_value(true);
        break;
      case BP:
        mg[1] += mg_pawn_table[i] + piece.get_material_value(false);
        eg[1] += eg_pawn_table[i] + piece.get_material_value(true);
        break;
      case BN:
        mg[1] += mg_knight_table[i] + piece.get_material_value(false);
        eg[1] += eg_knight_table[i] + piece.get_material_value(true);
        break;
      case BB:
        mg[1] += mg_bishop_table[i] + piece.get_material_value(false);
        eg[1] += eg_bishop_table[i] + piece.get_material_value(true);
        break;
      case BR:
        mg[1] += mg_rook_table[i] + piece.get_material_value(false);
        eg[1] += eg_rook_table[i] + piece.get_material_value(true);
        break;
      case BQ:
        mg[1] += mg_queen_table[i] + piece.get_material_value(false);
        eg[1] += eg_queen_table[i] + piece.get_material_value(true);
        break;
      case BK:
        mg[1] += mg_king_table[i] + piece.get_material_value(false);
        eg[1] += eg_king_table[i] + piece.get_material_value(true);
        break;
      case EMPTY:
        break;
    }
  }

  // Calculate PSQT and game phase with tampered eval (https://www.chessprogramming.org/PeSTO's_Evaluation_Function)
  // (MODIFIED by MARVIN).
  int mgScore = mg[board.player != WHITE] - mg[board.player == WHITE];
  int egScore = eg[board.player != WHITE] - eg[board.player == WHITE];
  // Calculate the game phase.
  int mgPhase = gamePhase;
  if (mgPhase > 24) mgPhase = 24;
  int egPhase = 24 - mgPhase;

  int evaluation = (((mgScore * mgPhase) + (egScore * egPhase)) / 24);

  // Give a bonus to side to move (TEMPO).
  return (evaluation + 20);
}

Move ChessBot::generate_best_next_move(Board& board, const int time_constraint) {
  // Set the time to now.
  iterative_time_point = std::chrono::high_resolution_clock::now();
  iterative_time_constraint = time_constraint;
  Move bestMove{};
  // Run until the timeout returns true.
  for (int i = 1;; i++) {
    // Search the best move for depth i.
    const Move move = search_best_next_move(board, i);
    if (is_time_up()) {
      // If the time is up, break the loop and don't apply the not fully evaluated move.
      break;
    }
    // Set the move if the search is fully done.
    bestMove = move;
  }
  // Return the best move calculated at depth i.
  return bestMove;
}

int ChessBot::search(Board& board, int depth, int alpha, int beta, int ply, Move& best_move) {
  if (depth <= 0) {
    // If depth reached, run qsearch so you don't sacrifice your piece and eval the board.
    return quiescence_search(board, alpha, beta);
  }

  // If time is up kill the prozess by returning anything.
  if (is_time_up()) {
    return -INT_MAX;
  }

  // Get all possible moves.
  auto moveList = moveGenUtils::get_all_pseudo_legal_moves(board, board.player == WHITE);
  // Sort so the best moves are first.
  moveList.sort_move_list_mvv_lva((*tt_array)[board.get_hash() % tt_size]);

  int legalMoves = 0;

  // First best score should be the worst.
  int bestScore = -INT_MAX;

  for (Move& move : moveList) {
    int score;

    // Make every move and gather the value of the opponent.
    if (board.make_move(move)) {
      score = -search(board, depth - 1, -beta, -alpha, ply + 1, best_move);
      legalMoves++;
      board.pop_last_move();
    } else {
      continue;
    }

    // Set best score if the current one is less.
    if (score > bestScore) {
      bestScore = score;
      // Add the best move for a position.
      (*tt_array)[board.get_hash() % tt_size] = move;
      if (ply == 0) {
        // Set best move if it is the root.
        best_move = move;
      }
    }

    // ALPHA BETA PRUNING
    if (bestScore > alpha) {
      alpha = bestScore;
    }

    if (alpha >= beta) {
      break;  // beta kill
    }
  }

  // If no legal moves
  if (legalMoves == 0) {
    if (board.is_king_in_check(board.player == WHITE)) {
      return -INT_MAX + ply;  // checkmate
    } else {
      return 0;  // stalemate
    }
  }

  return bestScore;
}

int ChessBot::quiescence_search(Board& board, int alpha, int beta) {
  // Eval the position.
  int stand_pat = eval(board);

  if (stand_pat >= beta) {
    return beta;
  }
  if (alpha < stand_pat) {
    alpha = stand_pat;
  }

  // Get all possible moves.
  auto moveList = moveGenUtils::get_all_pseudo_legal_moves(board, board.player == WHITE);
  moveList.sort_move_list_mvv_lva((*tt_array)[board.get_hash() % tt_size]);

  // First best score should be the worst.
  int bestScore = stand_pat;

  for (Move& move : moveList) {
    int score;

    // Filter the normal moves so I only have captures.
    if (move.captured_piece.piece_type == EMPTY && move.move_type != EN_PASSANT) {
      continue;
    }

    // Make every move and gather the value of the opponent.
    if (board.make_move(move)) {
      score = -quiescence_search(board, -beta, -alpha);
      board.pop_last_move();
    } else {
      continue;
    }

    // Update best score.
    bestScore = std::max(bestScore, score);
    // if score is less than alpha continue.
    if (score <= alpha) continue;
    // set the score
    alpha = score;
    if (score >= beta) {
      break;
    }
  }

  return bestScore;
}

Move ChessBot::search_best_next_move(Board& board, int depth) {
  Move move;
  search(board, depth, -INT_MAX, INT_MAX, 0, move);
  return move;
}
