#pragma once

#ifndef PHONE_HEADER_FILE_H
#define PHONE_HEADER_FILE_H

#include "account.h"

class TinyPhone
{
	map<pjsua_acc_id, SIPAccount*> accounts;
	pj::Endpoint* endpoint;

public:
	TinyPhone(pj::Endpoint* ep) {
		endpoint = ep;
	}

	~TinyPhone() {
		std::cout << "Shutting Down TinyPhone" << std::endl;
		logout();
	}

	bool hasAccounts() {
		return accounts.size() > 0;
	}

	SIPAccount* getPrimaryAccount() {
		if (!hasAccounts())
			return NULL;
		else {
			return accounts.begin()->second;
		}
	}

	SIPAccount* getAccountById(int pos) {
		if (!hasAccounts())
			return NULL;
		else {
			return accounts.find(pos)->second;
		}
	}

	SIPAccount* getAccountByURI(string uri) {
		if (!hasAccounts())
			return NULL;
		else {
			string full_uri = "sip:" + uri;
			SIPAccount* account = NULL;
			auto it = accounts.begin();
			while (it != accounts.end())
			{
				if (it->second->getInfo().uri == full_uri)
				{
					account = it->second;
					break;
				}
				it++;
			}
			return account;
		}
	}

	void logout() {
		auto it = accounts.begin();
		while (it != accounts.end()) {
			it->second->shutdown();
			it++;
		}
	}

	void hangupAllCalls() {
		endpoint->hangupAllCalls();
	}

	bool addAccount(string username, string domain, string password) {

		string account_name = username + "@" + domain;

		AccountConfig acc_cfg;
		acc_cfg.idUri = ("sip:" + account_name);
		acc_cfg.regConfig.registrarUri = ("sip:" + domain);
		acc_cfg.sipConfig.authCreds.push_back(AuthCredInfo("digest", "*", username, 0, password));

		acc_cfg.regConfig.timeoutSec = 180;
		acc_cfg.regConfig.retryIntervalSec = 30;
		acc_cfg.regConfig.firstRetryIntervalSec = 15;
		acc_cfg.videoConfig.autoTransmitOutgoing = PJ_FALSE;
		acc_cfg.videoConfig.autoShowIncoming = PJ_FALSE;

		SIPAccount *acc(new SIPAccount);
		acc->create(acc_cfg);

		accounts.insert(pair<pjsua_acc_id, SIPAccount*>(acc->getId(), acc));
	}

	SIPCall* makeCall(string uri) {
		SIPAccount* account = getPrimaryAccount();
		return makeCall(uri, account);
	}

	SIPCall* makeCall(string uri, SIPAccount* account) {
		SIPCall *call = new SIPCall(*account);
		account->calls.push_back(call);
		CallOpParam prm(true);
		prm.opt.audioCount = 1;
		prm.opt.videoCount = 0;
		call->makeCall(uri, prm);
		return call;
	}

};

#endif
