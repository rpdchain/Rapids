#!/bin/bash

SRCDIR=./src/
NUL=/dev/null
PASS=0
FAIL=0
clear
printf "Preparing a test environment...\n"
printf "   * Starting a fresh regtest daemon with open activation sender\n"
rm -r ~/.bitcoin/regtest
$SRCDIR/tokencored --regtest --server --daemon --tokenactivationallowsender=any >$NUL
sleep 3
printf "   * Preparing some mature testnet RPD\n"
$SRCDIR/tokencore-cli --regtest setgenerate true 102 >$NUL
printf "   * Obtaining a master address to work with\n"
ADDR=$($SRCDIR/tokencore-cli --regtest getnewaddress TOKENAccount)
printf "   * Funding the address with some testnet RPD for fees\n"
$SRCDIR/tokencore-cli --regtest sendtoaddress $ADDR 20 >$NUL
$SRCDIR/tokencore-cli --regtest setgenerate true 1 >$NUL
printf "   * Participating in the Exodus crowdsale to obtain some TOKEN\n"
JSON="{\"moneyqMan7uh8FqdCA2BV5yZ8qVrc9ikLP\":10,\""$ADDR"\":4}"
$SRCDIR/tokencore-cli --regtest sendmany TOKENAccount $JSON >$NUL
$SRCDIR/tokencore-cli --regtest setgenerate true 1 >$NUL
printf "   * Creating an indivisible test property\n"
$SRCDIR/tokencore-cli --regtest token_sendissuancefixed $ADDR 1 1 0 "Z_TestCat" "Z_TestSubCat" "Z_IndivisTestProperty" "Z_TestURL" "Z_TestData" 10000000 >$NUL
$SRCDIR/tokencore-cli --regtest setgenerate true 1 >$NUL
printf "   * Creating a divisible test property\n"
$SRCDIR/tokencore-cli --regtest token_sendissuancefixed $ADDR 1 2 0 "Z_TestCat" "Z_TestSubCat" "Z_DivisTestProperty" "Z_TestURL" "Z_TestData" 10000 >$NUL
$SRCDIR/tokencore-cli --regtest setgenerate true 1 >$NUL
printf "\nTesting a trade against self that uses Token (1.1 TOKEN for 20 #3)\n"
printf "   * Executing the trade\n"
TXID=$($SRCDIR/tokencore-cli --regtest sendtokentrade $ADDR 3 20 1 1.1)
$SRCDIR/tokencore-cli --regtest setgenerate true 1 >$NUL
printf "   * Verifiying the results\n"
printf "      # Checking the trade was valid..."
RESULT=$($SRCDIR/tokencore-cli --regtest token_gettransaction $TXID | grep valid | cut -c15-)
if [ $RESULT == "true," ]
  then
    printf "PASS\n"
    PASS=$((PASS+1))
  else
    printf "FAIL (result:%s)\n" $RESULT
    FAIL=$((FAIL+1))
fi
printf "\nTesting a trade against self that doesn't use Token to confirm non-Token pairs are disabled (4.45 #4 for 20 #3)\n"
printf "   * Executing the trade\n"
TXID=$($SRCDIR/tokencore-cli --regtest sendtokentrade $ADDR 3 20 4 4.45)
$SRCDIR/tokencore-cli --regtest setgenerate true 1 >$NUL
printf "   * Verifiying the results\n"
printf "      # Checking the trade was invalid..."
RESULT=$($SRCDIR/tokencore-cli --regtest token_gettransaction $TXID | grep valid | cut -c15-)
if [ $RESULT == "false," ]
  then
    printf "PASS\n"
    PASS=$((PASS+1))
  else
    printf "FAIL (result:%s)\n" $RESULT
    FAIL=$((FAIL+1))
fi
printf "\nActivating trading on all pairs\n"
printf "   * Sending the activation\n"
BLOCKS=$($SRCDIR/tokencore-cli --regtest getblockcount)
TXID=$($SRCDIR/tokencore-cli --regtest token_sendactivation $ADDR 8 $(($BLOCKS + 8)) 999)
$SRCDIR/tokencore-cli --regtest setgenerate true 1 >$NUL
printf "     # Checking the activation transaction was valid..."
RESULT=$($SRCDIR/tokencore-cli --regtest token_gettransaction $TXID | grep valid | cut -c15-)
if [ $RESULT == "true," ]
  then
    printf "PASS\n"
    PASS=$((PASS+1))
  else
    printf "FAIL (result:%s)\n" $RESULT
    FAIL=$((FAIL+1))
fi
printf "   * Mining 10 blocks to forward past the activation block\n"
$SRCDIR/tokencore-cli --regtest setgenerate true 10 >$NUL
printf "     # Checking the activation went live as expected..."
FEATUREID=$($SRCDIR/tokencore-cli --regtest token_getactivations | grep -A 10 completed | grep featureid | cut -c27)
if [ $FEATUREID == "8" ]
  then
    printf "PASS\n"
    PASS=$((PASS+1))
  else
    printf "FAIL (result:%s)\n" $FEATUREID
    FAIL=$((FAIL+1))
fi
printf "\nTesting a trade against self that doesn't use Token to confirm non-Token pairs are now enabled (4.45 #4 for 20 #3)\n"
printf "   * Executing the trade\n"
TXID=$($SRCDIR/tokencore-cli --regtest sendtokentrade $ADDR 3 20 4 4.45)
$SRCDIR/tokencore-cli --regtest setgenerate true 1 >$NUL
printf "   * Verifiying the results\n"
printf "      # Checking the trade was valid..."
RESULT=$($SRCDIR/tokencore-cli --regtest token_gettransaction $TXID | grep valid | cut -c15-)
if [ $RESULT == "true," ]
  then
    printf "PASS\n"
    PASS=$((PASS+1))
  else
    printf "FAIL (result:%s)\n" $RESULT
    FAIL=$((FAIL+1))
fi

printf "\n"
printf "####################\n"
printf "#  Summary:        #\n"
printf "#    Passed = %d    #\n" $PASS
printf "#    Failed = %d    #\n" $FAIL
printf "####################\n"
printf "\n"

$SRCDIR/tokencore-cli --regtest stop

