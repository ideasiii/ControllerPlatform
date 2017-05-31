/*
 * CHttpFBToken.h
 *
 *  Created on: May 25, 2017
 *      Author: joe
 */

#ifndef CONTROLLERHANDLER_CHTTPFBTOKEN_H_
#define CONTROLLERHANDLER_CHTTPFBTOKEN_H_

#include <string>

class CHttpFBToken
{
public:
	CHttpFBToken();

	virtual ~CHttpFBToken();

	void init();
private:
	static std::string serToken;
	std::string serHost;
	//std::string ser

};

#endif /* CONTROLLERHANDLER_CHTTPFBTOKEN_H_ */
