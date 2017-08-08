/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2014
 * \author Emneg Zeerd, emneg.zeerd@gmail.com 2017
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file CDltWrapper.h
 * For further information see http://www.genivi.org/.
 */

#ifndef _DLTWRAPPER_H_
#define _DLTWRAPPER_H_

#include <string>
#include <pthread.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <memory>

#include "dlt.h"

/**
 * Wraps around the dlt. This class is instantiated as a singleton and offers a default
 * context (maincontext) that is registered to log to.
 * Logging under the default context can simply be done with the logInfo/logError templates with up to 10 values at a time.
 * For logging with a different context, you can use the log template. First register a context with registerContext.
 */
class CDltWrapper
{
public:

       /**
        * This structure is used for context data used in an application.
        */
       typedef struct
       {
               DltContext *handle; /**< pointer to DltContext */
               std::stringstream buffer; /**< buffer for building log message*/
               int32_t log_level; /**< log level */
               int32_t trace_status; /**< trace status */
               int32_t args_num; /**< number of arguments for extended header*/
               uint8_t mcnt; /**< message counter */
               char* context_description; /**< description of context */
       } NoDltContextData;

       /*
        * The eunum gives the logtype
        */
       enum logDestination
       {
               DAEMON=0, //!< logging with the DLT daemon
               COMMAND_LINE=1, //!< logging with commandline
               FILE_OUT =2 //!< logging into a file
       };

       /**
        * Instanciate the Dlt Wrapper
        * @param appid The AppID
        * @param description A description of the Application
        * @param debugEnabled if set to true, debug outputs will be generated, default = true
        * @param logDest the destination, the log should be written
        * @param Filename the filename with absolute path where the log shall be written. only needed if logDest==FILE_OUT
        * @param onlyError if set to true, only errors will be logged. just valid for commandline and file logs, default value = false
        */
       static CDltWrapper* instanctiateOnce(const char *appid, const char * description, const bool debugEnabled = true, const logDestination logDest = logDestination::DAEMON, const std::string Filename="",bool onlyError=false);

    /**
     * get the Wrapper Instance
     */
    static CDltWrapper* instance();

       /**
        * register a context
        */
    void registerContext(DltContext& handle, const char *contextid, const char * description);
    void registerContext(DltContext& handle, const char *contextid, const char * description, const DltLogLevelType level, const DltTraceStatusType status);
    void unregisterContext(DltContext& handle);
    bool getEnabled();
    ~CDltWrapper();


    bool init(DltLogLevelType loglevel, DltContext* context = NULL);
    bool checkLogLevel(DltLogLevelType logLevel)
    {
    #ifdef DLT_IS_LOG_LEVEL_ENABLED
        return (dlt_user_is_logLevel_enabled(&mDltContext, logLevel) == DLT_RETURN_TRUE);
    #else
        (void)logLevel;
        return true;
    #endif
    }
    void deinit();
    void send();
    void append(const int8_t value);
    void append(const uint8_t value);
    void append(const int16_t value);
    void append(const uint16_t value);
    void append(const int32_t value);
    void append(const uint32_t value);
    void append(const uint64_t value);
    void append(const int64_t value);
    void append(const std::string& value);
    void append(const bool value);
    void append(const std::vector<uint8_t> & data);

    template<class T> void appendNoDLT(T value)
       {
               mNoDltContextData.buffer << value <<" ";
       }

    // specialization for const char*
    template<typename T = const char*> void append(const char* value)
    {
               if (mlogDestination == logDestination::DAEMON)
               {
                       dlt_user_log_write_string(&mDltContextData, value);
               }
               else
               {
                       mNoDltContextData.buffer << std::string(value);
               }
       }

public:

    // Template to print unknown pointer types with their address
    template<typename T> void append(T* value)
    {
        std::ostringstream ss;
        ss << "0x" << std::hex << (uint64_t)value;
        append(ss.str().c_str());
    }

    // Template to print unknown types
    template<typename T> void append(T value)
    {
        std::ostringstream ss;
        ss << std::dec << value;
        append(ss.str().c_str());
    }

    // Template parameter pack to generate recursive code
    void append(void) {}
    template<typename T, typename... TArgs> void append(T value, TArgs... args)
    {
        this->append(value);
        this->append(args...);
    }

private:
       /**
        * private contructor
        */
    CDltWrapper(const char *appid, const char * description, const bool debugEnabled = true, const logDestination logDest = logDestination::DAEMON, const std::string Filename="",bool onlyError=false); //is private because of singleton pattern
    bool initNoDlt(DltLogLevelType loglevel, DltContext* context);
    std::string now();
    DltContext mDltContext; //!< the default context
    DltContextData mDltContextData; //!< contextdata
    NoDltContextData mNoDltContextData; //!<contextdata for std out logging
    std::map<DltContext*,std::string> mMapContext; //!< a Map for all registered context
    bool mDebugEnabled;        //!< debug Enabled or not
    logDestination mlogDestination; //!< The log destination
    std::ofstream mFilename; //!< Filename for logging
    bool mOnlyError; //!< Only if Log Level is above Error
    bool mLogOn; //!< Used to keep track if currently logging is on
    static CDltWrapper* mpDLTWrapper; //!< pointer to the wrapper instance
    static pthread_mutex_t mMutex;

};

/**
 * logs given values with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void log(DltContext* const context, DltLogLevelType loglevel, T value, TArgs... args)
{
    CDltWrapper* inst(CDltWrapper::instance());
       if (!inst->getEnabled())
    {
               return;
       }
    if (!inst->init(loglevel, context))
    {
               return;
       }
    inst->append(value);
    inst->append(args...);
    inst->send();
}

/**
 * logs given values with debuglevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logDebug(T value, TArgs... args)
{
    log(NULL, DLT_LOG_DEBUG, value, args...);
}

/**
 * logs given values with infolevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logInfo(T value, TArgs... args)
{
    log(NULL, DLT_LOG_INFO, value, args...);
}

/**
 * logs given values with errorlevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logError(T value, TArgs... args)
{
    log(NULL, DLT_LOG_ERROR,value,args...);
}

/**
 * logs given values with warninglevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logWarning(T value, TArgs... args)
{
    log(NULL, DLT_LOG_WARN,value,args...);
}

/**
 * logs given values with verbose with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logVerbose(T value, TArgs... args)
{
    log(NULL, DLT_LOG_VERBOSE,value,args...);
}



#endif /* _DLTWRAPPER_H_ */
