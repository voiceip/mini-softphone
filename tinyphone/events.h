#pragma once

#ifndef EVENTS_HEADER_FILE_H
#define EVENTS_HEADER_FILE_H

#include "consts.h"
#include "channel.h"
#include <iostream>
#include <pjsua2.hpp>
#include "json.h"
#include "utils.h"
#include "config.h"

using namespace std;
using namespace pj;
using json = nlohmann::json;

class EventStream {
	channel<std::string>* chan;
public:

	EventStream(channel<std::string>* _chan) {
		chan = _chan;
	}

	~EventStream() {
		cout << "Shutting down Event Stream" << endl;
	}

	void publishEvent(AccountInfo ai, OnRegStateParam &prm) {
		UNUSED_ARG(prm);
		if (tp::ApplicationConfig.enableWSEvents) {
			json event = {
				{ "type","ACCOUNT" },
				{ "account", ai.uri},
				{ "presence", ai.onlineStatusText },
				{ "status", ai.regStatusText },
				{ "id", ai.id },
				{ "code",  prm.code },
			};
			if (ai.regIsActive)
				event["register"] = true;
			else
				event["unregister"] = true;
			chan->push(event.dump());
		}
	}

	void publishEvent(CallInfo ci, OnCallStateParam &prm) {
		UNUSED_ARG(prm);
		if (tp::ApplicationConfig.enableWSEvents) {
			tp::SIPUri uri;
			tp::ParseSIPURI(ci.remoteUri, &uri);
			json event = {
				{ "type","CALL" },
				{ "id", ci.id },
				{ "party", ci.remoteUri },
				{ "callerId", uri.user },
				{ "displayName", uri.name },
				{ "state", ci.stateText },
				{ "sid", ci.callIdString },
			};
			chan->push(event.dump());
		}
	}

	void publishEvent(CallInfo ci, OnIncomingCallParam &iprm) {
		UNUSED_ARG(iprm);
		if (tp::ApplicationConfig.enableWSEvents) {
			tp::SIPUri uri;
			tp::ParseSIPURI(ci.remoteUri, &uri);
			json event = {
				{"type","CALL"},
				{"incomming", true},
				{"party", ci.remoteUri },
				{ "from", uri.user },
				{ "displayName", uri.name },
				{"state", ci.stateText },
			};
			chan->push(event.dump());
		}
	}

};

#endif
