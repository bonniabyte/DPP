/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
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
 ************************************************************************************/
#include <map>
#include <dpp/discord.h>
#include <dpp/cluster.h>
#include <dpp/discordclient.h>
#include <dpp/discordevents.h>
#include <dpp/message.h>
#include <dpp/cache.h>
#include <dpp/nlohmann/json.hpp>
#include <utility>
#include <dpp/fmt/format.h>

namespace dpp {

void cluster::guild_get_members(snowflake guild_id, uint16_t limit, snowflake after, command_completion_event_t callback) {
	this->post_rest(API_PATH "/guilds", std::to_string(guild_id), fmt::format("members?limit={}&after={}", limit, after), m_get, "", [callback, guild_id](json &j, const http_request_completion_t& http) {
		guild_member_map guild_members;
		for (auto & curr_member : j) {
			guild_member gm;
			snowflake user_id = 0;
			if (curr_member.find("user") != curr_member.end()) {
				user_id = SnowflakeNotNull(&(curr_member["user"]), "id");
			}
			guild_members[SnowflakeNotNull(&curr_member, "id")] = guild_member().fill_from_json(&curr_member, guild_id, user_id);
		}
		if (callback) {
				callback(confirmation_callback_t("guild_member_map", guild_members, http));
		}
	});
}

};