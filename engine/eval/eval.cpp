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
    int gamePhase = 0;
    for (int i = 0; i < 64; i++)
    {
        Piece piece = board[i];
        // Calculate game phase for 'Tampered Eval'.
        gamePhase += piece.get_game_phase_value();
        // Add PSQT values + material value.
        switch (piece.piece_type)
        {
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

    // Calculate PSQT and game phase with tampered eval
    // (https://www.chessprogramming.org/PeSTO's_Evaluation_Function) (MODIFIED by
    // MARVIN).
    const int MG_SCORE = mg[board.player != WHITE] - mg[board.player == WHITE];
    const int EG_SCORE = eg[board.player != WHITE] - eg[board.player == WHITE];
    // Calculate the game phase.
    int mgPhase = gamePhase;
    if (mgPhase > 24)
        mgPhase = 24;
    const int EG_PHASE = 24 - mgPhase;

    const int EVALUATION = (((MG_SCORE * mgPhase) + (EG_SCORE * EG_PHASE)) / 24);

    // Give a bonus to side to move (TEMPO).
    return (EVALUATION + 20);
}
