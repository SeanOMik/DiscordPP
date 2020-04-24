#ifndef DISCPP_CONTEXT_H
#define DISCPP_CONTEXT_H

#include <vector>
#include <string>

#include "bot.h"
#include "guild.h"
#include "channel.h"
#include "user.h"
#include "member.h"
#include "message.h"

namespace discpp {

	class Context {
	private:

	public:
		discpp::Bot* bot;
		discpp::Guild guild;
		discpp::Channel channel;
		discpp::User user;
		discpp::Member author;
		discpp::Message message;
		std::string remainder;
		std::vector<std::string> arguments;

		Context(discpp::Bot* bot, discpp::Channel channel, discpp::Member author, discpp::Message message, std::string remainder, std::vector<std::string> arguments);

		discpp::Message Send(std::string text, bool tts = false, discpp::EmbedBuilder* embed = nullptr, std::vector<discpp::File> files = {});
	};
}

#endif