// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (   0, uint256("0x563ac70cc2642286ad8463559011621fc4debe7ab2525900f74d079fc73cb5f2"))
        (9649, uint256("0x76712bc630c81d539ca51d410784af8b0ad9034867a8a4db12e8f0f0c0f39c1c"))
        (20000, uint256("0x2bbee592fa2f3738cad266d038f725e0d2ba7edf1b80380fa39608f523a31404"))
        (30000, uint256("0x1c3fee4059cf4147b4e234937f8292304f92eca8d7f34338039e97937d1211f3"))
        (40000, uint256("0x65585e9d874db1b9c4d02e5a3ffaa6275efb6c14c731f05799b0485ef1f47919"))
        (50000, uint256("0xe25b98e32bfa15a9729e8e988021df1fea31f28e137fcc5b9ff610668aab0a9a"))
        (60000, uint256("0xd5ec242db805d2cefb7dcca8aa2888cc20b802f4819c9a4d488a66b0032a925b"))
        (70000, uint256("0xc34628d3939502c31b9173a452e7c2af31fe2c72d193e9bac26ce7356d0af2d7"))
        (80000, uint256("0xb6dc848ecd9c68a86536b09e068919a87cad69bf29b2378531486941caade839"))
        (90000, uint256("0x0ca9b832934f5afeff66bf22f48bd7c09cba227b25d33347df704c43788f01db"))
        (100000, uint256("0x4aab6fc1a528d587a8357f62ce9ec8a84e7990b486389d352c0eb0c1652e6ded"))
        (150000, uint256("0xb1391e2d3f10d596d715d49add86425ac5d4ec82dc3aff88bb230f0c8d5ef76f"))
        (196177, uint256("0x71d89b625667c8f4f6b6c6a70ca68fa8143dda941f896273794e821923b0dd57"))
	;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1489231307, // * UNIX timestamp of last checkpoint block
        23062,    // * total number of transactions between genesis and last checkpoint
                  //   (the tx=... number in the SetBestChain debug.log lines)
        576.0     // * estimated number of transactions per day after checkpoint
    };


    const CCheckpointData &Checkpoints() {

            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
