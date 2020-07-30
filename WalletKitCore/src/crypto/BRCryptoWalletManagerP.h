//
//  BRCryptoWalletManagerP.h
//  BRCrypto
//
//  Created by Michael Carrara on 6/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWalletManagerP_h
#define BRCryptoWalletManagerP_h

#include <pthread.h>
#include "support/BRArray.h"

#include "BRCryptoBase.h"
#include "BRCryptoNetwork.h"
#include "BRCryptoAccount.h"
#include "BRCryptoWallet.h"
#include "BRCryptoWalletManager.h"

#include "BRCryptoClientP.h"
#include "BRCryptoWalletP.h"

#include "support/BRFileService.h"
#include "support/event/BREvent.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - WalletManager Handlers

typedef BRCryptoWalletManager
(*BRCryptoWalletManagerCreateHandler) (BRCryptoListener listener,
                                       BRCryptoClient client,
                                       BRCryptoAccount account,
                                       BRCryptoNetwork network,
                                       BRCryptoSyncMode mode,
                                       BRCryptoAddressScheme scheme,
                                       const char *path);

typedef void
(*BRCryptoWalletManagerReleaseHandler) (BRCryptoWalletManager manager);

typedef void
(*BRCryptoWalletManagerInitializeHandler) (BRCryptoWalletManager manager);

typedef BRFileService
(*BRCryptoWalletManagerCreateFileServiceHandler) (BRCryptoWalletManager manager,
                                                  const char *basePath,
                                                  const char *currency,
                                                  const char *network,
                                                  BRFileServiceContext context,
                                                  BRFileServiceErrorHandler handler);

typedef const BREventType **
(*BRCryptoWalletManagerGetEventTypesHandler) (BRCryptoWalletManager manager,
                                              size_t *eventTypesCount);

typedef BRCryptoBoolean
(*BRCryptoWalletManagerSignTransactionWithSeedHandler) (BRCryptoWalletManager manager,
                                                        BRCryptoWallet wallet,
                                                        BRCryptoTransfer transfer,
                                                        UInt512 seed);

typedef BRCryptoBoolean
(*BRCryptoWalletManagerSignTransactionWithKeyHandler) (BRCryptoWalletManager manager,
                                                       BRCryptoWallet wallet,
                                                       BRCryptoTransfer transfer,
                                                       BRCryptoKey key);

typedef BRCryptoAmount
(*BRCryptoWalletManagerEstimateLimitHandler) (BRCryptoWalletManager cwm,
                                              BRCryptoWallet  wallet,
                                              BRCryptoBoolean asMaximum,
                                              BRCryptoAddress target,
                                              BRCryptoNetworkFee fee,
                                              BRCryptoBoolean *needEstimate,
                                              BRCryptoBoolean *isZeroIfInsuffientFunds,
                                              BRCryptoUnit unit);


typedef BRCryptoFeeBasis // If NULL, don't generate WalletEvent; expect QRY callback invoked.
(*BRCryptoWalletManagerEstimateFeeBasisHandler) (BRCryptoWalletManager cwm,
                                                 BRCryptoWallet  wallet,
                                                 BRCryptoCookie cookie,
                                                 BRCryptoAddress target,
                                                 BRCryptoAmount amount,
                                                 BRCryptoNetworkFee fee);

typedef BRCryptoClientP2PManager
(*BRCryptoWalletManagerCreateP2PManagerHandler) (BRCryptoWalletManager cwm);

typedef BRCryptoWallet
(*BRCryptoWalletManagerCreateWalletHandler) (BRCryptoWalletManager cwm,
                                               BRCryptoCurrency currency);

typedef void
(*BRCryptoWalletManagerRecoverTransfersFromTransactionBundleHandler) (BRCryptoWalletManager cwm,
                                                                      OwnershipKept BRCryptoClientTransactionBundle bundle);

typedef void
(*BRCryptoWalletManagerRecoverTransferFromTransferBundleHandler) (BRCryptoWalletManager cwm,
                                                                  OwnershipKept BRCryptoClientTransferBundle bundle);

typedef BRCryptoWalletSweeperStatus
(*BRCryptoWalletManagerWalletSweeperValidateSupportedHandler) (BRCryptoWalletManager cwm,
                                                               BRCryptoWallet wallet,
                                                               BRCryptoKey key);

typedef BRCryptoWalletSweeper
(*BRCryptoWalletManagerCreateWalletSweeperHandler) (BRCryptoWalletManager cwm,
                                                    BRCryptoWallet wallet,
                                                    BRCryptoKey key);

