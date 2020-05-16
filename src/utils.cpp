#include "utils.h"
#include "client.h"
#include "client_config.h"

#include <stdlib.h>
#include <numeric>

std::string discpp::GetOsName() {
	/**
	 * @brief Get the OS name this application is running on.
	 *
	 * ```cpp
	 *      std::string os_name = discpp::GetOSName();
	 * ```
	 *
	 * @return std::string
	 */

	#ifdef _WIN32
		return "Windows 32-bit";
	#elif _WIN64
		return "Windows 64-bit";
        #elif __APPLE__ || __MACH__
                return "Mac OSX";
        #elif __linux__
            return "Linux";
        #elif __FreeBSD__
            return "FreeBSD";
        #elif __unix || __unix__
            return "Unix";
        #else
            return "Other";
	#endif
}

// @TODO: Test if the json document type returned is what its supposed to be, like an array or object.
rapidjson::Document discpp::HandleResponse(cpr::Response response, snowflake object, RateLimitBucketType ratelimit_bucket) {
	/**
	 * @brief Handles a response from the discpp servers.
	 *
	 * ```cpp
	 *      rapidjson::Document response = discpp::HandleResponse(cpr_response, object, discpp::RateLimitBucketType::CHANNEL);
	 * ```
	 *
	 * @param[in] reponse The cpr response from the servers.
	 * @param[in] object The object id to handle the ratelimits for.
	 * @param[in] ratelimit_bucket The rate limit bucket.
	 *
	 * @return rapidjson::Document
	 */

    if (globals::client_instance != nullptr) {
        globals::client_instance->logger->Debug("Received requested payload: " + response.text);
    }

    rapidjson::Document tmp;
    if (!response.text.empty() && response.text[0] == '[' && response.text[response.text.size() - 1] == ']') {
        tmp.SetArray();
    } else {
        tmp.SetObject();
    }

	HandleRateLimits(response.header, object, ratelimit_bucket);
	tmp.Parse((!response.text.empty() ? response.text.c_str() : "{}"));

    // This shows an error in inteliisense for some reason but compiles fine.
#ifndef __INTELLISENSE__
    return std::move(tmp);
#endif
}

std::string CprBodyToString(cpr::Body body) {
	if (body.empty()) {
		return "Empty";
	}
	
	return body;
}

rapidjson::Document discpp::SendGetRequest(std::string url, cpr::Header headers, snowflake object, RateLimitBucketType ratelimit_bucket, cpr::Body body) {
	/**
	 * @brief Sends a get request to a url.
	 *
	 * ```cpp
	 *      rapidjson::Document response = discpp::SendGetRequest(url, discpp::DefaultHeaders(), object, discpp::RateLimitBucketType::CHANNEL, {});
	 * ```
	 *
	 * @param[in] url The url to create a request to.
	 * @param[in] headers The http header.
	 * @param[in] object The object id to handle the ratelimits for.
	 * @param[in] ratelimit_bucket The rate limit bucket.
	 * @param[in] The cpr response body.
	 *
	 * @return rapidjson::Document
	 */

    if (globals::client_instance != nullptr) {
        globals::client_instance->logger->Debug("Sending get request, URL: " + url + ", body: " + CprBodyToString(body));
    }
	WaitForRateLimits(object, ratelimit_bucket);
	cpr::Response result = cpr::Get(cpr::Url{ url }, headers, body);

	rapidjson::Document doc = HandleResponse(result, object, ratelimit_bucket);

    // This shows an error in inteliisense for some reason but compiles fine.
#ifndef __INTELLISENSE__
    return std::move(doc);
#endif
}

rapidjson::Document discpp::SendPostRequest(std::string url, cpr::Header headers, snowflake object, RateLimitBucketType ratelimit_bucket, cpr::Body body) {
	/**
	 * @brief Sends a post request to a url.
	 *
	 * ```cpp
	 *      rapidjson::Document response = discpp::SendPostRequest(url, discpp::DefaultHeaders(), object, discpp::RateLimitBucketType::CHANNEL, {});
	 * ```
	 *
	 * @param[in] url The url to create a request to.
	 * @param[in] headers The http header.
	 * @param[in] object The object id to handle the ratelimits for.
	 * @param[in] ratelimit_bucket The rate limit bucket.
	 * @param[in] The cpr response body.
	 *
	 * @return rapidjson::Document
	 */

    if (globals::client_instance != nullptr) {
        globals::client_instance->logger->Debug("Sending post request, URL: " + url + ", body: " + CprBodyToString(body));
    }
	WaitForRateLimits(object, ratelimit_bucket);
	cpr::Response result = cpr::Post(cpr::Url{ url }, headers, body);
	return HandleResponse(result, object, ratelimit_bucket);
}

