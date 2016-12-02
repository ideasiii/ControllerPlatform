/*    Copyright 2014 MongoDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "mongo/platform/basic.h"

#include "mongo/client/cyrus_sasl_client_session.h"

#include <boost/thread/mutex.hpp>

#include "mongo/base/init.h"
#include "mongo/client/native_sasl_client_session.h"
#include "mongo/util/assert_util.h"
#include "mongo/util/mongoutils/str.h"

namespace mongo {
namespace {

SaslClientSession* createCyrusSaslClientSession(const std::string& mech) {
    if (mech == "SCRAM-SHA-1") {
        return new NativeSaslClientSession();
    }
    return new CyrusSaslClientSession();
}

typedef int (*SaslCallbackFn)();

/*
 * Mutex functions to be used by the SASL library, if the client doesn't initialize the library
 * for us.
 */

void* saslMutexAlloc(void) {
    return new boost::mutex;
}

int saslMutexLock(void* mutex) {
    static_cast<boost::mutex*>(mutex)->lock();
    return SASL_OK;
}

int saslMutexUnlock(void* mutex) {
    static_cast<boost::mutex*>(mutex)->unlock();
    return SASL_OK;
}

void saslMutexFree(void* mutex) {
    delete static_cast<boost::mutex*>(mutex);
}

/**
 * Configures the SASL library to use allocator and mutex functions we specify,
 * unless the client application has previously initialized the SASL library.
 */
MONGO_INITIALIZER(CyrusSaslAllocatorsAndMutexes)(InitializerContext*) {
    sasl_set_mutex(saslMutexAlloc, saslMutexLock, saslMutexUnlock, saslMutexFree);
    return Status::OK();
}

int saslClientLogSwallow(void* context, int priority, const char* message) {
    return SASL_OK;  // do nothing
}

/**
 * Initializes the client half of the SASL library, but is effectively a no-op if the client
 * application has already done it.
 *
 * If a client wishes to override this initialization but keep the allocator and mutex
 * initialization, it should implement a MONGO_INITIALIZER_GENERAL with
 * CyrusSaslAllocatorsAndMutexes as a prerequisite and CyrusSaslClientContext as a
 * dependent.  If it wishes to override both, it should implement a MONGO_INITIALIZER_GENERAL
 * with CyrusSaslAllocatorsAndMutexes and CyrusSaslClientContext as dependents, or
 * initialize the library before calling mongo::runGlobalInitializersOrDie().
 */
MONGO_INITIALIZER_WITH_PREREQUISITES(CyrusSaslClientContext,
                                     ("NativeSaslClientContext", "CyrusSaslAllocatorsAndMutexes"))
(InitializerContext* context) {
    static sasl_callback_t saslClientGlobalCallbacks[] = {
        {SASL_CB_LOG, SaslCallbackFn(saslClientLogSwallow), NULL /* context */},
        {SASL_CB_LIST_END}};

    // If the client application has previously called sasl_client_init(), the callbacks passed
    // in here are ignored.
    //
    // TODO: Call sasl_client_done() at shutdown when we have a story for orderly shutdown.
    int result = sasl_client_init(saslClientGlobalCallbacks);
    if (result != SASL_OK) {
        return Status(ErrorCodes::UnknownError,
                      mongoutils::str::stream() << "Could not initialize sasl client components ("
                                                << sasl_errstring(result, NULL, NULL) << ")");
    }

    SaslClientSession::create = createCyrusSaslClientSession;
    return Status::OK();
}

/**
 * Callback registered on the sasl_conn_t underlying a CyrusSaslClientSession to allow the Cyrus
 * SASL library to query for the authentication id and other simple string configuration parameters.
 *
 * Note that in Mongo, the authentication and authorization ids (authid and authzid) are always
 * the same.  These correspond to SASL_CB_AUTHNAME and SASL_CB_USER.
 */
int saslClientGetSimple(void* context, int id, const char** result, unsigned* resultLen) throw() {
    CyrusSaslClientSession* session = static_cast<CyrusSaslClientSession*>(context);
    if (!session || !result)
        return SASL_BADPARAM;

    CyrusSaslClientSession::Parameter requiredParameterId;
    switch (id) {
        case SASL_CB_AUTHNAME:
        case SASL_CB_USER:
            requiredParameterId = CyrusSaslClientSession::parameterUser;
            break;
        default:
            return SASL_FAIL;
    }

    if (!session->hasParameter(requiredParameterId))
        return SASL_FAIL;
    StringData value = session->getParameter(requiredParameterId);
    *result = value.rawData();
    if (resultLen)
        *resultLen = static_cast<unsigned>(value.size());
    return SASL_OK;
}

