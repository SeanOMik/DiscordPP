#include "bot.h"
#include "utils.h"
#include "command_handler.h"
#include "guild.h"
#include "channel.h"
#include "message.h"
#include "member.h"
#include "role.h"
#include "activity.h"
#include "event_handler.h"

// Events includes: 
#include "events/channel_create_event.h"
#include "events/channel_delete_event.h"
#include "events/channel_pins_update_event.h"
#include "events/channel_update_event.h"
#include "events/guild_ban_add_event.h"
#include "events/guild_ban_remove_event.h"
#include "events/guild_create_event.h"
#include "events/guild_delete_event.h"
#include "events/guild_emojis_update_event.h"
#include "events/guild_integrations_update_event.h"
#include "events/guild_member_add_event.h"
#include "events/guild_member_remove_event.h"
#include "events/guild_member_update_event.h"
#include "events/guild_members_chunk_event.h"
#include "events/guild_role_create_event.h"
#include "events/guild_role_delete_event.h"
#include "events/guild_role_update_event.h"
#include "events/guild_update_event.h"
#include "events/invalid_session_event.h"
#include "events/message_bulk_delete_event.h"
#include "events/message_create_event.h"
#include "events/message_delete_event.h"
#include "events/message_reaction_add_event.h"
#include "events/message_reaction_remove_all_event.h"
#include "events/message_reaction_remove_event.h"
#include "events/message_update_event.h"
#include "events/presense_update_event.h"
#include "events/ready_event.h"
#include "events/reconnect_event.h"
#include "events/resumed_event.h"
#include "events/typing_start_event.h"
#include "events/user_update_event.h"
#include "events/voice_server_update_event.h"
#include "events/voice_state_update_event.h"
#include "events/webhooks_update_event.h"

namespace discpp {
    Bot::Bot(std::string token, BotConfig config) : token(
            token), config(config) {
        /**
         * @brief Constructs a discpp::Bot object.
         *
         * ```cpp
         *      discpp::Bot bot(TOKEN, {"+", "bot "}, discpp::logger_flags::ERROR_SEVERITY | discpp::logger_flags::WARNING_SEVERITY, "log.txt");
         * ```
         *
         * @param[in] token The discpp token the bot needs to run.
         * @param[in] prefixes The bot's prefixes that will be used for command handling.
         * @param[in] logger_flags The flags that will be passed to the logger.
         * @param[in] logger_path The file path for the logger, doesn't need one as it can just log to console/terminal instead.
         *
         * @return discpp::Bot, this is a constructor.
         */

        fire_command_method = std::bind(&discpp::FireCommand, std::placeholders::_1, std::placeholders::_2);

        discpp::globals::bot_instance = this;

        if (config.logger_path.empty()) {
            logger = discpp::Logger(config.logger_flags);
        } else {
            logger = discpp::Logger(config.logger_path, config.logger_flags);
        }
    }