rapidjson::Document discpp::SendPutRequest(std::string url, cpr::Header headers, snowflake object, RateLimitBucketType ratelimit_bucket, cpr::Body body) {
	/**
	 * @brief Sends a put request to a url.
	 *
	 * ```cpp
	 *      rapidjson::Document response = discpp::SendPutRequest(url, discpp::DefaultHeaders(), object, discpp::RateLimitBucketType::CHANNEL, {});
	 * ```
	 *
	 * @param[in] url The url to create a request to.
	 * @param[in] headers The http header.
	 * @param[in] object The object id to handle the ratelimits for.
	 * @param[in] ratelimit_bucket The rate limit bucket.
	 * @param[in] The cpr response body.
	 *
	 * @return rapidjson::Document
	 */

    if (globals::client_instance != nullptr) {
        globals::client_instance->logger->Debug("put patch request, URL: " + url + ", body: " + CprBodyToString(body));
    }
	WaitForRateLimits(object, ratelimit_bucket);
	cpr::Response result = cpr::Put(cpr::Url{ url }, headers, body);
	return HandleResponse(result, object, ratelimit_bucket);
}

rapidjson::Document discpp::SendPatchRequest(std::string url, cpr::Header headers, snowflake object, RateLimitBucketType ratelimit_bucket, cpr::Body body) {
	/**
	 * @brief Sends a patch request to a url.
	 *
	 * ```cpp
	 *      rapidjson::Document response = discpp::SendPatchRequest(url, discpp::DefaultHeaders(), object, discpp::RateLimitBucketType::CHANNEL, {});
	 * ```
	 *
	 * @param[in] url The url to create a request to.
	 * @param[in] headers The http header.
	 * @param[in] object The object id to handle the ratelimits for.
	 * @param[in] ratelimit_bucket The rate limit bucket.
	 * @param[in] The cpr response body.
	 *
	 * @return rapidjson::Document
	 */

	if (globals::client_instance != nullptr) {
        globals::client_instance->logger->Debug("Sending patch request, URL: " + url + ", body: " + CprBodyToString(body));
    }
	WaitForRateLimits(object, ratelimit_bucket);
	cpr::Response result = cpr::Patch(cpr::Url{ url }, headers, body);
	return HandleResponse(result, object, ratelimit_bucket);
}

rapidjson::Document discpp::SendDeleteRequest(std::string url, cpr::Header headers, snowflake object, RateLimitBucketType ratelimit_bucket) {
	/**
	 * @brief Sends a delete request to a url.
	 *
	 * ```cpp
	 *      rapidjson::Document response = discpp::SendDeleteRequest(url, discpp::DefaultHeaders(), object, discpp::RateLimitBucketType::CHANNEL);
	 * ```
	 *
	 * @param[in] url The url to create a request to.
	 * @param[in] headers The http header.
	 * @param[in] object The object id to handle the ratelimits for.
	 * @param[in] ratelimit_bucket The rate limit bucket.
	 *
	 * @return rapidjson::Document
	 */

    if (globals::client_instance != nullptr) {
        globals::client_instance->logger->Debug("Sending delete request, URL: " + url);
    }
	WaitForRateLimits(object, ratelimit_bucket);
	cpr::Response result = cpr::Delete(cpr::Url{ url }, headers);
	return HandleResponse(result, object, ratelimit_bucket);
}

