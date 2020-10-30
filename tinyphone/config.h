#pragma once

#ifndef CONFIG_HEADER_FILE_H
#define CONFIG_HEADER_FILE_H

#include <iostream>
#include <string>
#include "json.h"
#include "consts.h"
#include "utils.h"
#include <pjsua-lib/pjsua.h>

#define SIP_MAX_CALLS	4

#ifdef _DEBUG
#define SIP_MAX_ACC	3
#else
#define SIP_MAX_ACC	1
#endif

#define DEFUALT_PJ_LOG_LEVEL 3

namespace tp {

	struct appConfig {
		pjsip_transport_type_e transport;
		int timeoutSec;
		int refreshIntervalSec;
		int retryIntervalSec;
		int firstRetryIntervalSec;
		bool dropCallsOnFail;

		std::string uaPrefix;
		size_t maxCalls;
		size_t maxAccounts;
		size_t pjThreadCount;
		size_t pjMediaThreadCount;
		std::vector<std::string> audioCodecs;

		int pjLogLevel;

		bool enableNoiseCancel;
		bool useDefaultAudioDevice; 
		std::vector<std::string> prefferedAudioDevices;

		std::string securityCode;

		bool autoUnHold;
		bool testAudioDevice;
		bool unregisterOnDeviceError;

		bool deviceErrorAlert;
		bool disableVAD;

		size_t clockRate;
		size_t ecTailLen;

		bool enableWSEvents;

		bool enableMetrics;
		std::string metricsProto;
		std::vector<std::string> metricsServerHosts;
		int metricsServerPort;

		bool autoDeviceRefresh;
		bool autoAnswer;
		bool persistAccounts;


		std::string ua(){
			std::string productVersion;
			GetProductVersion(productVersion);
			return uaPrefix + productVersion;
		};
	};

	static void to_json(nlohmann::json& j, const appConfig& p) {
		j = nlohmann::json{
			{"transport", p.transport },
			{"timeoutSec", p.timeoutSec },
			{"refreshIntervalSec", p.refreshIntervalSec },
			{"retryIntervalSec", p.retryIntervalSec },
			{"firstRetryIntervalSec", p.firstRetryIntervalSec },
			{"dropCallsOnFail", p.dropCallsOnFail },
			{"uaPrefix", p.uaPrefix },
			{"maxCalls", p.maxCalls },
			{"maxAccounts", p.maxAccounts },
			{"audioCodecs", p.audioCodecs },
			{"pjLogLevel", p.pjLogLevel },
			{"enableNoiseCancel", p.enableNoiseCancel },
			{"useDefaultAudioDevice", p.useDefaultAudioDevice },
			{"prefferedAudioDevices", p.prefferedAudioDevices },
			{"securityCode", p.securityCode },
			{"autoUnHold", p.autoUnHold },
			{"testAudioDevice", p.testAudioDevice },
			{"unregisterOnDeviceError", p.unregisterOnDeviceError },
			{"deviceErrorAlert", p.deviceErrorAlert },
			{"pjThreadCount", p.pjThreadCount },
			{"pjMediaThreadCount", p.pjMediaThreadCount },
			{"disableVAD", p.disableVAD },
			{"clockRate", p.clockRate },
			{"ecTailLen", p.ecTailLen },
			{"enableWSEvents", p.enableWSEvents },
			{"enableMetrics", p.enableMetrics },
			{"metricsProto", p.metricsProto },
			{"metricsServerHosts", p.metricsServerHosts },
			{"metricsServerPort", p.metricsServerPort },
			{"autoDeviceRefresh", p.autoDeviceRefresh },
			{"autoAnswer", p.autoAnswer },
			{"persistAccounts", p.persistAccounts },
		};
    }

   static void from_json(const nlohmann::json& j, appConfig& p) {
		j.at("transport").get_to(p.transport);
		j.at("timeoutSec").get_to(p.timeoutSec);
		j.at("refreshIntervalSec").get_to(p.refreshIntervalSec);
		j.at("retryIntervalSec").get_to(p.retryIntervalSec);
		j.at("firstRetryIntervalSec").get_to(p.firstRetryIntervalSec);
		j.at("dropCallsOnFail").get_to(p.dropCallsOnFail);
		j.at("uaPrefix").get_to(p.uaPrefix);
		j.at("maxCalls").get_to(p.maxCalls);
		j.at("maxAccounts").get_to(p.maxAccounts);
		j.at("audioCodecs").get_to(p.audioCodecs);
		j.at("pjLogLevel").get_to(p.pjLogLevel);
		j.at("enableNoiseCancel").get_to(p.enableNoiseCancel);
		j.at("useDefaultAudioDevice").get_to(p.useDefaultAudioDevice);
		j.at("prefferedAudioDevices").get_to(p.prefferedAudioDevices);
		j.at("securityCode").get_to(p.securityCode);	
		j.at("autoUnHold").get_to(p.autoUnHold);
		j.at("testAudioDevice").get_to(p.testAudioDevice);
		j.at("unregisterOnDeviceError").get_to(p.unregisterOnDeviceError);
		j.at("deviceErrorAlert").get_to(p.deviceErrorAlert);
		j.at("pjThreadCount").get_to(p.pjThreadCount);
		j.at("pjMediaThreadCount").get_to(p.pjMediaThreadCount);
		j.at("disableVAD").get_to(p.disableVAD);
		j.at("clockRate").get_to(p.clockRate);
		j.at("ecTailLen").get_to(p.ecTailLen);
		j.at("enableWSEvents").get_to(p.enableWSEvents);
		j.at("enableMetrics").get_to(p.enableMetrics);
		j.at("metricsServerHosts").get_to(p.metricsServerHosts);
		j.at("metricsProto").get_to(p.metricsProto);
		j.at("metricsServerPort").get_to(p.metricsServerPort);
		j.at("autoDeviceRefresh").get_to(p.autoDeviceRefresh);
		j.at("autoAnswer").get_to(p.autoAnswer);
		j.at("persistAccounts").get_to(p.persistAccounts);
    }
   
    extern appConfig ApplicationConfig;

    void InitConfig();

	struct AccountConfig {
		std::string username;
		std::string domain;
		std::string password;
		std::string proxy;
	};

	void from_json(const nlohmann::json& j, AccountConfig& p);

	void to_json(nlohmann::json& j, const AccountConfig& p);

	struct tpUserConfig {
		std::vector<AccountConfig> accounts;
	};

	static void from_json(const nlohmann::json& j, tpUserConfig& p) {
		j.at("accounts").get_to(p.accounts);
	}

	static void to_json(nlohmann::json& j, const tpUserConfig& p) {
		j = nlohmann::json{
			{"accounts", p.accounts },
		};
	}
}


#endif