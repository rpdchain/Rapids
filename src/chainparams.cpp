// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2020 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "chainparamsseeds.h"
#include "consensus/merkle.h"
#include "util.h"
#include "spork.h"
#include "utilstrencodings.h"

#include <boost/assign/list_of.hpp>

#include <assert.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.nVersion = nVersion;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of the genesis coinbase cannot
 * be spent as it did not originally exist in the database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "RPD New Chain";
    const CScript genesisOutputScript = CScript() << ParseHex("04f51fa7f2cf12177576b4294618ded175db33f3c644b68e3fb66f59ea49e02f1eae1afcdb000481226685708661abb4ce72824958ef23994a086d07fff8e1e7d1") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void GenesisGeneratorV2(CBlock genesis)
{
    // This is used inorder to mine the genesis block.
    // Once found, we can use the nonce and block hash found to create a valid genesis block.
    // To use this comment out the bellow and change the nGenesisTime here to the current UnixTimeStamp
    //
    //  genesis = CreateGenesisBlock(1626521690, 1187313, 0x1e0ffff0, 1, 0 * COIN);
    //  consensus.hashGenesisBlock = genesis.GetHash();
    //  assert(consensus.hashGenesisBlock == uint256S("0x000003053360edf16eea7c0f25026dc55c511b3fc8fdbbbb986012359273b5ca"));
    //  assert(genesis.hashMerkleRoot == uint256S("0xe980eec274480a0309fa533f5c35269f402c1ba5a4af59acc5585ae0d0c44802"));
    //
    // Now add above the lines you just commented out and recompile the source and launch the daemon to generate a new genesis.
    //
    //  GenesisGeneratorV2(genesis)
    //
    // /////////////////////////////////////////////////////////////////

    uint32_t nGenesisTime = 1679198507;

    arith_uint256 test;
    uint256 hashGenesisBlock;
    bool fNegative;
    bool fOverflow;
    test.SetCompact(0x1e0ffff0, &fNegative, &fOverflow);
    std::cout << "Test threshold: " << test.GetHex() << "\n\n";

    int genesisNonce = 0;
    uint256 TempHashHolding = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
    uint256 BestBlockHash = uint256S("0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    for (int i = 0; i < 40000000; i++) {
        genesis = CreateGenesisBlock(nGenesisTime, i, 0x1e0ffff0, 1, 0 * COIN);
        //genesis.hashPrevBlock = TempHashHolding;
        hashGenesisBlock = genesis.GetHash();

        arith_uint256 BestBlockHashArith = UintToArith256(BestBlockHash);
        if (UintToArith256(hashGenesisBlock) < BestBlockHashArith) {
            BestBlockHash = hashGenesisBlock;
            std::cout << BestBlockHash.GetHex() << " Nonce: " << i << "\n";
            std::cout << "   PrevBlockHash: " << genesis.hashPrevBlock.GetHex() << "\n";
        }

        TempHashHolding = hashGenesisBlock;

        if (BestBlockHashArith < test) {
            genesisNonce = i - 1;
            break;
        }
        //std::cout << consensus.hashGenesisBlock.GetHex() << "\n";
    }
    std::cout << "\n";
    std::cout << "\n";
    std::cout << "\n";

    std::cout << "time: " << genesis.nTime << std::endl;
    std::cout << "hashGenesisBlock to 0x" << BestBlockHash.GetHex() << std::endl;
    std::cout << "Genesis Nonce to " << genesisNonce << std::endl;
    std::cout << "Genesis Merkle 0x" << genesis.hashMerkleRoot.GetHex() << std::endl;

    exit(0);

    // /////////////////////////////////////////////////////////////////
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
static Checkpoints::MapCheckpoints mapCheckpoints =
    boost::assign::map_list_of
	(0, uint256S("0x00000b444395e32c86230466900dba6483f9175cd385c1aa4456651a848900ed"))
    ;

static const Checkpoints::CCheckpointData data = {
    &mapCheckpoints,
    1679198507, // * UNIX timestamp of last checkpoint block
    2003954,    // * total number of transactions between genesis and last checkpoint
                //   (the tx=... number in the UpdateTip debug.log lines)
    2000        // * estimated number of transactions per day after checkpoint
};

static Checkpoints::MapCheckpoints mapCheckpointsTestnet =
    boost::assign::map_list_of
    (0, uint256S("0x001"))
 ;

static const Checkpoints::CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    0,
    0,
    0
};