cpr::Header discpp::DefaultHeaders(cpr::Header add) {
	/**
	 * @brief Gets the default headers to communicate with the discpp servers.
	 *
	 * ```cpp
	 *      cpr::Header default_headers = discpp::DefaultHeaders({});
	 * ```
	 *
	 * @param[in] add The headers to add to the default ones.
	 *
	 * @return rapidjson::Document
	 */


    cpr::Header headers = { { "User-Agent", "DiscordBot (https://github.com/seanomik/DisCPP, v0.0.0)" },
                            { "X-RateLimit-Precision", "millisecond"} };
    // Add the correct authorization header depending on the token type.
	if (globals::client_instance->config->type == TokenType::USER) {
	    headers.insert({ "Authorization", discpp::globals::client_instance->token });
	} else {
        headers.insert({ "Authorization", "Bot " + discpp::globals::client_instance->token });
	}

	for (auto head : add) {
		headers.insert(headers.end(), head);
	}

	return headers;
}

bool discpp::StartsWith(std::string string, std::string prefix) {
	/**
	 * @brief Check if a string starts with another string.
	 *
	 * ```cpp
	 *      bool does_start_with = discpp::StartsWith("$test_command", "$");
	 * ```
	 *
	 * @param[in] string The string to check if it starts with.
	 * @param[in] prefix The prefix to check if the string starts with it.
	 *
	 * @return bool
	 */

	return string.substr(0, prefix.size()) == prefix;
}

std::vector<std::string> discpp::SplitString(std::string str, std::string delimiter) {
	/**
	 * @brief Split a string into a vector.
	 *
	 * ```cpp
	 *      std::vector<std::string> argument_vec = discpp::SplitString(message.content, " ");
	 * ```
	 *
	 * @param[in] str The string to split
	 * @param[in] delimiter The delimiter to split the string at.
	 *
	 * @return std::vector<std::string>
	 */

    size_t pos = 0;
    std::vector<std::string> tokens;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);

        // If the string is not empty then add it to the vector.
        if (!token.empty()) {
            tokens.push_back(token);
        }

        str.erase(0, pos + delimiter.length());
    }

    // Push back the last token from the string.
    size_t last_token = str.find_last_of(delimiter);
    tokens.push_back(str.substr(last_token + 1));

    // If the vector is empty, then just return a vector filled with the given string.
    if (tokens.empty()) {
        return { str };
    }

	return tokens;
}

std::string discpp::CombineStringVector(std::vector<std::string> vector, std::string delimiter, int offset) {
	/**
	 * @brief Combine a vector into a string with spaces between each element.
	 *
	 * ```cpp
	 *      std::string combined = discpp::CombineVectorWithSpaces(arguments, 1);
	 * ```
	 *
	 * @param[in] vector The vector to combine.
	 * @param[in] delmiter The delimiter to combine the vector by.
	 * @param[in] offset The vector element offset to start at (if you wanted to skip the first 2 elements, set this offset to "2").
	 *
	 * @return std::string
	 */

	if (vector.size() == 0) return "";

	return std::accumulate(vector.begin() + offset, vector.end(), std::string(""), [delimiter](std::string s0, std::string const& s1) { return s0 += delimiter + s1; }).substr(1);
}