/**
 * Callback registered on the sasl_conn_t underlying a CyrusSaslClientSession to allow
 * the Cyrus SASL library to query for the password data.
 */
int saslClientGetPassword(sasl_conn_t* conn,
                          void* context,
                          int id,
                          sasl_secret_t** outSecret) throw() {
    CyrusSaslClientSession* session = static_cast<CyrusSaslClientSession*>(context);
    if (!session || !outSecret)
        return SASL_BADPARAM;

    sasl_secret_t* secret = session->getPasswordAsSecret();
    if (secret == NULL) {
        sasl_seterror(conn, 0, "No password data provided");
        return SASL_FAIL;
    }

    *outSecret = secret;
    return SASL_OK;
}
}  // namespace

CyrusSaslClientSession::CyrusSaslClientSession()
    : SaslClientSession(), _saslConnection(NULL), _step(0), _done(false) {
    const sasl_callback_t callbackTemplate[maxCallbacks] = {
        {SASL_CB_AUTHNAME, SaslCallbackFn(saslClientGetSimple), this},
        {SASL_CB_USER, SaslCallbackFn(saslClientGetSimple), this},
        {SASL_CB_PASS, SaslCallbackFn(saslClientGetPassword), this},
        {SASL_CB_LIST_END}};
    std::copy(callbackTemplate, callbackTemplate + maxCallbacks, _callbacks);
}

CyrusSaslClientSession::~CyrusSaslClientSession() {
    sasl_dispose(&_saslConnection);
}

void CyrusSaslClientSession::setParameter(Parameter id, const StringData& value) {
    fassert(18665, id >= 0 && id < numParameters);
    if (id == parameterPassword) {
        // The parameterPassword is stored as a sasl_secret_t,  while other
        // parameters are stored directly.  This facilitates memory ownership management for
        // getPasswordAsSecret().
        _secret.reset(new char[sizeof(sasl_secret_t) + value.size() + 1]);
        sasl_secret_t* secret = static_cast<sasl_secret_t*>(static_cast<void*>(_secret.get()));
        secret->len = value.size();
        value.copyTo(static_cast<char*>(static_cast<void*>(&secret->data[0])), false);
    }
    SaslClientSession::setParameter(id, value);
}

sasl_secret_t* CyrusSaslClientSession::getPasswordAsSecret() {
    // See comment in setParameter() about the special storage of parameterPassword.
    return static_cast<sasl_secret_t*>(static_cast<void*>(_secret.get()));
}

Status CyrusSaslClientSession::initialize() {
    if (_saslConnection != NULL)
        return Status(ErrorCodes::AlreadyInitialized,
                      "Cannot reinitialize CyrusSaslClientSession.");

    int result = sasl_client_new(getParameter(parameterServiceName).toString().c_str(),
                                 getParameter(parameterServiceHostname).toString().c_str(),
                                 NULL,
                                 NULL,
                                 _callbacks,
                                 0,
                                 &_saslConnection);

    if (SASL_OK != result) {
        return Status(ErrorCodes::UnknownError,
                      mongoutils::str::stream() << sasl_errstring(result, NULL, NULL));
    }

    return Status::OK();
}

Status CyrusSaslClientSession::step(const StringData& inputData, std::string* outputData) {
    const char* output = NULL;
    unsigned outputSize = 0xFFFFFFFF;

    int result;
    if (_step == 0) {
        const char* actualMechanism;
        result = sasl_client_start(_saslConnection,
                                   getParameter(parameterMechanism).toString().c_str(),
                                   NULL,
                                   &output,
                                   &outputSize,
                                   &actualMechanism);
    } else {
        result = sasl_client_step(_saslConnection,
                                  inputData.rawData(),
                                  static_cast<unsigned>(inputData.size()),
                                  NULL,
                                  &output,
                                  &outputSize);
    }
    ++_step;
    switch (result) {
        case SASL_OK:
            _done = true;
        // Fall through
        case SASL_CONTINUE:
            *outputData = std::string(output, outputSize);
            return Status::OK();
        case SASL_NOMECH:
            return Status(ErrorCodes::BadValue, sasl_errdetail(_saslConnection));
        case SASL_BADAUTH:
            return Status(ErrorCodes::AuthenticationFailed, sasl_errdetail(_saslConnection));
        default:
            return Status(ErrorCodes::ProtocolError, sasl_errdetail(_saslConnection));
    }
}
}  // namespace

MONGO_INITIALIZER_FUNCTION_ASSURE_FILE(client_cyrus_sasl_client_session)
