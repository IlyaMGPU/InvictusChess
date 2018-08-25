/**************************************************/
/*  Invictus 2018						          */
/*  Edsel Apostol                                 */
/*  ed_apostol@yahoo.com                          */
/**************************************************/

#define USE_PEXT

#include <vector>
#ifdef USE_PEXT
#include <immintrin.h>
#endif
#include "typedefs.h"
#include "constants.h"
#include "attacks.h"
#include "bitutils.h"
#include "log.h"

namespace {
	const uint64_t BMagic[64] =
	{
		0x0048610528020080ULL, 0x00c4100212410004ULL, 0x0004180181002010ULL, 0x0004040188108502ULL,
		0x0012021008003040ULL, 0x0002900420228000ULL, 0x0080808410c00100ULL, 0x000600410c500622ULL,
		0x00c0056084140184ULL, 0x0080608816830050ULL, 0x00a010050200b0c0ULL, 0x0000510400800181ULL,
		0x0000431040064009ULL, 0x0000008820890a06ULL, 0x0050028488184008ULL, 0x00214a0104068200ULL,
		0x004090100c080081ULL, 0x000a002014012604ULL, 0x0020402409002200ULL, 0x008400c240128100ULL,
		0x0001000820084200ULL, 0x0024c02201101144ULL, 0x002401008088a800ULL, 0x0003001045009000ULL,
		0x0084200040981549ULL, 0x0001188120080100ULL, 0x0048050048044300ULL, 0x0008080000820012ULL,
		0x0001001181004003ULL, 0x0090038000445000ULL, 0x0010820800a21000ULL, 0x0044010108210110ULL,
		0x0090241008204e30ULL, 0x000c04204004c305ULL, 0x0080804303300400ULL, 0x00a0020080080080ULL,
		0x0000408020220200ULL, 0x0000c08200010100ULL, 0x0010008102022104ULL, 0x0008148118008140ULL,
		0x0008080414809028ULL, 0x0005031010004318ULL, 0x0000603048001008ULL, 0x0008012018000100ULL,
		0x0000202028802901ULL, 0x004011004b049180ULL, 0x0022240b42081400ULL, 0x00c4840c00400020ULL,
		0x0084009219204000ULL, 0x000080c802104000ULL, 0x0002602201100282ULL, 0x0002040821880020ULL,
		0x0002014008320080ULL, 0x0002082078208004ULL, 0x0009094800840082ULL, 0x0020080200b1a010ULL,
		0x0003440407051000ULL, 0x000000220e100440ULL, 0x00480220a4041204ULL, 0x00c1800011084800ULL,
		0x000008021020a200ULL, 0x0000414128092100ULL, 0x0000042002024200ULL, 0x0002081204004200ULL
	};

	const uint64_t RMagic[64] =
	{
		0x00800011400080a6ULL, 0x004000100120004eULL, 0x0080100008600082ULL, 0x0080080016500080ULL,
		0x0080040008000280ULL, 0x0080020005040080ULL, 0x0080108046000100ULL, 0x0080010000204080ULL,
		0x0010800424400082ULL, 0x00004002c8201000ULL, 0x000c802000100080ULL, 0x00810010002100b8ULL,
		0x00ca808014000800ULL, 0x0002002884900200ULL, 0x0042002148041200ULL, 0x00010000c200a100ULL,
		0x00008580004002a0ULL, 0x0020004001403008ULL, 0x0000820020411600ULL, 0x0002120021401a00ULL,
		0x0024808044010800ULL, 0x0022008100040080ULL, 0x00004400094a8810ULL, 0x0000020002814c21ULL,
		0x0011400280082080ULL, 0x004a050e002080c0ULL, 0x00101103002002c0ULL, 0x0025020900201000ULL,
		0x0001001100042800ULL, 0x0002008080022400ULL, 0x000830440021081aULL, 0x0080004200010084ULL,
		0x00008000c9002104ULL, 0x0090400081002900ULL, 0x0080220082004010ULL, 0x0001100101000820ULL,
		0x0000080011001500ULL, 0x0010020080800400ULL, 0x0034010224009048ULL, 0x0002208412000841ULL,
		0x000040008020800cULL, 0x001000c460094000ULL, 0x0020006101330040ULL, 0x0000a30010010028ULL,
		0x0004080004008080ULL, 0x0024000201004040ULL, 0x0000300802440041ULL, 0x00120400c08a0011ULL,
		0x0080006085004100ULL, 0x0028600040100040ULL, 0x00a0082110018080ULL, 0x0010184200221200ULL,
		0x0040080005001100ULL, 0x0004200440104801ULL, 0x0080800900220080ULL, 0x000a01140081c200ULL,
		0x0080044180110021ULL, 0x0008804001001225ULL, 0x00a00c4020010011ULL, 0x00001000a0050009ULL,
		0x0011001800021025ULL, 0x00c9000400620811ULL, 0x0032009001080224ULL, 0x001400810044086aULL
	};

	uint64_t KnightMoves[64];
	uint64_t KingMoves[64];
	uint64_t PawnCaps[2][64];
	uint64_t PawnMoves[2][64];
	uint64_t PawnMoves2[2][64];
	uint64_t RMagicAttacks[0x19000];
	uint64_t BMagicAttacks[0x1480];
	uint64_t RMagicMask[64];
	uint64_t BMagicMask[64];
	int RShift[64];
	int ROffset[64];
	int BShift[64];
	int BOffset[64];

