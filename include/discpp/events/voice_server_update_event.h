#ifndef DISCPP_VOICE_SERVER_UPDATE_EVENT_H
#define DISCPP_VOICE_SERVER_UPDATE_EVENT_H

#include "../event.h"
#include "../user.h"

#include <nlohmann/json.hpp>

namespace discpp {
	class VoiceServerUpdateEvent : public Event {
	public:
		inline VoiceServerUpdateEvent(nlohmann::json json) : json(json) { }

		nlohmann::json json;
	};
}

#endif