std::string discpp::ReadEntireFile(std::ifstream& file) {
	/**
	 * @brief Reads an entire file.
	 *
	 * ```cpp
	 *      std::string file_contents = discpp::ReadEntireFile(file);
	 * ```
	 *
	 * @param[in] file The file to read.
	 *
	 * @return std::string
	 */

	return std::string((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
}

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

std::string discpp::Base64Encode(std::string text) {
	/**
	 * @brief Encode Base64.
	 *
	 * ```cpp
	 *		std::string encodedStuff = Base64Encode("text");
	 * ```
	 *
	 * @param[in] text The text to encode.
	 *
	 * @return std::string
	 */

    unsigned char* buf = (unsigned char *) text.c_str();
    unsigned int buf_len = text.size();
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (buf_len--) {
        char_array_3[i++] = *(buf++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) {
            ret += base64_chars[char_array_4[j]];
        }

        while ((i++ < 3)) {
            ret += '=';
        }
    }

    return ret;
}

std::string discpp::ReplaceAll(std::string data, std::string to_search, std::string replace_str) {
	/**
	 * @brief Replace all occurences of sub strings
	 *
	 * ```cpp
	 *      std::string raw_text = ReplaceAll("discpp text", " ", "_");
	 * ```
	 *
	 * @param[in] string The string to escape.
	 *
	 * @return std::string
	 */

    // Get the first occurrence
    size_t pos = data.find(to_search);

    // Repeat till end is reached
    while(pos != std::string::npos) {
        // Replace this occurrence of Sub String
        data.replace(pos, to_search.size(), replace_str);
        // Get the next occurrence from the current position
        pos = data.find(to_search, pos + replace_str.size());
    }

	return data;
}

std::string discpp::EscapeString(std::string string) {
	/**
	 * @brief Escape strings for discpp json endpoints and cpr body
	 *
	 * ```cpp
	 *      std::string raw_text = "{\"content\":\"" + EscapeString(text) + (tts ? "\",\"tts\":\"true\"" : "\"") + "}";
	 *		cpr::Body body = cpr::Body(raw_text);
	 *		rapidjson::Document result = SendPostRequest(Endpoint("/channels/%/messages", id), DefaultHeaders({ { "Content-Type", "application/json" } }), id, RateLimitBucketType::CHANNEL, body);
	 * ```
	 *
	 * @param[in] string The string to escape.
	 *
	 * @return std::string
	 */

	string = ReplaceAll(string, "\\", "\\\\");
	string = ReplaceAll(string, "\"", "\\\"");
	string = ReplaceAll(string, "\a", "\\a");
	string = ReplaceAll(string, "\b", "\\b");
	string = ReplaceAll(string, "\f", "\\f");
	string = ReplaceAll(string, "\r", "\\r");
	string = ReplaceAll(string, "\t", "\\t");
	// \u + four-hex-digits

	return string;
}

int discpp::WaitForRateLimits(snowflake object, RateLimitBucketType ratelimit_bucket) {
	/**
	 * @brief Wait for rate limits.
	 *
	 * ```cpp
	 *      discpp::WaitForRateLimits(message.id, discpp::RateLimitBucketType::CHANNEL);
	 * ```
	 *
	 * @param[in] object The object id.
	 * @param[in] ratelimit_bucket The rate limit bucket.
	 *
	 * @return int
	 */

	RateLimit* rlmt = nullptr;

	if (global_ratelimit.remaining_limit == 0) {
		rlmt = &global_ratelimit;
	}
	else {
		switch (ratelimit_bucket) {
		case RateLimitBucketType::CHANNEL:
			rlmt = &channel_ratelimit[object];
			break;
		case RateLimitBucketType::GUILD:
			rlmt = &guild_ratelimit[object];
			break;
		case RateLimitBucketType::WEBHOOK:
			rlmt = &webhook_ratelimit[object];
			break;
		case RateLimitBucketType::GLOBAL:
			rlmt = &global_ratelimit;
			break;
		default:
			globals::client_instance->logger->Error(LogTextColor::RED + "RateLimitBucketType is invalid!");
			throw std::runtime_error("RateLimitBucketType is invalid!");
			break;
		}
	}

	if (rlmt->remaining_limit == 0) {
		double milisecond_time = rlmt->ratelimit_reset * 1000 - time(NULL) * 1000;

		if (milisecond_time > 0) {
			globals::client_instance->logger->Debug("Rate limit wait time: " + std::to_string(milisecond_time) + " miliseconds");
			std::this_thread::sleep_for(std::chrono::milliseconds((int)milisecond_time));
		}
	}
	return 0;
}

bool HeaderContains(cpr::Header header, std::string key) {
	/**
	 * @brief Check if a cpr::Header contains a specific key.
	 *
	 * ```cpp
	 *      bool contains = discpp::HeaderContains(headers, "auth");
	 * ```
	 *
	 * @param[in] header The headers to see if the key is contained in.
	 * @param[in] key The key to check if the headers contain.
	 *
	 * @return bool
	 */

	for (auto head : header) {
		if (head.first == key) return true;
	}
	return false;
}

void discpp::HandleRateLimits(cpr::Header header, snowflake object, RateLimitBucketType ratelimit_bucket) {
	/**
	 * @brief Handle rate limites
	 *
	 * ```cpp
	 *      discpp::HandleRateLimits(header, id, bucket);
	 * ```
	 *
	 * @param[in] header The headers to get the wait rate limit from.
	 * @param[in] object The object id.
	 * @param[in] ratelimit_bucket The rate limit bucket.
	 *
	 * @return int
	 */

	RateLimit* obj = nullptr;
	if (HeaderContains(header, "x-ratelimit-global")) {
		obj = &global_ratelimit;
	} else if (HeaderContains(header, "x-ratelimit-limit")) {
		switch (ratelimit_bucket) {
		    case RateLimitBucketType::CHANNEL:
		        obj = &channel_ratelimit[object];
		        break;
		    case RateLimitBucketType::GUILD:
		        obj = &guild_ratelimit[object];
		        break;
		    case RateLimitBucketType::WEBHOOK:
		        obj = &webhook_ratelimit[object];
		        break;
		    case RateLimitBucketType::GLOBAL:
		        obj = &global_ratelimit;
		        break;
		    default:
		        throw std::runtime_error("Invalid RateLimitBucketType!");
		        break;
		}
	} else {
		return;
	}

	obj->limit = std::stoi(header["x-ratelimit-limit"]);
	obj->remaining_limit = std::stoi(header["x-ratelimit-remaining"]);
	obj->ratelimit_reset = std::stod(header["x-ratelimit-reset"]);
}

time_t discpp::TimeFromSnowflake(snowflake snow) {
    constexpr static uint64_t discord_epoch = 1420070400000;
    return ((snow >> 22) + discord_epoch) / 1000;
}

std::string discpp::FormatTimeFromSnowflake(snowflake snow) {
	time_t unix_time = TimeFromSnowflake(snow);

	tm* n = std::gmtime(&unix_time);
	tm now = *n;
	char buffer[256];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d @ %H:%M:%S GMT", &now);

	return buffer;
}

bool discpp::ContainsNotNull(rapidjson::Document &json, char *value_name) {
    rapidjson::Value::ConstMemberIterator itr = json.FindMember(value_name);
    if (itr != json.MemberEnd()) {
        return !json[value_name].IsNull();
    }

    return false;
}

void discpp::IterateThroughNotNullJson(rapidjson::Document &json, std::function<void(rapidjson::Document&)> func) {
    for (auto const& object : json.GetArray()) {
        if (!object.IsNull()) {
            rapidjson::Document object_json;
            object_json.CopyFrom(object, object_json.GetAllocator());

            func(object_json);
        }
    }
}

rapidjson::Document discpp::GetDocumentInsideJson(rapidjson::Document &json, const char* value_name) {
    rapidjson::Document inside_json;
    inside_json.CopyFrom(json[value_name], inside_json.GetAllocator());

    // This shows an error in inteliisense for some reason but compiles fine.
#ifndef __INTELLISENSE__
    return std::move(inside_json);
#endif
}

std::string discpp::DumpJson(rapidjson::Document &json) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
	std::string tmp = buffer.GetString();

    return tmp;
}

std::string discpp::DumpJson(rapidjson::Value &json) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
    std::string tmp = buffer.GetString();

    return tmp;
}