	uint64_t slideAttacks(int sq, const std::vector<int>& D, uint64_t occ) {
		uint64_t att = 0;
		for (int d : D) {
			for (int p = sq, n = sq + d; n >= 0 && n < 64 && abs((n & 7) - (p & 7)) < 2; p = n, n += d) {
				att |= BitMask[n];
				if (BitMask[n] & occ) break;
			}
		}
		return att;
	}

	inline int sliderIndex(uint64_t occ, uint64_t mask, uint64_t magic, int shift) {
#ifdef USE_PEXT
		return _pext_u64(occ, mask);
#else
		return ((occ & mask) * magic) >> shift;
#endif
	}

	void initSliderTable(uint64_t atktable[], uint64_t mask[], const uint64_t magic[], int offset[], int shift[], const std::vector<int>& dir) {
		offset[0] = 0;
		for (int s = 0; s < 0x40; s++) {
			const uint64_t edges = ((Rank1BB | Rank8BB) & ~RankBB[sqRank(s)]) | ((FileABB | FileHBB) & ~FileBB[sqFile(s)]);
			mask[s] = slideAttacks(s, dir, 0) & ~edges;
			shift[s] = 64 - BitUtils::bitCnt(mask[s]);
			if (s < 63) offset[s + 1] = offset[s] + (1 << BitUtils::bitCnt(mask[s]));
			uint64_t occ = 0;
			do {
				atktable[offset[s] + sliderIndex(occ, mask[s], magic[s], shift[s])] = slideAttacks(s, dir, occ);
				occ = (occ - mask[s]) & mask[s];
			} while (occ);
		}
	}

	void initMovesTable(const std::vector<int>& D, uint64_t A[]) {
		for (int i = 0; i < 0x40; i++) {
			for (int j : D) {
				int n = i + j;
				if (n >= 0 && n < 64 && abs((n & 7) - (i & 7)) <= 2)
					A[i] |= BitMask[n];
			}
		}
	}
}

namespace Attacks {
	void initArr(void) {
		const std::vector<int> kingd = { -9, -1, 7, 8, 9, 1, -7, -8 };
		const std::vector<int> knightd = { -17, -10, 6, 15, 17, 10, -6, -15 };
		const std::vector<int> bishopd = { -9, 7, 9, -7 };
		const std::vector<int> rookd = { -1, 8, 1, -8 };
		const std::vector<int> wpawnd = { 8 };
		const std::vector<int> bpawnd = { -8 };
		const std::vector<int> wpawnc = { 7, 9 };
		const std::vector<int> bpawnc = { -7, -9 };
		const std::vector<int> wpawn2mov = { 16 };
		const std::vector<int> bpawn2mov = { -16 };

		initMovesTable(knightd, KnightMoves);
		initMovesTable(kingd, KingMoves);
		initMovesTable(wpawnd, PawnMoves[WHITE]);
		initMovesTable(bpawnd, PawnMoves[BLACK]);
		initMovesTable(wpawnc, PawnCaps[WHITE]);
		initMovesTable(bpawnc, PawnCaps[BLACK]);
		initMovesTable(wpawn2mov, PawnMoves2[WHITE]);
		initMovesTable(bpawn2mov, PawnMoves2[BLACK]);

		initSliderTable(RMagicAttacks, RMagicMask, RMagic, ROffset, RShift, rookd);
		initSliderTable(BMagicAttacks, BMagicMask, BMagic, BOffset, BShift, bishopd);
	}

	uint64_t pawnMovesBB(int from, uint64_t s) {
		return PawnMoves[s][from];
	}
	uint64_t pawnMoves2BB(int from, uint64_t s) {
		return PawnMoves2[s][from];
	}
	uint64_t pawnAttacksBB(int from, uint64_t s) {
		return PawnCaps[s][from];
	}
	uint64_t knightAttacksBB(int from, uint64_t occ) {
		return KnightMoves[from];
	}
	uint64_t bishopAttacksBB(int from, uint64_t occ) {
		return BMagicAttacks[BOffset[from] + sliderIndex(occ, BMagicMask[from], BMagic[from], BShift[from])];
	}
	uint64_t rookAttacksBB(int from, uint64_t occ) {
		return RMagicAttacks[ROffset[from] + sliderIndex(occ, RMagicMask[from], RMagic[from], RShift[from])];
	}
	uint64_t queenAttacksBB(int from, uint64_t occ) {
		return bishopAttacksBB(from, occ) | rookAttacksBB(from, occ);
	}
	uint64_t bishopAttacksBBX(int from, uint64_t occ) {
		return bishopAttacksBB(from, occ & ~(bishopAttacksBB(from, occ) & occ));
	}
	uint64_t rookAttacksBBX(int from, uint64_t occ) {
		return rookAttacksBB(from, occ & ~(rookAttacksBB(from, occ) & occ));
	}
	uint64_t kingAttacksBB(int from, uint64_t occ) {
		return KingMoves[from];
	}
	uint64_t knightMovesBB(int from) {
		return KnightMoves[from];
	}
	uint64_t kingMovesBB(int from) {
		return KingMoves[from];
	}
}