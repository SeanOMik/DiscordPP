#ifndef DISCPP_INTENTS_H
#define DISCPP_INTENTS_H

#include <stdint.h>

namespace discpp {
    enum GatewayIntents: uint64_t {
        /* GUILD_CREATE
         * GUILD_UPDATE
         * GUILD_DELETE
         * GUILD_ROLE_CREATE
         * GUILD_ROLE_UPDATE
         * GUILD_ROLE_DELETE
         * CHANNEL_CREATE
         * CHANNEL_UPDATE
         * CHANNEL_DELETE
         * CHANNEL_PINS_UPDATE */
        GUILDS = 1 << 0,

        /* Privileged intent
         * GUILD_MEMBER_ADD
         * GUILD_MEMBER_UPDATE
         * GUILD_MEMBER_REMOVE */
        GUILD_MEMBERS = 1 << 1,

        /* GUILD_BAN_ADD
         * GUILD_BAN_REMOVE */
        GUILD_BANS = 1 << 2,

        // GUILD_EMOJIS_UPDATE
        GUILD_EMOJIS = 1 << 3,

        // GUILD_INTEGRATIONS_UPDATE
        GUILD_INTEGRATIONS = 1 << 4,

        // WEBHOOKS_UPDATE
        GUILD_WEBHOOKS = 1 << 5,
        
        /* INVITE_CREATE
         * INVITE_DELETE */
        GUILD_INVITES = 1 << 6,

        // VOICE_STATE_UPDATE
        GUILD_VOICE_STATES = 1 << 7,

        /* Privileged intent
         * PRESENCE_UPDATE */
        GUILD_PRESENCES = 1 << 8,

        /* MESSAGE_CREATE
         * MESSAGE_UPDATE
         * MESSAGE_DELETE
         * MESSAGE_DELETE_BULK */
        GUILD_MESSAGES = 1 << 9,

        /* MESSAGE_REACTION_ADD
         * MESSAGE_REACTION_REMOVE
         * MESSAGE_REACTION_REMOVE_ALL
         * MESSAGE_REACTION_REMOVE_EMOJI */
        GUILD_MESSAGE_REACTIONS = 1 << 10,

        // TYPING_START
        GUILD_MESSAGE_TYPING = 1 << 11,

        /* MESSAGE_CREATE
        * MESSAGE_UPDATE
        * MESSAGE_DELETE
        * CHANNEL_PINS_UPDATE */
        DIRECT_MESSAGES = 1 << 12,

        /* MESSAGE_REACTION_ADD
         * MESSAGE_REACTION_REMOVE
         * MESSAGE_REACTION_REMOVE_ALL
         * MESSAGE_REACTION_REMOVE_EMOJI */
        DIRECT_MESSAGE_REACTIONS = 1 << 13,

        // TYPING_START
        DIRECT_MESSAGE_TYPING = 1 << 14,

        NON_PRIVILAGED = GUILDS | GUILD_BANS | GUILD_EMOJIS | GUILD_INTEGRATIONS | GUILD_WEBHOOKS | GUILD_INVITES 
            | GUILD_VOICE_STATES | GUILD_MESSAGES | GUILD_MESSAGE_REACTIONS | GUILD_MESSAGE_TYPING | DIRECT_MESSAGES
            | DIRECT_MESSAGE_REACTIONS | DIRECT_MESSAGE_TYPING,

        ALL = GUILDS | GUILD_MEMBERS | GUILD_BANS | GUILD_EMOJIS | GUILD_INTEGRATIONS | GUILD_WEBHOOKS | GUILD_INVITES 
            | GUILD_VOICE_STATES | GUILD_PRESENCES | GUILD_MESSAGES | GUILD_MESSAGE_REACTIONS | GUILD_MESSAGE_TYPING
            | DIRECT_MESSAGES | DIRECT_MESSAGE_REACTIONS | DIRECT_MESSAGE_TYPING
    };
}

#endif