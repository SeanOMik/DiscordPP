#ifndef DISCPP_VOICE_STATE_UPDATE_EVENT_H
#define DISCPP_VOICE_STATE_UPDATE_EVENT_H

#include "../event.h"
#include "../user.h"

#include <nlohmann/json.hpp>

namespace discpp {
	class VoiceStateUpdateEvent : public Event {
	public:
		inline VoiceStateUpdateEvent(nlohmann::json json) : json(json) { }

		nlohmann::json json;
	};
}

#endif