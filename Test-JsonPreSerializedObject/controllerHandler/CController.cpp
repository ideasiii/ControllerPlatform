#include "CController.h"

#include <list>
#include <limits.h>
#include <random>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "common.h"
#include "event.h"
#include "packet.h"
#include "utility.h"
#include "CConfig.h"
#include "CThreadHandler.h"
#include "JSONObject.h"

using namespace std;

#define LOG_TAG "[CController]"
#define LOG_TAG_COLORED "[\033[1;33mCController\033[0m]"
#define VERY_LONG_JSON_STR R"({"servlet":[{"servlet-name":"cofaxCDS","servlet-class":"org.cofax.cds.CDSServlet","init-param":{"configGlossary: installationAt":"Philadelphia, PA","configGlossary: adminEmail":"ksm@pobox.com","configGlossary: poweredBy":"Cofax","configGlossary: poweredByIcon":"/images/cofax.gif","configGlossary: staticPath":"/content/static","templateProcessorClass":"org.cofax.WysiwygTemplate","templateLoaderClass":"org.cofax.FilesTemplateLoader","templatePath":"templates","templateOverridePath":"","defaultListTemplate":"listTemplate.htm","defaultFileTemplate":"articleTemplate.htm","useJSP":false,"jspListTemplate":"listTemplate.jsp","jspFileTemplate":"articleTemplate.jsp","cachePackageTagsTrack":200,"cachePackageTagsStore":200,"cachePackageTagsRefresh":60,"cacheTemplatesTrack":100,"cacheTemplatesStore":50,"cacheTemplatesRefresh":15,"cachePagesTrack":200,"cachePagesStore":100,"cachePagesRefresh":10,"cachePagesDirtyRead":10,"searchEngineListTemplate":"forSearchEnginesList.htm","searchEngineFileTemplate":"forSearchEngines.htm","searchEngineRobotsDb":"WEB-INF/robots.db","useDataStore":true,"dataStoreClass":"org.cofax.SqlDataStore","redirectionClass":"org.cofax.SqlRedirection","dataStoreName":"cofax","dataStoreDriver":"com.microsoft.jdbc.sqlserver.SQLServerDriver","dataStoreUrl":"jdbc: microsoft: sqlserver: //LOCALHOST: 1433;DatabaseName=goon","dataStoreUser":"sa","dataStorePassword":"dataStoreTestQuery","dataStoreTestQuery":"SETNOCOUNTON;selecttest='test';","dataStoreLogFile":"/usr/local/tomcat/logs/datastore.log","dataStoreInitConns":10,"dataStoreMaxConns":100,"dataStoreConnUsageLimit":100,"dataStoreLogLevel":"debug","maxUrlLength":500}},{"servlet-name":"cofaxEmail","servlet-class":"org.cofax.cds.EmailServlet","init-param":{"mailHost":"mail1","mailHostOverride":"mail2"}},{"servlet-name":"cofaxAdmin","servlet-class":"org.cofax.cds.AdminServlet"},{"servlet-name":"fileServlet","servlet-class":"org.cofax.cds.FileServlet"},{"servlet-name":"cofaxTools","servlet-class":"org.cofax.cms.CofaxToolsServlet","init-param":{"templatePath":"toolstemplates/","log":1,"logLocation":"/usr/local/tomcat/logs/CofaxTools.log","logMaxSize":"","dataLog":1,"dataLogLocation":"/usr/local/tomcat/logs/dataLog.log","dataLogMaxSize":"","removePageCache":"/content/admin/remove?cache=pages&id=","removeTemplateCache":"/content/admin/remove?cache=templates&id=","fileTransferFolder":"/usr/local/tomcat/webapps/content/fileTransferFolder","lookInContext":1,"adminGroupID":4,"betaServer":true}}],"servlet-mapping":{"cofaxCDS":"/","cofaxEmail":"/cofaxutil/aemail/*","cofaxAdmin":"/admin/*","fileServlet":"/static/*","cofaxTools":"/tools/*"},"taglib":{"taglib-uri":"cofax.tld","taglib-location":"/WEB-INF/tlds/cofax.tld"}})"
#define NORMAL_LEN_JSON_STR R"({"userId":1,"id":1,"title":"optio rerit","body":"\"arcto\""})"

CController::CController() :
	mnMsqKey(-1)
{
	// allocate resources in onInitial() instead
}

CController::~CController()
{
	// release resources in onFinish() instead
}

int CController::onCreated(void* nMsqKey)
{
	// Use the msq key given
	mnMsqKey = *(reinterpret_cast<int*>(nMsqKey));

	_log(LOG_TAG" onCreated() mnMsqKey = %d", mnMsqKey);

	while (true)
	{
		JSONObject obj;
		JSONObject *tempObj = new JSONObject(NORMAL_LEN_JSON_STR);
		obj.create();
		obj.put("number1", 233);
		obj.put("number2", 243.66);
		obj.put("string", R"(asdf53454 "important" 4455)");
		obj.put("object", *tempObj);
		obj.putSerialized("serializedObject", *tempObj);

		tempObj->release();
		delete tempObj;

		printf("obj.toUnformattedString():\n%s\n\n", obj.toUnformattedString().c_str());
		printf("obj.toJSON():\n%s\n", obj.toJSON().c_str());

		obj.release();
		sleep(100);
	}

	exit(11);
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	return FALSE;
}

int CController::onFinish(void* nMsqKey)
{
	return FALSE;
}

void CController::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{
	switch (nCommand)
	{
	default:
		_log(LOG_TAG_COLORED" Unknown message command %s", numberToHex(nCommand).c_str());
		break;
	}
}

void CController::onHandleMessage(Message &message)
{
	switch (message.what)
	{
	default:
		_log(LOG_TAG_COLORED" onHandleMessage(): unknown message.what %d", message.what);
		break;
	}
}

std::string CController::taskName()
{
	return "CController";
}
