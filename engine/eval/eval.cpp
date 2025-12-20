//
// Created by Marvin Becker on 16.12.25.
//

#include "eval.h"

#include "../core/board.h"
#include "../core/piece.h"
#include "psqt_tables.h"

int eval::evaluate(const Board& board)
{
    int mg[2] = {};
    int eg[2] = {};
    int game_phase = 0;
    for (int i = 0; i < 64; i++)
    {
        Piece piece = board[i];
        // Calculate game phase for 'Tampered Eval'.
        game_phase += piece.get_game_phase_value();
        // Add PSQT values + material value.
        switch (piece.piece_type_)
        {
        case WP:
            mg[0] += MG_PAWN_TABLE[i ^ 56] + piece.get_material_value(false);
            eg[0] += EG_PAWN_TABLE[i ^ 56] + piece.get_material_value(true);
            break;
        case WN:
            mg[0] += MG_KNIGHT_TABLE[i ^ 56] + piece.get_material_value(false);
            eg[0] += EG_KNIGHT_TABLE[i ^ 56] + piece.get_material_value(true);
            break;
        case WB:
            mg[0] += MG_BISHOP_TABLE[i ^ 56] + piece.get_material_value(false);
            eg[0] += EG_BISHOP_TABLE[i ^ 56] + piece.get_material_value(true);
            break;
        case WR:
            mg[0] += MG_ROOK_TABLE[i ^ 56] + piece.get_material_value(false);
            eg[0] += EG_ROOK_TABLE[i ^ 56] + piece.get_material_value(true);
            break;
        case WQ:
            mg[0] += MG_QUEEN_TABLE[i ^ 56] + piece.get_material_value(false);
            eg[0] += EG_QUEEN_TABLE[i ^ 56] + piece.get_material_value(true);
            break;
        case WK:
            mg[0] += MG_KING_TABLE[i ^ 56] + piece.get_material_value(false);
            eg[0] += EG_KING_TABLE[i ^ 56] + piece.get_material_value(true);
            break;
        case BP:
            mg[1] += MG_PAWN_TABLE[i] + piece.get_material_value(false);
            eg[1] += EG_PAWN_TABLE[i] + piece.get_material_value(true);
            break;
        case BN:
            mg[1] += MG_KNIGHT_TABLE[i] + piece.get_material_value(false);
            eg[1] += EG_KNIGHT_TABLE[i] + piece.get_material_value(true);
            break;
        case BB:
            mg[1] += MG_BISHOP_TABLE[i] + piece.get_material_value(false);
            eg[1] += EG_BISHOP_TABLE[i] + piece.get_material_value(true);
            break;
        case BR:
            mg[1] += MG_ROOK_TABLE[i] + piece.get_material_value(false);
            eg[1] += EG_ROOK_TABLE[i] + piece.get_material_value(true);
            break;
        case BQ:
            mg[1] += MG_QUEEN_TABLE[i] + piece.get_material_value(false);
            eg[1] += EG_QUEEN_TABLE[i] + piece.get_material_value(true);
            break;
        case BK:
            mg[1] += MG_KING_TABLE[i] + piece.get_material_value(false);
            eg[1] += EG_KING_TABLE[i] + piece.get_material_value(true);
            break;
        case EMPTY:
            break;
        }
    }

    // Calculate PSQT and game phase with tampered eval
    // (https://www.chessprogramming.org/PeSTO's_Evaluation_Function) (MODIFIED by
    // MARVIN).
    const int MG_SCORE = mg[board.player_ != WHITE] - mg[board.player_ == WHITE];
    const int EG_SCORE = eg[board.player_ != WHITE] - eg[board.player_ == WHITE];
    // Calculate the game phase.
    int mgPhase = game_phase;

    if (mgPhase > 24)
        mgPhase = 24;

    const int EG_PHASE = 24 - mgPhase;

    const int EVALUATION = (((MG_SCORE * mgPhase) + (EG_SCORE * EG_PHASE)) / 24);

    // Give a bonus to side to move (TEMPO).
    return (EVALUATION + 20);
}
