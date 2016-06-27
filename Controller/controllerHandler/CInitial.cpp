/*
 * CInitial.cpp
 *
 *  Created on: 2015年12月14日
 *      Author: Louis Ju
 */

#include "../controllerHandler/CInitial.h"

#include "packet.h"

using namespace std;

CInitial::CInitial()
{

}

CInitial::~CInitial()
{

}

string CInitial::getInitData(const int nType)
{
	string strData;
	switch (nType)
	{
		case TYPE_MOBILE_SERVICE:
			strData =
					"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"54.199.198.94\",\"port\": 6607	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"54.199.198.94\",\"port\": 2307	}]}";
			break;
		case TYPE_POWER_CHARGE_SERVICE:
			strData =
					"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"54.199.198.94\",\"port\": 6607	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"54.199.198.94\",\"port\": 2307	}]}";
			break;
		case TYPE_SDK_SERVICE:
			strData =
					"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"54.199.198.94\",\"port\": 6607	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"54.199.198.94\",\"port\": 2307	}]}";
			break;
		case TYPE_TRACKER_SERVICE:
			strData =
					"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"54.199.198.94\",\"port\": 6607	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"54.199.198.94\",\"port\": 2307	}]}";
			break;
		case TYPE_TRACKER_APPLIENCE:
			strData =
					"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"54.199.198.94\",\"port\": 6607	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"54.199.198.94\",\"port\": 2307	}]}";
			break;
		case TYPE_TRACKER_TOY:
			strData =
					"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"54.199.198.94\",\"port\": 6607	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"54.199.198.94\",\"port\": 2307	}]}";
			break;
		case TYPE_TRACKER_IOT:
			strData =
					"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"54.199.198.94\",\"port\": 6607	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"54.199.198.94\",\"port\": 2307	}]}";
			break;
	}
	return strData;
}

