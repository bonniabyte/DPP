/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * SPDX-License-Identifier: Apache-2.0
 * Copyright 2021 Craig Edwards and D++ contributors
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This folder is a modified fork of libdave, https://github.com/discord/libdave
 * Copyright (c) 2024 Discord, Licensed under MIT
 *
 ************************************************************************************/
#pragma once

#include <deque>
#include <memory>
#include <optional>
#include <unordered_map>

#include "cipher_interface.h"
#include "key_ratchet.h"
#include "common.h"
#include "clock.h"

namespace dpp {
	class cluster;
}

namespace dpp::dave {

/**
 * @brief Compute wrapped generation for key
 * @param oldest oldest key generation
 * @param generation key generation
 * @return wrapped computed generation
 */
key_generation compute_wrapped_generation(key_generation oldest, key_generation generation);

/**
 * @brief A big nonce (64 bits)
 */
using big_nonce = uint64_t;

/**
 * @brief Compute wrapped big nonce
 * @param generation generation
 * @param nonce truncated sync nonce
 * @return big nonce (64 bits)
 */
big_nonce compute_wrapped_big_nonce(key_generation generation, truncated_sync_nonce nonce);

/**
 * @brief A manager to handle whichever cipher is best for the current version of DAVE
 *
 * This will currently instantiate an AES 128 GCM AEAD cipher.
 */
class aead_cipher_manager {
public:
	/**
	 * @brief Chrono time point
	 */
	using time_point = typename clock_interface::time_point;

	/**
	 * @brief Constructor
	 * @param cl Creating cluster
	 * @param clock chrono clock
	 * @param keyRatchet key ratchet for cipher
	 */
	aead_cipher_manager(dpp::cluster& cl, const clock_interface& clock, std::unique_ptr<key_ratchet_interface> keyRatchet);

	/**
	 * @brief Update cipher expiry
	 * @param expiry expiry time
	 */
	void update_expiry(time_point expiry) {
		ratchetExpiry_ = expiry;
	}

	/**
	 * @brief True if cipher has expired
	 * @return true if expired
	 */
	bool is_expired() const {
		return clock_.now() > ratchetExpiry_;
	}

	/**
	 * @brief True if nonce can be processed for generation
	 * @param generation key generation
	 * @param nonce nonce/IV
	 * @return true if can be processed
	 */
	bool can_process_nonce(key_generation generation, truncated_sync_nonce nonce) const;

	/**
	 * Compute wrapped generation for key
	 * @param generation key generation
	 * @return key generation
	 */
	key_generation compute_wrapped_generation(key_generation generation);

	cipher_interface* get_cipher(key_generation generation);

	/**
	 * @brief Updates the expiry time for all old ciphers
	 * @param generation key generation
	 * @param nonce nonce/IV
	 */
	void report_cipher_success(key_generation generation, truncated_sync_nonce nonce);

private:
	/**
	 * @brief Cipher with an expiry date/time
	 */
	struct expiring_cipher {
		/**
		 * @brief Cipher
		 */
		std::unique_ptr<cipher_interface> cryptor;

		/**
		 * @brief Expiry time
		 */
		time_point expiry;
	};

	/**
	 * Create a cipher with an expiry time
	 * @param generation key generation
	 * @return expiring cipher
	 */
	expiring_cipher make_expiring_cipher(key_generation generation);

	/**
	 * @brief Clean up old expired ciphers
	 */
	void cleanup_expired_ciphers();

	const clock_interface& clock_;
	std::unique_ptr<key_ratchet_interface> keyRatchet_;
	std::unordered_map<key_generation, expiring_cipher> cryptors_;

	time_point ratchetCreation_;
	time_point ratchetExpiry_;
	key_generation oldestGeneration_{0};
	key_generation newestGeneration_{0};

	std::optional<big_nonce> newestProcessedNonce_;
	std::deque<big_nonce> missingNonces_;

	/**
	 * @brief DPP Cluster, used for logging
	 */
	dpp::cluster& creator;

};

} // namespace dpp::dave

