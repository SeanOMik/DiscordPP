#ifndef DISCPP_MESSAGE_BULK_DELETE_EVENT_H
#define DISCPP_MESSAGE_BULK_DELETE_EVENT_H

#include "../event.h"
#include "../message.h"
#include "../bot.h"

#include <vector>

#include <nlohmann/json.hpp>

namespace discpp {
	class MessageBulkDeleteEvent : public Event {
	public:
		inline MessageBulkDeleteEvent(std::vector<discpp::Message> message) : messages(messages) {}

		std::vector<discpp::Message> messages;
	};
}

#endif