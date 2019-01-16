#include "stdafx.h"
#include "call.h"
#include "account.h"
#include "utils.h"

void SIPCall::onCallState(OnCallStateParam &prm)
{
	PJ_UNUSED_ARG(prm);
	CallInfo ci = getInfo();

	PJ_LOG(3, (__FILENAME__, "CallState Change: %s [%s]", ci.remoteUri.c_str(), ci.stateText.c_str()));

	account->eventStream->publishEvent(ci, prm);

	switch (ci.state) {
	case PJSIP_INV_STATE_DISCONNECTED:
		onCallEnd();
		account->removeCall(this);
		account->onCallEnd(this);
		/* Delete the call */
		delete this;
		break;
	case PJSIP_INV_STATE_CONFIRMED:
		break;
	default:
		break;
	}
}


void SIPCall::onCallMediaState(OnCallMediaStateParam &prm)
{
	CallInfo ci = getInfo();

	const char *status_name[] = {
		"None",
		"Active",
		"Local hold",
		"Remote hold",
		"Error"
	};

	// Iterate all the call medias
	for (unsigned i = 0; i < ci.media.size(); i++) {
		if (ci.media[i].type == PJMEDIA_TYPE_AUDIO && getMedia(i)) {
			if (ci.media[i].status == PJSUA_CALL_MEDIA_ACTIVE || ci.media[i].status == PJSUA_CALL_MEDIA_REMOTE_HOLD) {
				AudioMedia *aud_med = (AudioMedia *)getMedia(i);
				// Connect the call audio media to sound device
				AudDevManager& mgr = Endpoint::instance().audDevManager();
				PJ_LOG(3, (__FILENAME__, "Connecting Call to Media Device Input #%d , Output # %d", mgr.getCaptureDev(), mgr.getPlaybackDev()));
				aud_med->startTransmit(mgr.getPlaybackDevMedia());
				mgr.getCaptureDevMedia().startTransmit(*aud_med);
			}
			else {
				pj_assert(ci.media[i].status <= PJ_ARRAY_SIZE(status_name));
				PJ_LOG(3, (__FILENAME__, "Call [%d] OnCallMediaState Media State %s", ci.id , status_name[ci.media[i].status]));
			}
		}
	}
}

tp::HoldStatus SIPCall::HoldState() {
	auto call_info = getInfo();
	if (call_info.state == PJSIP_INV_STATE_CONFIRMED) {
		if (call_info.media.size() > 0) {
			auto current_media = call_info.media.front();
			if (current_media.status == PJSUA_CALL_MEDIA_LOCAL_HOLD || current_media.status == PJSUA_CALL_MEDIA_NONE) {
				return tp::HoldStatus::LOCAL_HOLD;
			}
			else if (current_media.status == PJSUA_CALL_MEDIA_REMOTE_HOLD) {
				return tp::HoldStatus::REMOTE_HOLD;
			}
		}
	}
	return tp::HoldStatus::NOT_IN_HOLD;
}

bool SIPCall::HoldCall() {

	auto call_info = getInfo();
	if (call_info.state == PJSIP_INV_STATE_CONFIRMED) {
		//must have a local media
		if (call_info.media.size() > 0) {
			auto current_media = call_info.media.front();
			if (current_media.status != PJSUA_CALL_MEDIA_LOCAL_HOLD && current_media.status != PJSUA_CALL_MEDIA_NONE) {
				//CallOpParam prm;
				//prm.options = PJSUA_CALL_UPDATE_CONTACT;
				//setHold(prm);
				PJ_LOG(3, (__FILENAME__, "Call %d Hold Triggered", call_info.id));
				pjsua_call_set_hold(call_info.id, NULL);
				return true;
			}
			else
				PJ_LOG(3, (__FILENAME__, "Hold Failed, already on hold maybe?"));
		}
		else
			PJ_LOG(3, (__FILENAME__, "Hold Failed, Call Doesn't have any media"));
	}
	else
		PJ_LOG(3, (__FILENAME__, "Hold Failed, Call Not in Confirmed State"));
	return false;
}

bool SIPCall::UnHoldCall() {

	auto call_info = getInfo();
	if (call_info.state == PJSIP_INV_STATE_CONFIRMED) {
		if (call_info.media.size() > 0) {
			auto current_media = call_info.media.front();
			if (current_media.status == PJSUA_CALL_MEDIA_LOCAL_HOLD || current_media.status == PJSUA_CALL_MEDIA_NONE) {
				CallOpParam prm(true);
				prm.opt.audioCount = 1;
				prm.opt.videoCount = 0;
				prm.opt.flag |= PJSUA_CALL_UNHOLD;
				reinvite(prm);
				return true;
			}
			else
				PJ_LOG(3, (__FILENAME__, "UnHold Failed, already active maybe?"));
		}
		else
			PJ_LOG(3, (__FILENAME__, "UnHold Failed, Call Doesn't have any media"));
	}
	else
		PJ_LOG(3, (__FILENAME__, "UnHold Failed, Call Not in Confirmed State"));
	return false;
}

void SIPCall::onCallEnd() {
	ToneGenerator toneGenerator;
	AudioMedia& play_med = Endpoint::instance().audDevManager().getPlaybackDevMedia();
	try {

		ToneDesc tone;
		tone.freq1 = 425;
		tone.freq2 = 0;
		tone.on_msec = 200;
		tone.off_msec = 100;

		ToneDescVector tones = { tone };

		toneGenerator.createToneGenerator();
		toneGenerator.play(tones, true);
		toneGenerator.startTransmit(play_med);

		// let the tone play for a sec.
		pj_thread_sleep(1000);

		toneGenerator.stop();
		toneGenerator.stopTransmit(play_med);
	}
	catch (Error& err) {
		UNUSED_ARG(err);
		PJ_LOG(3, (__FILENAME__, "SIPCall::onCallEnd Error"));
	}

}

void SIPCall::Hangup() {
	CallOpParam prm;
	hangup(prm);
}