static Checkpoints::MapCheckpoints mapCheckpointsRegtest =
    boost::assign::map_list_of(0, uint256S("0x001"));
static const Checkpoints::CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1454124731,
    0,
    100};

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        networkID = CBaseChainParams::MAIN;
        strNetworkID = "main";

        genesis = CreateGenesisBlock(1679198507, 8020, 0x1e0ffff0, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00000b444395e32c86230466900dba6483f9175cd385c1aa4456651a848900ed"));
        assert(genesis.hashMerkleRoot == uint256S("0xe980eec274480a0309fa533f5c35269f402c1ba5a4af59acc5585ae0d0c44802"));

        //GenesisGeneratorV2(genesis);

        consensus.fPowAllowMinDifficultyBlocks = true;

        consensus.powLimit   = ~UINT256_ZERO >> 2; 
        consensus.posLimit   = ~UINT256_ZERO >> 24;
        consensus.posLimitV2 = ~UINT256_ZERO >> 20;
        consensus.nBudgetCycleBlocks = 43200;       // approx. 1 every 30 days
        consensus.nBudgetFeeConfirmations = 6;      // Number of confirmations for the finalization fee
        consensus.nCoinbaseMaturity = 10;

        consensus.nFutureTimeDriftPoW = 7200;
        consensus.nFutureTimeDriftPoS = 180;
        consensus.nPoolMaxTransactions = 3;

        consensus.nSingleThreadMaxTxesSize = 500;
        consensus.nMaxAmountLoadedRecords = 500;

        // Block reward related params
        consensus.nMaxMoneyOut = 35000000 * COIN;     // 35m
        consensus.nHalvingInterval = 500000;          // Halve block reward every 500k blocks
        consensus.nPreMine = 10000000 * COIN;         // 10m premined coins
        consensus.nBlockReward = 0.445875 * COIN;     // Initial Block reward
        consensus.nMasternodeReward = 0.7;            // 70%
        consensus.nDevReward = 0.1;                   // 10%
        consensus.nStakeReward = 0.2;                 // 20%

        consensus.nProposalEstablishmentTime = 60 * 60 * 24;    // must be at least a day old to make it into a budget
        consensus.nMaxProposalPayments = 6;

        //REMOVE BEFORE GOING LIVE THIS IS FORKING HEIGHTS FOR TESTING
        consensus.nTargetForkHeightV2 = 20000; // Block 20k
        consensus.nTargetForkHeightV3 = 50000; // Block 50k

        //Pow phase
        consensus.nPowTargetTimespan = 30 * 60;    // 30mins
        consensus.nPowTargetSpacing = 15;        // 15sec
        //consensus.nPowTargetTimespanV2 = 3 * 60;   // 3 mins
        //consensus.nPowTargetSpacingV2 = 20;        // 20sec

        //Pos phase
        consensus.nPosTargetSpacing = 15;        // 15 sec block time
        consensus.lwmaAveragingWindow = 8;       // 8 second retarget 
        consensus.nPosTargetSpacingV2 = std::numeric_limits<int>::max();
        consensus.lwmaAveragingWindowV2 = std::numeric_limits<int>::max();
        consensus.nPosTargetSpacingV3 = std::numeric_limits<int>::max();
        consensus.lwmaAveragingWindowV3 = std::numeric_limits<int>::max();     

        consensus.nTimeSlotLength = 15;         // 15sec
        consensus.nStakeMinAge = 10 * 60;       // 10min
        consensus.nStakeMinDepth = 60;          // 60 blocks        
        
        // spork keys
        consensus.strSporkPubKey = "02b1f75be7556ab096f2fd94378c153e6164435e76d29393e1e6dc422edb8c9135";
        // Address RqA8AmLpEL9MKUHn9m59W5ptruS8Z26WJq
        // PrivKey 7tNyh74rJpNwVRhipzzCBW9FAFiY17UBpydV9LUVcGjrZmb3Thac
        consensus.strSporkPubKeyOld = "02f8564bbb59972e10fb297f55f401c6743dba1a0f864e526a25476984717f5856";
        // 7uS7r5smJVipqE1Fv7YYDWmENHKnR56TTUdqVFN9ixaUnzcmQYDp
        consensus.nTime_EnforceNewSporkKey = 1675572203; //Wed Nov 30 2022 03:40:09 GMT-0500 (Eastern Standard Time)
        consensus.nTime_RejectOldSporkKey = 1675572200;  //Wed Nov 30 2022 03:35:09 GMT-0500 (Eastern Standard Time)

        // height-based activations
        consensus.height_last_PoW = 199;
        consensus.nLwmaProtocolHeight = 200;
        consensus.height_governance = 500000;

        consensus.height_last_ZC_AccumCheckpoint = std::numeric_limits<int>::max();
        consensus.height_last_ZC_WrappedSerials = std::numeric_limits<int>::max();
        consensus.height_start_InvalidUTXOsCheck = std::numeric_limits<int>::max();
        consensus.height_start_ZC_InvalidSerials = std::numeric_limits<int>::max();
        consensus.height_start_ZC_SerialRangeCheck = std::numeric_limits<int>::max();
        consensus.height_ZC_RecalcAccumulators = std::numeric_limits<int>::max();

        // Zerocoin-related params
        consensus.ZC_Modulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
                "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
                "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
                "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
                "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
                "31438167899885040445364023527381951378636564391212010397122822120720357";
        consensus.ZC_MaxPublicSpendsPerTx = 637;    // Assume about 220 bytes each input
        consensus.ZC_MaxSpendsPerTx = 7;            // Assume about 20kb each input
        consensus.ZC_MinMintConfirmations = 20;
        consensus.ZC_MinMintFee = 1 * CENT;
        consensus.ZC_MinStakeDepth = 200;
        consensus.ZC_TimeStart = std::numeric_limits<int>::max();
        consensus.ZC_WrappedSerialsSupply = 0;

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
		consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 200;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 201;
        consensus.vUpgrades[Consensus::UPGRADE_ZC].nActivationHeight            = std::numeric_limits<int>::max();
        consensus.vUpgrades[Consensus::UPGRADE_ZC_V2].nActivationHeight         = std::numeric_limits<int>::max();
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         = 1;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_PUBLIC].nActivationHeight     = std::numeric_limits<int>::max();
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].nActivationHeight          = 201;
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].nActivationHeight          = 202;
        consensus.vUpgrades[Consensus::UPGRADE_V5_DUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.vUpgrades[Consensus::UPGRADE_ZC].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_ZC_V2].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_ZC_PUBLIC].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
	    pchMessageStart[0] = 0x61;
	    pchMessageStart[1] = 0xa2;
	    pchMessageStart[2] = 0xf5;
	    pchMessageStart[3] = 0xcb;
        nDefaultPort = 1591;

        // Note that of those with the service bits flag, most only support a subset of possible options
        vSeeds.push_back(CDNSSeedData("146.190.13.2409 ", "146.190.13.240 ", true));
        vSeeds.push_back(CDNSSeedData("24.199.68.190 ", "24.199.68.190 ", true));
        vSeeds.push_back(CDNSSeedData("24.199.68.69", "24.199.68.69", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 61);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 6);
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 28);     // starting with 'C'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 46);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();
        // BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0xde).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        // Sapling
        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "ps";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "pviews";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "pivks";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]         = "p-secret-spending-key-main";

        strMasterAddress = "RgKH1GPkTzkrqSWyycW7YqxF5yhFCrGc7W";
        //Master pubkey 76a9145470d8063798c610d935fb94105fefaf56dd4fd288ac
        //Master privkey 7vca8MkNMpASidNYiK5V9R1Cx7KdN5RqUqHUGx6EmxLCLiKcYWkF

        foundationFundAddress = "Rw7E4wNwr6AikK3TaZYQS9H79WMqU6z1WP";
        //foundationFundAddress pubkey 76a914f6b349a29c8d13b9a1ff2bba7a735a92ae6a9b0888ac
        //foundationFundAddress privkey 7r2ieUJGrbQ781U9qwbwnCcm7JJX7M77HeUSGF6zE9Ky6paNDmhe

        strFeeAddress = "Ram7H6Avt4WY1mL1avcXbjBVYHnKojC6Ai";
        //Fee pubkey 76a9141782c8c898d1a5d29fd120202903bcde86e706c188ac
        //Fee privkey 7rEfxZUajg7ZVk1rQNLFBodU4H4ea53y3wxHA2GcqikGDiNoGBAt

        tokenFixedFee = 1 * COIN;
        tokenManagedFee = 1 * COIN;
        tokenVariableFee = 1 * COIN;
        tokenUsernameFee = 1 * COIN;
        tokenSubFee = 0 * COIN;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return data;
    }

};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams
{
public:
    CTestNetParams()
    {
        networkID = CBaseChainParams::TESTNET;
        strNetworkID = "test";

        genesis = CreateGenesisBlock(1674751352, 1047466, 0x1e0ffff0, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000006bc3b91b1f97edfafe72c6226f5bd337087f2e4b0ca636ff9b0cfc12e7e"));
        assert(genesis.hashMerkleRoot == uint256S("0xe980eec274480a0309fa533f5c35269f402c1ba5a4af59acc5585ae0d0c44802"));

        consensus.powLimit = ~UINT256_ZERO >> 2;
        consensus.posLimit = ~UINT256_ZERO >> 24;
        consensus.posLimitV2 = ~UINT256_ZERO >> 20;

        consensus.fPowAllowMinDifficultyBlocks = true;
        
        consensus.nProposalEstablishmentTime = 60;      // at least minute old to make it into a budget
        consensus.nBudgetCycleBlocks = 300;             // every 5 minutes (60 blocks)
        consensus.nBudgetFeeConfirmations = 3;          // only 3-blocks window for finalization on testnet
        consensus.nMaxProposalPayments = 6;

        // Block reward related params
        consensus.nMaxMoneyOut = 35000000000 * COIN;
        consensus.nHalvingInterval = 500000;  // Halve block reward every 500k blocks
        consensus.nPreMine = 10000000 * COIN; // 10m premined coins
        consensus.nBlockReward = 0.17835 * COIN;
        consensus.nMasternodeReward = 0.1; // 10%
        consensus.nDevReward = 0.7;        // 70%
        consensus.nStakeReward = 0.2;      // 20%

        consensus.nFutureTimeDriftPoW = 7200;
        consensus.nFutureTimeDriftPoS = 180;
        consensus.nPoolMaxTransactions = 2;

        consensus.nCoinbaseMaturity = 10;

        consensus.nPowTargetTimespan = 30 * 6;
        consensus.nPowTargetSpacing = 15;
        consensus.nPosTargetSpacing = 15;
        consensus.nPosTargetSpacingV2 = 30;
        consensus.nStakeMinAge = 5 * 60; //5min
        consensus.nStakeMinDepth = 25; //25 blocks

        consensus.nSingleThreadMaxTxesSize = 500;
        consensus.nMaxAmountLoadedRecords = 500;

        consensus.nTimeSlotLength = 15;
        
        // Height based params
        consensus.height_last_PoW = 199;
        consensus.nLwmaProtocolHeight = 200;
        consensus.height_governance = 500000;

        // spork keys
        // private key for testnet = f3ef66a62a9a1a9154c2822e75430f6d23653400b6b7b60d8248caa4e5d440bb
        //                           eff3ef66a62a9a1a9154c2822e75430f6d23653400b6b7b60d8248caa4e5d440bb (add privkey byte)
        //                         = cce29ade6834684da5a062951df587da5c6f7cfa9b1bf3dd46910f15ec29ae2c   (result of sha256 of 33byte)
        //                         = f832ed9714ec3ac66117cd49ff884ea58706b8a386233166201431ca529aca47   (result of sha256 of 32byte)
        //                         = f832ed97 (first 4 bytes)
        //                         = eff3ef66a62a9a1a9154c2822e75430f6d23653400b6b7b60d8248caa4e5d440bbf832ed97 (private key + privkey byte + checksum4b)
        //                  base58 = 93SMACiTD5eUF28mmpgZgwGboWXDgj6HXx15wCrgrr7wfrBFWt6
        
        // Address xyT264deWQZ6p8YJwQT3rVcqpf8dkCeJfM
        // Pubkey 03c064d2dadca0c11d4f31bc9f1857b3b1a51289f3c8e2be6653f49951c53bf083
        // Privkey cNX6wDaf33qeNHSgiz85EWBzTQCbX9rKtgiu5h8DgvMwFf4T8kSB

        consensus.strSporkPubKey = "03c064d2dadca0c11d4f31bc9f1857b3b1a51289f3c8e2be6653f49951c53bf083";
        consensus.strSporkPubKeyOld = "0457fe3e90da4bb4899ced14cbed073fb0174294975cb1b9e1a085990674117860a90912d9e91b83d8e2fff11716ed0e938a742e9862af37a6e545318e1ccd7472";
        consensus.nTime_EnforceNewSporkKey = 1669797609; //Wed Nov 30 2022 03:40:09 GMT-0500 (Eastern Standard Time)
        consensus.nTime_RejectOldSporkKey = 1669797309; //Wed Nov 30 2022 03:35:09 GMT-0500 (Eastern Standard Time)

        consensus.height_last_ZC_AccumCheckpoint = std::numeric_limits<int>::max();
        consensus.height_last_ZC_WrappedSerials = std::numeric_limits<int>::max();
        consensus.height_start_InvalidUTXOsCheck = std::numeric_limits<int>::max();
        consensus.height_start_ZC_InvalidSerials = std::numeric_limits<int>::max();
        consensus.height_start_ZC_SerialRangeCheck = std::numeric_limits<int>::max();
        consensus.height_ZC_RecalcAccumulators = std::numeric_limits<int>::max();

        // Zerocoin-related params
        consensus.ZC_Modulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
                "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
                "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
                "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
                "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
                "31438167899885040445364023527381951378636564391212010397122822120720357";
        consensus.ZC_MaxPublicSpendsPerTx = 637;    // Assume about 220 bytes each input
        consensus.ZC_MaxSpendsPerTx = 7;            // Assume about 20kb each input
        consensus.ZC_MinMintConfirmations = 20;
        consensus.ZC_MinMintFee = 1 * CENT;
        consensus.ZC_MinStakeDepth = 200;
        consensus.ZC_TimeStart = std::numeric_limits<int>::max();
        consensus.ZC_WrappedSerialsSupply = 0;   // WrappedSerials only on main net

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight          = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight     = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 200;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 201;
        consensus.vUpgrades[Consensus::UPGRADE_ZC].nActivationHeight            = std::numeric_limits<int>::max();
        consensus.vUpgrades[Consensus::UPGRADE_ZC_V2].nActivationHeight         = std::numeric_limits<int>::max();
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_PUBLIC].nActivationHeight     = std::numeric_limits<int>::max();
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].nActivationHeight          = 201; // Setting 1 block past last pow
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].nActivationHeight          = 202; // Setting 2 blocks past last pow
        consensus.vUpgrades[Consensus::UPGRADE_V5_DUMMY].nActivationHeight      = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.vUpgrades[Consensus::UPGRADE_ZC].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_ZC_V2].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_ZC_PUBLIC].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].hashActivationBlock =
                uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */

        pchMessageStart[0] = 0x11;
        pchMessageStart[1] = 0x22;
        pchMessageStart[2] = 0x33;
        pchMessageStart[3] = 0x44;
        nDefaultPort = 11591;

        vFixedSeeds.clear();
        vSeeds.clear();


        vSeeds.push_back(CDNSSeedData("146.190.13.2409 ", "146.190.13.240 ", true));
        vSeeds.push_back(CDNSSeedData("24.199.68.190 ", "24.199.68.190 ", true));
        vSeeds.push_back(CDNSSeedData("24.199.68.69", "24.199.68.69", true));


        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 139); // Testnet pivx addresses start with 'x' or 'y'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 19);  // Testnet pivx script addresses start with '8' or '9'
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 73);     // starting with 'W'
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);     // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        // Testnet pivx BIP32 pubkeys start with 'DRKV'
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x3a)(0x80)(0x61)(0xa0).convert_to_container<std::vector<unsigned char> >();
        // Testnet pivx BIP32 prvkeys start with 'DRKP'
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x3a)(0x80)(0x58)(0x37).convert_to_container<std::vector<unsigned char> >();
        // Testnet pivx BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = boost::assign::list_of(0x80)(0x00)(0x00)(0x01).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        // Sapling
        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "ptestsapling";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "pviewtestsapling";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "pivktestsapling";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "p-secret-spending-key-test";

        strMasterAddress = "yKvgjULiZYkikTNCqJaD9YuUYkLrEyRg3m";
        //Master pubkey 03c8b46e7a2894c9cc049a679b76ed40ad1ef65c6601f4f6987c193611369b3a9d
        //Master privkey cSgY6DmSKhG2ci2sckpQSoy1cgwRCx7eEMHd82HgwtqL7ieGSgjy

        foundationFundAddress = "yEMBsHESUmk1mVNugUrGmazWEhA4qesSmm";
        //foundationFundAddress pubkey 031ce00e132c89ce5a50c17d6dd8e734c2283f217e46ccc1a7f833d12e34141578
        //foundationFundAddress privkey cNrKLx5PqtBzHYbkoA7KNg18pzyargQ4hMx9m5bz3tJLZ8bn212x

        strFeeAddress = "xwwLWkB1nr8NsPgfRXt63wxBDLYaJZ7dSM";
        //Fee pubkey 029704f22d19be0d31205dfe30e7928b509408dbc7c943aae62f708d50a3c028cc
        //Fee privkey cVcBR3TwmLaBkuNBYi5YGFr3AxGexwJ3Qu2miH8BcGhEyB7UwF6h

        tokenFixedFee = 1 * COIN;
        tokenManagedFee = 1 * COIN;
        tokenVariableFee = 1 * COIN;
        tokenUsernameFee = 1 * COIN;
        tokenSubFee = 0 * COIN;

    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams
{
public:
    CRegTestParams()
    {
        networkID = CBaseChainParams::REGTEST;
        strNetworkID = "regtest";

        genesis = CreateGenesisBlock(1454124731, 2402015, 0x1e0ffff0, 1, 250 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0x0000041e482b9b9691d98eefb48473405c0b8ec31b76df3797c74a78680ef818"));
        //assert(genesis.hashMerkleRoot == uint256S("0x1b2ef6e2f28be914103a277377ae7729dcd125dfeb8bf97bd5964ba72b6dc39b"));

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.powLimit   = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimit   = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV2 = ~UINT256_ZERO >> 20;
        consensus.nBudgetCycleBlocks = 43200;         // approx 10 cycles per day
        consensus.nBudgetFeeConfirmations = 3;      // (only 8-blocks window for finalization on regtest)
        consensus.nCoinbaseMaturity = 1;

        // Block reward related params
        consensus.nMaxMoneyOut = 35000000000 * COIN;
        consensus.nHalvingInterval = 500000;  // Halve block reward every 500k blocks
        consensus.nPreMine = 10000000 * COIN; // 10m premined coins
        consensus.nBlockReward = 0.17835 * COIN;
        consensus.nMasternodeReward = 0.1;             // 10%
        consensus.nDevReward = 0.7;                    // 70%
        consensus.nStakeReward = 0.2;                  // 20%

        consensus.nProposalEstablishmentTime = 60 * 5;  // at least 5 min old to make it into a budget
        consensus.nPowTargetSpacing = 1 * 60;
        consensus.nTimeSlotLength = 15;
        consensus.nFutureTimeDriftPoW = 7200;
        consensus.nFutureTimeDriftPoS = 180;
        consensus.nMaxProposalPayments = 6;

        consensus.nPoolMaxTransactions = 2;

        consensus.nStakeMinAge = 0;
        consensus.nStakeMinDepth = 2;
        consensus.nPowTargetTimespan = 30 * 60;

        consensus.nSingleThreadMaxTxesSize = 500;
        consensus.nMaxAmountLoadedRecords = 500;

        /* Spork Key for RegTest:
        WIF private key: 932HEevBSujW2ud7RfB1YF91AFygbBRQj3de3LyaCRqNzKKgWXi
        private key hex: bd4960dcbd9e7f2223f24e7164ecb6f1fe96fc3a416f5d3a830ba5720c84b8ca
        Address: yCvUVd72w7xpimf981m114FSFbmAmne7j9
        */
        consensus.strSporkPubKey = "043969b1b0e6f327de37f297a015d37e2235eaaeeb3933deecd8162c075cee0207b13537618bde640879606001a8136091c62ec272dd0133424a178704e6e75bb7";
        consensus.strSporkPubKeyOld = "";
        consensus.nTime_EnforceNewSporkKey = 0;
        consensus.nTime_RejectOldSporkKey = 0;

        // height based activations
        consensus.height_last_PoW = 250;
        consensus.height_last_ZC_AccumCheckpoint = 310;     // no checkpoints on regtest
        consensus.height_last_ZC_WrappedSerials = -1;
        consensus.height_start_InvalidUTXOsCheck = 999999999;
        consensus.height_start_ZC_InvalidSerials = 999999999;
        consensus.height_start_ZC_SerialRangeCheck = 300;
        consensus.height_ZC_RecalcAccumulators = 999999999;

        consensus.height_governance = std::numeric_limits<int>::max();

        // Zerocoin-related params
        consensus.ZC_Modulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
                "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
                "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
                "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
                "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
                "31438167899885040445364023527381951378636564391212010397122822120720357";
        consensus.ZC_MaxPublicSpendsPerTx = 637;    // Assume about 220 bytes each input
        consensus.ZC_MaxSpendsPerTx = 7;            // Assume about 20kb each input
        consensus.ZC_MinMintConfirmations = 10;
        consensus.ZC_MinMintFee = 1 * CENT;
        consensus.ZC_MinStakeDepth = 10;
        consensus.ZC_TimeStart = 0;                 // not implemented on regtest
        consensus.ZC_WrappedSerialsSupply = 0;

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 251;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 251;
        consensus.vUpgrades[Consensus::UPGRADE_ZC].nActivationHeight            = 300;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_V2].nActivationHeight         = 300;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_PUBLIC].nActivationHeight     = 400;
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].nActivationHeight          = 251;
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].nActivationHeight          =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_V5_DUMMY].nActivationHeight       = 300;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */

        pchMessageStart[0] = 0xa1;
        pchMessageStart[1] = 0xcf;
        pchMessageStart[2] = 0x7e;
        pchMessageStart[3] = 0xac;
        nDefaultPort = 21591;

        vFixedSeeds.clear(); //! Testnet mode doesn't have any fixed seeds.
        vSeeds.clear();      //! Testnet mode doesn't have any DNS seeds.

        // Sapling
        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "pregs";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "pregviews";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "pregivks";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]         = "p-reg-secret-spending-key-main";

        foundationFundAddress = "yEMBsHESUmk1mVNugUrGmazWEhA4qesSmm";
        //foundationFundAddress pubkey 031ce00e132c89ce5a50c17d6dd8e734c2283f217e46ccc1a7f833d12e34141578
        //foundationFundAddress privkey cNrKLx5PqtBzHYbkoA7KNg18pzyargQ4hMx9m5bz3tJLZ8bn212x

        strFeeAddress = "xwwLWkB1nr8NsPgfRXt63wxBDLYaJZ7dSM";
        //Fee pubkey 029704f22d19be0d31205dfe30e7928b509408dbc7c943aae62f708d50a3c028cc
        //Fee privkey cVcBR3TwmLaBkuNBYi5YGFr3AxGexwJ3Qu2miH8BcGhEyB7UwF6h

        tokenFixedFee = 1 * COIN;
        tokenManagedFee = 1 * COIN;
        tokenVariableFee = 1 * COIN;
        tokenUsernameFee = 1 * COIN;
        tokenSubFee = 1 * COIN;
    }

    const Checkpoints::CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }

    void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
    {
        assert(idx > Consensus::BASE_NETWORK && idx < Consensus::MAX_NETWORK_UPGRADES);
        consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
    }
};
static CRegTestParams regTestParams;

static CChainParams* pCurrentParams = 0;

const CChainParams& Params()
{
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(CBaseChainParams::Network network)
{
    switch (network) {
    case CBaseChainParams::MAIN:
        return mainParams;
    case CBaseChainParams::TESTNET:
        return testNetParams;
    case CBaseChainParams::REGTEST:
        return regTestParams;
    default:
        assert(false && "Unimplemented network");
        return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    regTestParams.UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}