// Safe characters for URIEncode
char SAFE[256] = {
        /*      0 1 2 3  4 5 6 7  8 9 A B  C D E F */
        /* 0 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 1 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 2 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 3 */ 1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,

        /* 4 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 5 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
        /* 6 */ 0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        /* 7 */ 1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,

        /* 8 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* 9 */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* A */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* B */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,

        /* C */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* D */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* E */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        /* F */ 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

std::string discpp::URIEncode(std::string str) {
    const char DEC2HEX[16 + 1] = "0123456789ABCDEF";
    const unsigned char * pSrc = (const unsigned char *) str.c_str();
    const int SRC_LEN = str.length();
    unsigned char * const pStart = new unsigned char[SRC_LEN * 3];
    unsigned char * pEnd = pStart;
    const unsigned char * const SRC_END = pSrc + SRC_LEN;

    for (; pSrc < SRC_END; ++pSrc) {
        if (SAFE[*pSrc]) {
            *pEnd++ = *pSrc;
        } else {
            // escape this char
            *pEnd++ = '%';
            *pEnd++ = DEC2HEX[*pSrc >> 4];
            *pEnd++ = DEC2HEX[*pSrc & 0x0F];
        }
    }

    std::string sResult((char *)pStart, (char *)pEnd);
    delete [] pStart;
    return sResult;
}

discpp::snowflake discpp::SnowflakeFromString(std::string str) {
    return strtoll(str.c_str(), NULL, 10);
}
