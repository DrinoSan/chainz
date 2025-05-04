# Blockchain Documentation

## Overview
This project implements a basic blockchain system with functionalities such as transaction validation, block mining, dynamic difficulty adjustment, and UTXO management. The `Blockchain` class is the core of the system, managing the chain of blocks, pending transactions, and UTXO set.

---

## Blockchain Class

### Constructor
- **`Blockchain()`**
  - Initializes the blockchain with a genesis block.

---

### Key Methods

#### `addTransaction(const Transaction& tx)`
- Adds a transaction to the mempool after validation.
- **Returns:** `true` if the transaction is valid and added, `false` otherwise.

#### `mineBlock()`
- Mines a new block using the pending transactions and adds it to the chain.

#### `isChainValid() const`
- Validates the integrity of the blockchain.
- **Returns:** `true` if the chain is valid, `false` otherwise.

#### `addBlock(const Block& block)`
- Adds a block to the chain after performing various validations.
- **Returns:** `true` if the block is valid and added, `false` otherwise.

#### `isValidTransaction(const Transaction& tx) const`
- Validates a transaction by checking UTXOs, balances, and preventing double-spending.
- **Returns:** `true` if the transaction is valid, `false` otherwise.

#### `minePendingTransactions(std::string& minerAddress)`
- Mines a block with pending transactions and rewards the miner.
- **Returns:** The mined `Block`.

#### `selectTransactions(size_t max)`
- Selects transactions from the mempool based on fees for inclusion in the next block.
- **Returns:** A vector of selected `Transaction` objects.

#### `getUTXOsForAddress(const std::string& address) const`
- Retrieves all UTXOs associated with a specific address.
- **Returns:** A vector of `UTXO` objects.

#### `adjustDifficulty() const`
- Dynamically adjusts the mining difficulty based on the time taken to mine the last 10 blocks.
- **Returns:** The new difficulty level.

#### `calculateExpectedDifficulty() const`
- Calculates the expected difficulty for the next block based on the current chain state.
- **Returns:** The expected difficulty level.

---

## Additional Notes
- **Proof of Work (PoW):** The difficulty level for mining is dynamically adjusted based on block generation time.
- **Dynamic Difficulty Adjustment:** The system adjusts the mining difficulty to maintain a consistent block generation time. If blocks are mined too quickly, the difficulty increases; if they are mined too slowly, the difficulty decreases.
  - **Epoch Duration:** The difficulty is updated every 10 blocks. This ensures that adjustments are made periodically based on the average block generation time over the last epoch.
- **Reward Transactions:** Miners receive a reward of `10` units plus transaction fees for successfully mining a block.
- **UTXO Management:** The system ensures proper handling of UTXOs to prevent double-spending and maintain balance integrity.
- **Transaction Fees:** Transactions include fees, which are added to the miner's reward.

---

## Future Improvements
1. **Improve Transaction Validation**
   - Add digital signature verification for transactions using cryptographic libraries like OpenSSL or libsodium.
   - Implement public-private key pairs for users to sign transactions.
   - Ensure double-spend prevention by validating UTXOs more rigorously.

2. **Enhance Mining Mechanism**
   - Introduce a block reward halving mechanism to mimic Bitcoin's reward schedule.
   - Implement orphan block handling to manage forks in the chain.

3. **Implement Networking Features**
   - Add peer-to-peer communication for decentralized block and transaction propagation.
   - Implement a gossip protocol for efficient data sharing between nodes.

4. **Add Wallet Functionality**
   - Create a wallet system for users to manage their keys and balances.
   - Provide a command-line interface for wallet operations.

5. **Improve Security**
   - Add replay protection to prevent transaction reuse across forks.
   - Implement Merkle trees for efficient transaction verification within blocks.

6. **Optimize Performance**
   - Improve transaction selection by prioritizing those with the highest fees.
   - Optimize the mining process to reduce computational overhead.

7. **Add Consensus Mechanisms**
   - Implement a basic consensus algorithm to resolve forks and ensure chain consistency.
   - Explore Proof of Stake (PoS) as an alternative to Proof of Work (PoW).

8. **Enhance Block Validation**
   - Add stricter validation rules for blocks, including timestamp checks and transaction ordering.

9. **Improve Logging and Debugging**
   - Add detailed logging for blockchain operations to aid in debugging and monitoring.
   - Provide tools to visualize the blockchain and its state.

10. **Add Support for Smart Contracts**
    - Introduce a basic scripting language for creating and executing smart contracts.

11. **Implement Checkpoints**
    - Add checkpoint blocks to improve security and prevent long-range attacks.

12. **Add Unit and Integration Tests**
    - Write comprehensive tests for all components to ensure reliability and correctness.

---

## Learning Goals
This project is designed as a learning tool to understand the core concepts of blockchain technology. While it aims to mimic Bitcoin's functionality, it is intentionally simplified to focus on educational value.

## Dependencies
- [nlohmann/json](https://github.com/nlohmann/json): A modern C++ library for JSON parsing and serialization.
- [yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib): A C++11 single-file header-only cross-platform HTTP/HTTPS library.
