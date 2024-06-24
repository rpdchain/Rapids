// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "chain.h"
#include "chainparams.h"
#include "main.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"
#include "spork.h"

#include <math.h>

const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
        pindex = pindex->pprev;
    return pindex;
}

static arith_uint256 GetTargetLimit(int64_t nTime, bool fProofOfStake, const Consensus::Params& params)
{
    uint256 nLimit;

    if (fProofOfStake) {
        nLimit = params.posLimit;
    } else {
        nLimit = params.powLimit;
    }

    return UintToArith256(nLimit);
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader* pblock)
{
    const Consensus::Params& params = Params().GetConsensus();

    bool fProofOfStake = pindexLast->IsProofOfStake();
    unsigned int nTargetLimit = UintToArith256(fProofOfStake ? params.posLimit : params.powLimit).GetCompact();

    if (pindexLast->nHeight + 1 > params.nLwmaProtocolHeight) {
        return CalculateNextWorkRequired(pindexLast, fProofOfStake, params);
    } else {
        return nTargetLimit;
    }

    // Genesis block
    if (pindexLast == NULL) {
        return nTargetLimit;
    }

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL) {
        return nTargetLimit; // first block
    }

    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL) {
        return nTargetLimit; // second block
    }

    return CalculateNextWorkRequired(pindexPrev, pindexPrevPrev->GetBlockTime(), params);
}

unsigned int LwmaCalculateNextWorkRequired(const CBlockIndex* pindexLast, bool fProofOfStake, const Consensus::Params& params)
{
    /*
    int64_t T;
    int64_t N;


    if (chainActive.Height() <= params.nTargetForkHeightV2) {        // Prior block 20k
        T = params.nPosTargetSpacing;                                // 15 second block time
        N = params.lwmaAveragingWindow;                              // 30 minute retarget window  1800 * (1800 + 1) * 15 / 2 = 24,313,500 seconds base retarget time = 40 week retarget time
    } else if (chainActive.Height() <= params.nTargetForkHeightV3) { // After block 20k
        T = params.nPosTargetSpacingV2;                              // 20 second block time
        N = params.lwmaAveragingWindowV2;                            // 3 minute retarget window   180 * (180 + 1) * 15 / 2 = 325,800 seconds base retarget time = to long to figure out
    } else {                                                         // After block 45k
        T = params.nPosTargetSpacingV3;                              // 15 second block time
        N = params.lwmaAveragingWindowV3;                            // 8 second retarget window   8 * (8 + 1) * 15 / 2 = 540 seconds base retarget time
    }
    */

    int64_t T = params.nPosTargetSpacing;
    int64_t N = params.lwmaAveragingWindow;

    const int64_t k = N * (N + 1) * T / 2;
    const int64_t height = pindexLast->nHeight;
    const arith_uint256 posLimit = UintToArith256(params.posLimit);

    if (height < N) {
        return posLimit.GetCompact();
    }

    arith_uint256 sumTarget, nextTarget;
    int64_t thisTimestamp, previousTimestamp;
    int64_t t = 0, j = 0;

    // Uncomment next 2 lines to use LWMA-3 jump rule.
    arith_uint256 previousTarget = 0;
    int64_t sumLast3Solvetimes = 0;

    const CBlockIndex* blockPreviousTimestamp = pindexLast->GetAncestor(height - N);
    previousTimestamp = blockPreviousTimestamp->GetBlockTime();

    // Loop through N most recent blocks.
    for (int64_t i = height - N + 1; i <= height; i++) {
        const CBlockIndex* block = pindexLast->GetAncestor(i);
        thisTimestamp = (block->GetBlockTime() > previousTimestamp) ?
                            block->GetBlockTime() :
                            previousTimestamp + 1;

        int64_t solvetime = std::min(6 * T, thisTimestamp - previousTimestamp);
        previousTimestamp = thisTimestamp;

        j++;
        t += solvetime * j; // Weighted solvetime sum.
        arith_uint256 target;
        target.SetCompact(block->nBits);
        sumTarget += target / (k * N); 

        // Uncomment next 2 lines to use LWMA-3.
        if (i > height - 3) { sumLast3Solvetimes  += solvetime; }
        if (i == height) { previousTarget = target.SetCompact(block->nBits); }
    }
    nextTarget = t * sumTarget;

    // Uncomment the following to use LWMA-3.
    // This is a "memory-less" jump in difficulty approximately 2x normal
     if (sumLast3Solvetimes < (8 * T) / 10) { nextTarget = (previousTarget*100)/(100+(N*26)/200); }

    if (nextTarget > posLimit) {
        nextTarget = posLimit;
    }

    return nextTarget.GetCompact();
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    /*
    int64_t nTargetSpacing;
    int64_t nInterval;

    if (chainActive.Height() <= params.nTargetForkHeightV2) {
        nTargetSpacing = params.nPosTargetSpacing;
        nInterval = params.lwmaAveragingWindow / params.nPosTargetSpacing;
    } else if (chainActive.Height() <= params.nTargetForkHeightV3) {
        nTargetSpacing = params.nPosTargetSpacingV2;
        nInterval = params.lwmaAveragingWindowV2 / params.nPosTargetSpacingV2;
    } else {
        nTargetSpacing = params.nPosTargetSpacingV3;
        nInterval = params.lwmaAveragingWindowV3;
    }
    */

    int64_t nTargetSpacing = params.nPosTargetSpacing;
    int64_t nInterval = params.lwmaAveragingWindow;

    bool fProofOfStake = pindexLast->IsProofOfStake();
    if (!fProofOfStake && params.fPowAllowMinDifficultyBlocks)
        return pindexLast->nBits;
    int64_t nActualSpacing = pindexLast->GetBlockTime() - nFirstBlockTime;
    // int64_t nTargetSpacing = params.nPosTargetSpacing;
    
    // Limit adjustment step
    if (nActualSpacing < 0) {
        nActualSpacing = nTargetSpacing;
    }
    if (nActualSpacing > nTargetSpacing * 10) {
        nActualSpacing = nTargetSpacing * 10;
    }
    // retarget with exponential moving toward target spacing
    const arith_uint256 bnTargetLimit = GetTargetLimit(pindexLast->GetBlockTime(), fProofOfStake, params);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    //int64_t nInterval = params.lwmaAveragingWindow / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);
    if (bnNew <= 0 || bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;
    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    if (Params().IsRegTestNet()) return true;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    //if (Params().IsTestNet()) return true;

    // Check range
    if (fNegative || bnTarget.IsNull() || fOverflow || bnTarget > UintToArith256(Params().GetConsensus().powLimit))
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    //if (UintToArith256(hash) > bnTarget)
       // return error("CheckProofOfWork() : hash doesn't match nBits");

    return true;
}

arith_uint256 GetBlockProof(const CBlockIndex& block)
{
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget.IsNull())
        return ARITH_UINT256_ZERO;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}