typedef struct {
    BRCryptoWalletManagerCreateHandler create;
    BRCryptoWalletManagerReleaseHandler release;
    BRCryptoWalletManagerInitializeHandler initialize;
    BRCryptoWalletManagerCreateFileServiceHandler createFileService;
    BRCryptoWalletManagerGetEventTypesHandler getEventTypes;
    BRCryptoWalletManagerCreateWalletHandler createWallet;
    BRCryptoWalletManagerSignTransactionWithSeedHandler signTransactionWithSeed;
    BRCryptoWalletManagerSignTransactionWithKeyHandler signTransactionWithKey;
    BRCryptoWalletManagerEstimateLimitHandler estimateLimit;
    BRCryptoWalletManagerEstimateFeeBasisHandler estimateFeeBasis;
    BRCryptoWalletManagerCreateP2PManagerHandler createP2PManager;
    BRCryptoWalletManagerRecoverTransfersFromTransactionBundleHandler recoverTransfersFromTransactionBundle;
    BRCryptoWalletManagerRecoverTransferFromTransferBundleHandler recoverTransferFromTransferBundle;
    BRCryptoWalletManagerWalletSweeperValidateSupportedHandler validateSweeperSupported;
    BRCryptoWalletManagerCreateWalletSweeperHandler createSweeper;
} BRCryptoWalletManagerHandlers;

// MARK: - Wallet Manager State

private_extern BRCryptoWalletManagerState
cryptoWalletManagerStateInit(BRCryptoWalletManagerStateType type);

private_extern BRCryptoWalletManagerState
cryptoWalletManagerStateDisconnectedInit(BRCryptoWalletManagerDisconnectReason reason);

// MARK: - Wallet Manager

struct BRCryptoWalletManagerRecord {
    BRCryptoBlockChainType type;
    const BRCryptoWalletManagerHandlers *handlers;
    BRCryptoRef ref;
    size_t sizeInBytes;

    pthread_mutex_t lock;
    BRCryptoListener listener;
    BRCryptoClient client;
    BRCryptoNetwork network;
    BRCryptoAccount account;
    BRCryptoAddressScheme addressScheme;

    char *path;
    BRFileService fileService;

    BREventHandler handler;

    BRCryptoClientP2PManager p2pManager;   // Null unless BTC, BCH, ETH, ...
    BRCryptoClientQRYManager qryManager;

    BRCryptoClientQRYByType byType;
    
    BRCryptoSyncMode syncMode;
    BRCryptoClientSync canSync;
    BRCryptoClientSend canSend;

    /// The primary wallet
    BRCryptoWallet wallet;

    /// All wallets
    BRArrayOf(BRCryptoWallet) wallets;

    BRCryptoWalletManagerState state;

    BRCryptoWalletListener listenerWallet;
    BRCryptoListener listenerTrampoline;
};

extern BRCryptoWalletManager
cryptoWalletManagerAllocAndInit (size_t sizeInBytes,
                                 BRCryptoBlockChainType type,
                                 BRCryptoListener listener,
                                 BRCryptoClient client,
                                 BRCryptoAccount account,
                                 BRCryptoNetwork network,
                                 BRCryptoAddressScheme scheme,
                                 const char *path,
                                 BRCryptoClientQRYByType byType);

private_extern BRCryptoBlockChainType
cryptoWalletManagerGetType (BRCryptoWalletManager manager);

private_extern void
cryptoWalletManagerSetState (BRCryptoWalletManager cwm,
                             BRCryptoWalletManagerState state);

private_extern void
cryptoWalletManagerStop (BRCryptoWalletManager cwm);

private_extern void
cryptoWalletManagerAddWallet (BRCryptoWalletManager cwm,
                              BRCryptoWallet wallet);

private_extern void
cryptoWalletManagerRemWallet (BRCryptoWalletManager cwm,
                              BRCryptoWallet wallet);

#ifdef REFACTOR
private_extern BRWalletManagerClient
cryptoWalletManagerClientCreateBTCClient (OwnershipKept BRCryptoWalletManager cwm);

private_extern BREthereumClient
cryptoWalletManagerClientCreateETHClient (OwnershipKept BRCryptoWalletManager cwm);

private_extern BRGenericClient
cryptoWalletManagerClientCreateGENClient (OwnershipKept BRCryptoWalletManager cwm);

extern void
cryptoWalletManagerHandleTransferGEN (BRCryptoWalletManager cwm,
                                      OwnershipGiven BRGenericTransfer transferGeneric);

private_extern void
cryptoWalletManagerSetTransferStateGEN (BRCryptoWalletManager cwm,
                                        BRCryptoWallet wallet,
                                        BRCryptoTransfer transfer,
                                        BRGenericTransferState newGenericState);
#endif

private_extern void
cryptoWalletManagerRecoverTransfersFromTransactionBundle (BRCryptoWalletManager cwm,
                                                          OwnershipKept BRCryptoClientTransactionBundle bundle);

private_extern void
cryptoWalletManagerRecoverTransferFromTransferBundle (BRCryptoWalletManager cwm,
                                                      OwnershipKept BRCryptoClientTransferBundle bundle);

//private_extern void
//cryptoWalletManagerGenerateTransferEvent (BRCryptoWalletManager cwm,
//                                          BRCryptoWallet wallet,
//                                          BRCryptoTransfer transfer,
//                                          BRCryptoTransferEvent event);
//
//private_extern void
//cryptoWalletManagerGenerateWalletEvent (BRCryptoWalletManager cwm,
//                                          BRCryptoWallet wallet,
//                                          BRCryptoWalletEvent event);
//

private_extern void
cryptoWalletManagerGenerateEvent (BRCryptoWalletManager cwm,
                                  BRCryptoWalletManagerEvent event);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoWalletManagerP_h */