    int Bot::Run() {
        /**
         * @brief Executes the discpp bot.
         *
         * ```cpp
         *      discpp::Bot bot(TOKEN, {"+", "bot ", discpp::logger_flags::ERROR_SEVERITY | discpp::logger_flags::WARNING_SEVERITY, "log.txt");
         *		bot.Run();
         * ```
         *
         * @return int, currently only returns zero.
         */

        WebSocketStart();
        while (true) {
            for (size_t i = 0; i < futures.size(); i++) {
                if (!futures[i].valid() ||
                    !(futures[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready)) {
                    continue;
                }
                futures.erase(futures.begin() + i);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return 0;
    }

    discpp::Guild Bot::GetGuild(snowflake guild_id) {
        /**
         * @brief Gets a discpp::Guild from a guild id.
         *
         * This will throw a runtime exception if the guild is not found.
         *
         * ```cpp
         *      discpp::Guild guild = bot.GetGuild(583251190591258624);
         * ```
         *
         * @param[in] guild_id The guild id of the guild you want to get.
         *
         * @return discpp::Guild
         */

        std::unordered_map<snowflake, Guild>::iterator it = discpp::globals::bot_instance->guilds.find(guild_id);
        if (it != discpp::globals::bot_instance->guilds.end()) {
            return it->second;
        }
        throw std::runtime_error("Guild not found!");
    }

    discpp::User Bot::ModifyCurrentUser(std::string username) {
        /**
         * @brief Modify the bot's username.
         *
         * ```cpp
         *      discpp::User user = bot.ModifyCurrent("New bot name!");
         * ```
         *
         * @param[in] username The new username.
         *
         * @return discpp::User
         */

        nlohmann::json result = SendPatchRequest(Endpoint("/users/@me"), DefaultHeaders(), 0,
                                                 discpp::RateLimitBucketType::GLOBAL);

        bot_user = discpp::User(result);

        return bot_user;
    }

    void Bot::LeaveGuild(discpp::Guild guild) {
        /**
         * @brief Leave the guild
         *
         * ```cpp
         *      bot.LeaveGuild(guild);
         * ```
         *
         * @param[in] guild The guild the bot will be leaving.
         *
         * @return void
         */

        SendDeleteRequest(Endpoint("/users/@me/guilds/" + guild.id), DefaultHeaders(), 0, RateLimitBucketType::GLOBAL);
    }

    void Bot::UpdatePresence(discpp::Activity activity) {
        /**
         * @brief Updates the bot's presence.
         *
         * ```cpp
         *      bot.UpdatePresence(activity);
         * ```
         *
         * @param[in] activity The presence the bot will update to.
         *
         * @return void
         */

        nlohmann::json payload = nlohmann::json({
                                                        {"op", 3},
                                                        {"d",  activity.ToJson()}
                                                });

        CreateWebsocketRequest(payload);
    }

    void discpp::Bot::CreateWebsocketRequest(nlohmann::json json) {
        /**
         * @brief Send a request to the websocket.
         *
         * Be cautious with this as it may close the websocket connection if it is invalid.
         *
         * ```cpp
         *      bot.CreateWebsocketRequest(request_json);
         * ```
         *
         * @param[in] json The request to send to the websocket.
         *
         * @return void
         */

        logger.Log(LogSeverity::SEV_DEBUG, "Sending gateway payload: " + json.dump());
        WaitForRateLimits(bot_user.id, RateLimitBucketType::GLOBAL);


        websocket_outgoing_message msg;
        msg.set_utf8_message(json.dump());

        auto threadlock = std::lock_guard(websocket_client_mutex);
        websocket_client.send(msg);
    }

    void Bot::SetCommandHandler(std::function<void(discpp::Bot *, discpp::Message)> command_handler) {
        /**
         * @brief Change the command handler.
         *
         * This is used in case you wanted to add functionality to the command handler.
         *
         * ```cpp
         *      bot.SetCommandHandler(std::bind(&my_discpp_bot::command_handler::HandleCommands, std::placeholders::_1, std::placeholders::_2));
         * ```
         *
         * @param[in] command_handler The method that will handle commands from a user.
         *
         * @return void
         */

        fire_command_method = command_handler;
    }

    void Bot::DisconnectWebsocket() {
        logger.Log(LogSeverity::SEV_DEBUG, LogTextColor::YELLOW + "Locking Mutex before disconnect...");
        std::lock_guard<std::mutex> lck(websocket_client_mutex);
        logger.Log(LogSeverity::SEV_DEBUG, LogTextColor::YELLOW + "Closing websocket connection...");
        websocket_client.close(web::websockets::client::websocket_close_status::server_terminate).wait();
    }

    void Bot::BindEvents() {
        internal_event_map["READY"] = std::bind(&Bot::ReadyEvent, this, std::placeholders::_1);
        internal_event_map["RESUMED"] = std::bind(&discpp::Bot::ResumedEvent, this, std::placeholders::_1);
        internal_event_map["INVALID_SESSION"] = std::bind(&discpp::Bot::InvalidSessionEvent, this, std::placeholders::_1);
        internal_event_map["CHANNEL_CREATE"] = std::bind(&discpp::Bot::ChannelCreateEvent, this, std::placeholders::_1);
        internal_event_map["CHANNEL_UPDATE"] = std::bind(&discpp::Bot::ChannelUpdateEvent, this, std::placeholders::_1);
        internal_event_map["CHANNEL_DELETE"] = std::bind(&discpp::Bot::ChannelDeleteEvent, this, std::placeholders::_1);
        internal_event_map["CHANNEL_PINS_UPDATE"] = std::bind(&discpp::Bot::ChannelPinsUpdateEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_CREATE"] = std::bind(&discpp::Bot::GuildCreateEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_UPDATE"] = std::bind(&discpp::Bot::GuildUpdateEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_DELETE"] = std::bind(&discpp::Bot::GuildDeleteEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_BAN_ADD"] = std::bind(&discpp::Bot::GuildBanAddEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_BAN_REMOVE"] = std::bind(&discpp::Bot::GuildBanRemoveEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_EMOJIS_UPDATE"] = std::bind(&discpp::Bot::GuildEmojisUpdateEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_INTEGRATIONS_UPDATE"] = std::bind(&discpp::Bot::GuildIntegrationsUpdateEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_MEMBER_ADD"] = std::bind(&discpp::Bot::GuildMemberAddEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_MEMBER_REMOVE"] = std::bind(&discpp::Bot::GuildMemberRemoveEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_MEMBER_UPDATE"] = std::bind(&discpp::Bot::GuildMemberUpdateEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_MEMBERS_CHUNK"] = std::bind(&discpp::Bot::GuildMembersChunkEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_ROLE_CREATE"] = std::bind(&discpp::Bot::GuildRoleCreateEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_ROLE_UPDATE"] = std::bind(&discpp::Bot::GuildRoleUpdateEvent, this, std::placeholders::_1);
        internal_event_map["GUILD_ROLE_DELETE"] = std::bind(&discpp::Bot::GuildRoleDeleteEvent, this, std::placeholders::_1);
        internal_event_map["MESSAGE_CREATE"] = std::bind(&discpp::Bot::MessageCreateEvent, this, std::placeholders::_1);
        internal_event_map["MESSAGE_UPDATE"] = std::bind(&discpp::Bot::MessageUpdateEvent, this, std::placeholders::_1);
        internal_event_map["MESSAGE_DELETE"] = std::bind(&discpp::Bot::MessageDeleteEvent, this, std::placeholders::_1);
        internal_event_map["MESSAGE_DELETE_BULK"] = std::bind(&discpp::Bot::MessageDeleteBulkEvent, this, std::placeholders::_1);
        internal_event_map["MESSAGE_REACTION_ADD"] = std::bind(&discpp::Bot::MessageReactionAddEvent, this, std::placeholders::_1);
        internal_event_map["MESSAGE_REACTION_REMOVE"] = std::bind(&discpp::Bot::MessageReactionRemoveEvent, this, std::placeholders::_1);
        internal_event_map["MESSAGE_REACTION_REMOVE_ALL"] = std::bind(&discpp::Bot::MessageReactionRemoveAllEvent, this, std::placeholders::_1);
        internal_event_map["PRESENCE_UPDATE"] = std::bind(&discpp::Bot::PresenceUpdateEvent, this, std::placeholders::_1);
        internal_event_map["TYPING_START"] = std::bind(&discpp::Bot::TypingStartEvent, this, std::placeholders::_1);
        internal_event_map["USER_UPDATE"] = std::bind(&discpp::Bot::UserUpdateEvent, this, std::placeholders::_1);
        internal_event_map["VOICE_STATE_UPDATE"] = std::bind(&discpp::Bot::VoiceStateUpdateEvent, this, std::placeholders::_1);
        internal_event_map["VOICE_SERVER_UPDATE"] = std::bind(&discpp::Bot::VoiceServerUpdateEvent, this, std::placeholders::_1);
        internal_event_map["WEBHOOKS_UPDATE"] = std::bind(&discpp::Bot::WebhooksUpdateEvent, this, std::placeholders::_1);
    }

    void Bot::WebSocketStart() {
        nlohmann::json gateway_request = SendGetRequest(Endpoint("/gateway/bot"), {{"Authorization", "Bot " + token},
                                                                                   {"User-Agent",    "discppBot (https://github.com/seanomik/DISCPP, v0.0.0)"}},
                                                        {}, {});

        if (gateway_request.contains("url")) {
            logger.Log(LogSeverity::SEV_DEBUG, LogTextColor::YELLOW + "Connecting to gateway...");

            if (gateway_request["session_start_limit"]["remaining"].get<int>() == 0) {
                logger.Log(LogSeverity::SEV_ERROR, LogTextColor::RED + "GATEWAY ERROR: Maximum start limit reached");
                throw std::runtime_error{"GATEWAY ERROR: Maximum start limit reached"};
            }

            // Specify version and encoding just ot be safe
            gateway_endpoint = gateway_request["url"].get<std::string>() + "?v=6&encoding=json";

            std::thread bindthread{&Bot::BindEvents, this};
            utility::string_t stringt = utility::conversions::to_string_t(gateway_endpoint);
            auto threadlock = std::lock_guard(websocket_client_mutex);

            // Recreate a websocket client just incase we're trying to reconnect.
            if (reconnecting) {
                websocket_client = websocket_callback_client();
            }

            websocket_client.connect(web::uri(stringt));
            websocket_client.set_message_handler(std::bind(&Bot::OnWebSocketPacket, this, std::placeholders::_1));
            websocket_client.set_close_handler(
                    std::bind(&Bot::HandleDiscordDisconnect, this, std::placeholders::_1, std::placeholders::_2,
                              std::placeholders::_3));

            disconnected = false;

            logger.Log(LogSeverity::SEV_INFO, LogTextColor::GREEN + "Connected to gateway!");
            bindthread.join();
        } else {
            logger.Log(LogSeverity::SEV_ERROR,
                       LogTextColor::RED + "Improper token, failed to connect to discpp gateway!");
            throw std::runtime_error("Improper token, failed to connect to discpp gateway!");
        }
    }

    void Bot::HandleDiscordDisconnect(websocket_close_status close_status, utility::string_t reason,
                                      std::error_code error_code) {
        logger.Log(LogSeverity::SEV_ERROR,
                   LogTextColor::RED + "Websocket was closed with error: 400" + std::to_string(error_code.value()) +
                   "! Attemping reconnect in 10 seconds...");
        heartbeat_acked = false;
        disconnected = true;
        reconnecting = true;

        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        if (disconnected && !reconnecting) {
            ReconnectToWebsocket();
        }
    }

    void Bot::OnWebSocketPacket(websocket_incoming_message msg) {
        std::string packet_raw = msg.extract_string().get();

        nlohmann::json result = nlohmann::json::parse(packet_raw);
        logger.Log(LogSeverity::SEV_DEBUG, "Received payload: " + result.dump());

        switch (result["op"].get<int>()) {
            case (hello): {
                if (reconnecting) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
                    logger.Log(LogSeverity::SEV_INFO, LogTextColor::GREEN + "Reconnected!");

                    std::string resume =
                            "{ \"op\": 6, \"d\": { \"token\": \"" + token + "\", \"session_id\": \"" + session_id +
                            "\", \"seq\": " + std::to_string(last_sequence_number) + "} }";
                    CreateWebsocketRequest(nlohmann::json::parse(resume));

                    // Heartbeat just to be safe
                    nlohmann::json data = {{"op", packet_opcode::heartbeat},
                                           {"d",  nullptr}};
                    if (last_sequence_number != -1) {
                        data["d"] = last_sequence_number;
                    }

                    reconnecting = false;
                } else {
                    logger.Log(LogSeverity::SEV_DEBUG, "Sending gateway payload: " + GetIdentifyPacket().dump());

                    hello_packet = result;
                    websocket_outgoing_message identify_msg;
                    identify_msg.set_utf8_message(GetIdentifyPacket().dump());

                    websocket_client_mutex.lock();
                    websocket_client.send(identify_msg);
                    websocket_client_mutex.unlock();
                }
                break;
            }
            case heartbeat_ack:
                heartbeat_acked = true;
                break;
            case reconnect:
                ReconnectToWebsocket();
                break;
            case invalid_session:
                // Check if the session is resumable
                if (result["d"].get<bool>()) {
                    std::string resume =
                            "{ \"op\": 6, \"d\": { \"token\": \"" + token + "\", \"session_id\": \"" + session_id +
                            "\", \"seq\": " + std::to_string(last_sequence_number) + "} }";
                    CreateWebsocketRequest(nlohmann::json::parse(resume));
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    CreateWebsocketRequest(GetIdentifyPacket());
                }

                break;
            default:
                HandleDiscordEvent(result, result["t"]);
                break;
        }

        packet_counter++;
    }

    void Bot::HandleDiscordEvent(nlohmann::json const j, std::string event_name) {
        const nlohmann::json data = j["d"];
        last_sequence_number = (j.contains("s") && j["s"].is_number()) ? j["s"].get<int>() : -1;

        if (internal_event_map.find(event_name) != internal_event_map.end()) {
            if (ready) {
                internal_event_map[event_name](data);
            } else {
                futures.push_back(std::async(std::launch::async, internal_event_map[event_name], data));
            }
        }
    }

    void Bot::HandleHeartbeat() {
        try {
            while (true) {
                // Make sure that it doesn't try to do anything while its trying to reconnect.
                while (reconnecting) {}

                nlohmann::json data = {{"op", packet_opcode::heartbeat},
                                       {"d",  nullptr}};
                if (last_sequence_number != -1) {
                    data["d"] = last_sequence_number;
                }

                logger.Log(LogSeverity::SEV_DEBUG, "Sending heartbeat payload: " + data.dump());

                websocket_client_mutex.lock();
                websocket_outgoing_message msg;
                msg.set_utf8_message(data.dump());
                websocket_client.send(msg);
                websocket_client_mutex.unlock();

                heartbeat_acked = false;

                logger.Log(LogSeverity::SEV_DEBUG, "Waiting for next heartbeat (" + std::to_string(
                        hello_packet["d"]["heartbeat_interval"].get<int>() / 1000.0 - 10) + " seconds)...");
                // Wait for the required heartbeat interval, while waiting it should be acked from another thread.
                std::this_thread::sleep_for(
                        std::chrono::milliseconds(hello_packet["d"]["heartbeat_interval"].get<int>() - 10));

                if (!heartbeat_acked) {
                    logger.Log(LogSeverity::SEV_WARNING,
                               LogTextColor::YELLOW + "Heartbeat wasn't acked, trying to reconnect...");
                    disconnected = true;

                    ReconnectToWebsocket();
                }
            }
        } catch (std::exception &e) {
            logger.Log(LogSeverity::SEV_ERROR, LogTextColor::RED + "ERROR: " + e.what());
        }
    }

    nlohmann::json Bot::GetIdentifyPacket() {
        nlohmann::json obj = {{"op", packet_opcode::identify},
                              {"d",
                                     {{"token", token},
                                             {"properties",
                                                     {{"$os", GetOsName()},
                                                             {"$browser", "DISCPP"},
                                                             {"$device", "DISCPP"}}},
                                             {"compress", false},
                                             {"large_threshold", 250}}}};
        return obj;
    }

    void Bot::ReconnectToWebsocket() {
        logger.Log(LogSeverity::SEV_INFO, LogTextColor::YELLOW + "Reconnecting to Discord gateway!");

        reconnecting = true;

        DisconnectWebsocket();
        WebSocketStart();
    }

    void Bot::ReadyEvent(nlohmann::json result) {
        // Check if we're just resuming, and if we are dont try to create a new thread.
        if (!heartbeat_thread.joinable()) {
            heartbeat_thread = std::thread{&Bot::HandleHeartbeat, this};
        }

        ready = true;
        session_id = result["session_id"];

        // Get the bot user
        nlohmann::json user_json = SendGetRequest(Endpoint("/users/@me"), DefaultHeaders(), {}, {});
        bot_user = discpp::User(user_json);

        discpp::DispatchEvent(discpp::ReadyEvent());
    }

    void Bot::ResumedEvent(nlohmann::json result) {
        discpp::DispatchEvent(discpp::ResumedEvent());
    }

    void Bot::ReconnectEvent(nlohmann::json result) {
        discpp::DispatchEvent(discpp::ReconnectEvent());
    }

    void Bot::InvalidSessionEvent(nlohmann::json result) {
        discpp::DispatchEvent(discpp::InvalidSessionEvent());
    }

    void Bot::ChannelCreateEvent(nlohmann::json result) {
        discpp::Channel new_channel = discpp::Channel(result, result["id"].get<snowflake>());
        channels.insert(std::pair<snowflake, Channel>(static_cast<snowflake>(new_channel.id),
                                                      static_cast<Channel>(new_channel)));

        discpp::DispatchEvent(discpp::ChannelCreateEvent(new_channel));
    }

    void Bot::ChannelUpdateEvent(nlohmann::json result) {
        discpp::Channel new_channel = discpp::Channel(result);

        std::unordered_map<snowflake, Channel>::iterator it = channels.find(new_channel.id);
        if (it != channels.end()) {
            it->second = new_channel;
        }

        discpp::DispatchEvent(discpp::ChannelUpdateEvent(new_channel));
    }

    void Bot::ChannelDeleteEvent(nlohmann::json result) {
        channels.erase(result["id"].get<snowflake>());

        discpp::DispatchEvent(discpp::ChannelDeleteEvent(discpp::Channel(result)));
    }

    void Bot::ChannelPinsUpdateEvent(nlohmann::json result) {
        discpp::Channel new_channel = discpp::Channel(result["channel_id"].get<snowflake>());
        new_channel.last_pin_timestamp = GetDataSafely<std::string>(result, "last_pin_timestamp");
        new_channel.guild_id = result["guild_id"].get<snowflake>();

        std::unordered_map<snowflake, Channel>::iterator it = channels.find(new_channel.id);
        if (it != channels.end()) {
            it->second = new_channel;
        }

        discpp::DispatchEvent(discpp::ChannelPinsUpdateEvent(new_channel));
    }

    void Bot::GuildCreateEvent(nlohmann::json result) {
        snowflake guild_id = result["id"].get<snowflake>();
        discpp::Guild guild(result);
        logger.LogToConsole(LogSeverity::SEV_INFO, LogTextColor::GREEN + "Connected to " + guild.name);
        guilds.insert(std::pair<snowflake, Guild>(static_cast<snowflake>(guild.id), static_cast<Guild>(guild)));

        members.insert(guild.members.begin(), guild.members.end());

        for (auto &channel : result["channels"]) {
            discpp::Channel _channel = Channel(channel, guild_id);
            channels.insert(
                    std::pair<snowflake, Channel>(static_cast<snowflake>(_channel.id), static_cast<Channel>(_channel)));
        }

        discpp::DispatchEvent(discpp::GuildCreateEvent(guild));
    }

    void Bot::GuildUpdateEvent(nlohmann::json result) {
        discpp::Guild guild(result);
        std::unordered_map<snowflake, Guild>::iterator it = guilds.find(guild.id);
        if (it != guilds.end()) {
            it->second = guild;
        }
        discpp::DispatchEvent(discpp::GuildUpdateEvent(guild));
    }

    void Bot::GuildDeleteEvent(nlohmann::json result) {
        discpp::Guild guild;
        guild.id = result["id"].get<snowflake>();
        guild.unavailable = true;
        guilds.erase(guild.id);

        discpp::DispatchEvent(discpp::GuildDeleteEvent(guild));
    }

    void Bot::GuildBanAddEvent(nlohmann::json result) {
        discpp::Guild guild(result["guild_id"].get<snowflake>());
        discpp::User user(result["user"]);

        discpp::DispatchEvent(discpp::GuildBanAddEvent(guild, user));
    }

    void Bot::GuildBanRemoveEvent(nlohmann::json result) {
        discpp::Guild guild(result["guild_id"].get<snowflake>());
        discpp::User user(result["user"]);

        discpp::DispatchEvent(discpp::GuildBanRemoveEvent(guild, user));
    }

    void Bot::GuildEmojisUpdateEvent(nlohmann::json result) {
        discpp::Guild guild(result["guild_id"].get<snowflake>());
        std::unordered_map<snowflake, Emoji> emojis;
        for (nlohmann::json emoji : result["emojis"]) {
            discpp::Emoji tmp = discpp::Emoji(emoji);
            emojis.insert(std::pair<snowflake, Emoji>(static_cast<snowflake>(tmp.id), static_cast<Emoji>(tmp)));
        }

        guild.emojis = emojis;
        std::unordered_map<snowflake, Guild>::iterator it = discpp::globals::bot_instance->guilds.find(guild.id);
        if (it != discpp::globals::bot_instance->guilds.end()) {
            it->second = guild;
        }

        discpp::DispatchEvent(discpp::GuildEmojisUpdateEvent(guild));
    }

    void Bot::GuildIntegrationsUpdateEvent(nlohmann::json result) {
        discpp::DispatchEvent(
                discpp::GuildIntegrationsUpdateEvent(discpp::Guild(result["guild_id"].get<snowflake>())));
    }

    void Bot::GuildMemberAddEvent(nlohmann::json result) {
        discpp::Guild guild(result["guild_id"].get<snowflake>());
        discpp::Member member(result, guild.id);
        members.insert(std::pair<snowflake, Member>(static_cast<snowflake>(member.id), static_cast<Member>(member)));

        discpp::DispatchEvent(discpp::GuildMemberAddEvent(guild, member));
    }

    void Bot::GuildMemberRemoveEvent(nlohmann::json result) {
        discpp::Guild guild(result["guild_id"].get<snowflake>());
        discpp::Member member(result["user"]["id"].get<snowflake>());
        members.erase(member.id);

        discpp::DispatchEvent(discpp::GuildMemberRemoveEvent(guild, member));
    }

    void Bot::GuildMemberUpdateEvent(nlohmann::json result) {
        discpp::Guild guild(result["guild_id"].get<snowflake>());
        std::unordered_map<snowflake, Member>::iterator it = guild.members.find(result["user"]["id"]);
        discpp::Member member;
        if (it != guild.members.end()) {
            member = it->second;
        }
        else {
            member = discpp::Member(result["user"]["id"].get<snowflake>());
            guild.members.insert(std::pair<snowflake, Member>(static_cast<snowflake>(member.id), static_cast<discpp::Member>(member)));
        }
        member.roles.clear();
        for (auto role : result["roles"]) {
            member.roles.push_back(discpp::Role(role, guild));
        }
        if (result.contains("nick") && !result["nick"].is_null()) {
            member.nick = result["nick"];
        }
        discpp::DispatchEvent(discpp::GuildMemberUpdateEvent(guild, member));
    }

    void Bot::GuildMembersChunkEvent(nlohmann::json result) {
        discpp::DispatchEvent(discpp::GuildMembersChunkEvent());
    }

    void Bot::GuildRoleCreateEvent(nlohmann::json result) {
        discpp::Role role(result["role"]);

        discpp::DispatchEvent(discpp::GuildRoleCreateEvent(role));
    }

    void Bot::GuildRoleUpdateEvent(nlohmann::json result) {
        discpp::Role role(result["role"]);

        discpp::DispatchEvent(discpp::GuildRoleUpdateEvent(role));
    }

    void Bot::GuildRoleDeleteEvent(nlohmann::json result) {
        discpp::Guild guild(result["guild_id"].get<snowflake>());
        discpp::Role role(result["role"]);
        guild.roles.erase(role.id);

        discpp::DispatchEvent(discpp::GuildRoleDeleteEvent(role));
    }

    void Bot::MessageCreateEvent(nlohmann::json result) {
        discpp::Message message(result);
        if (messages.size() >= message_cache_count) {
            messages.erase(messages.begin());
        }
        messages.insert({message.id, message});

        DoFunctionLater(fire_command_method, this, message);
        DispatchEvent(discpp::MessageCreateEvent(message));
    }

    void Bot::MessageUpdateEvent(nlohmann::json result) {
        auto message = messages.find(result["id"].get<snowflake>());

        if (message != messages.end()) {
            if (messages.size() >= message_cache_count) {
                messages.erase(messages.begin());
            }

            discpp::Message old_message(result["id"].get<snowflake>());
            /*std::replace_if(messages.begin(), messages.end(),
                            [message](discpp::Message &msg) { return msg.id == message.id; }, message);*/
            bool is_edited = !result["edited_timestamp"].empty();

            discpp::DispatchEvent(discpp::MessageUpdateEvent(message->second, old_message, is_edited));
        }
    }

    void Bot::MessageDeleteEvent(nlohmann::json result) {
        auto message = messages.find(result["id"].get<snowflake>());

        if (message != messages.end()) {
            discpp::DispatchEvent(discpp::MessageDeleteEvent(message->second));

            messages.erase(message);
        }
    }

    void Bot::MessageDeleteBulkEvent(nlohmann::json result) {
        std::vector<discpp::Message> msgs;
        for (auto id : result["ids"]) {
            auto message = messages.find(id.get<snowflake>());

            if (message != messages.end()) {
                message->second.channel = discpp::Channel(result["channel_id"].get<snowflake>());
                if (result.contains("guild_id")) {
                    message->second.guild = discpp::Guild(result["guild_id"].get<snowflake>());
                }

                msgs.push_back(message->second);
            }
        }

        for (discpp::Message message : msgs) {
            messages.erase(message.id);
        }

        discpp::DispatchEvent(discpp::MessageBulkDeleteEvent(msgs));
    }

    void Bot::MessageReactionAddEvent(nlohmann::json result) {
        auto message = messages.find(result["message_id"].get<snowflake>());

        if (message != messages.end()) {
            discpp::Channel channel = channels.find(result["channel_id"].get<snowflake>())->second;
            message->second.channel = channel;

            if (result.contains("guild_id")) {
                message->second.guild = discpp::Guild(result["guild_id"].get<snowflake>());
                channel.guild_id = result["guild_id"].get<snowflake>();
            }

            discpp::Emoji emoji(result["emoji"]);
            discpp::User user(result["user_id"].get<snowflake>());

            auto reaction = std::find_if(message->second.reactions.begin(), message->second.reactions.end(),
                                         [emoji](discpp::Reaction react) {
                                             return react.emoji.name == emoji.name || (!react.emoji.id.empty() &&
                                                                                       react.emoji.id == emoji.id);
                                         });

            if (reaction != message->second.reactions.end()) {
                reaction->count++;

                if (user.bot) {
                    reaction->from_bot = true;
                }
            } else {
                discpp::Reaction r = discpp::Reaction(1, user.bot, emoji);
                message->second.reactions.push_back(r);
            }

            discpp::DispatchEvent(discpp::MessageReactionAddEvent(message->second, emoji, user));
        }
    }

    void Bot::MessageReactionRemoveEvent(nlohmann::json result) {
        auto message = messages.find(result["message_id"].get<snowflake>());

        if (message != messages.end()) {
            discpp::Channel channel = channels.find(result["channel_id"].get<snowflake>())->second;
            message->second.channel = channel;

            if (result.contains("guild_id")) {
                channel.guild_id = result["guild_id"].get<snowflake>();
            }

            discpp::Emoji emoji(result["emoji"]);
            discpp::User user(result["user_id"].get<snowflake>());

            auto reaction = std::find_if(message->second.reactions.begin(), message->second.reactions.end(),
                                         [emoji](discpp::Reaction react) {
                                             return react.emoji.name == emoji.name || react.emoji.id == emoji.id;
                                         });

            if (reaction != message->second.reactions.end()) {
                if (reaction->count == 1) {
                    message->second.reactions.erase(reaction);
                } else {
                    reaction->count--;

                    // @TODO: Add a way to change reaction::from_bot
                }
            }

            discpp::DispatchEvent(discpp::MessageReactionRemoveEvent(message->second, emoji, user));
        }
    }

    void Bot::MessageReactionRemoveAllEvent(nlohmann::json result) {
        auto message = messages.find(result["message_id"].get<snowflake>());

        if (message != messages.end()) {
            discpp::Channel channel = channels.find(result["channel_id"].get<snowflake>())->second;
            message->second.channel = channel;

            if (result.contains("guild_id")) {
                channel.guild_id = result["guild_id"].get<snowflake>();
            }

            discpp::DispatchEvent(discpp::MessageReactionRemoveAllEvent(message->second));
        }
    }

    void Bot::PresenceUpdateEvent(nlohmann::json result) {
        discpp::DispatchEvent(discpp::PresenseUpdateEvent(discpp::User(result["user"])));
    }

    void Bot::TypingStartEvent(nlohmann::json result) {
        discpp::User user(result["user_id"].get<snowflake>());
        discpp::Channel channel(result["channel_id"].get<snowflake>());
        if (result.contains("guild_id")) channel.guild_id = result["guild_id"].get<snowflake>();
        int timestamp = result["timestamp"].get<int>();

        discpp::DispatchEvent(discpp::TypingStartEvent(user, channel, timestamp));
    }

    void Bot::UserUpdateEvent(nlohmann::json result) {
        discpp::User user(result);

        discpp::DispatchEvent(discpp::UserUpdateEvent(user));
    }

    void Bot::VoiceStateUpdateEvent(nlohmann::json result) {
        discpp::DispatchEvent(discpp::VoiceStateUpdateEvent(result));
    }

    void Bot::VoiceServerUpdateEvent(nlohmann::json result) {
        discpp::DispatchEvent(discpp::VoiceServerUpdateEvent(result));
    }

    void Bot::WebhooksUpdateEvent(nlohmann::json result) {
        discpp::Channel channel(result["channel_id"].get<snowflake>());
        channel.guild_id = result["guild_id"].get<snowflake>();

        discpp::DispatchEvent(discpp::WebhooksUpdateEvent(channel));
    }
}