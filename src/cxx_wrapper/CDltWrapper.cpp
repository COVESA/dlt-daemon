/**
 *  SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project DLT - Diagnostic Log and Trace.
 * Porting from GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2014
 * \author Emneg Zeerd, emneg.zeerd@gmail.com 2017
 *
 * \file CDltWrapper.cpp
 * For further information see http://www.genivi.org/.
 *
 */


#include "CDltWrapper.h"
#include <string>
#include <iostream>
#include <string.h>
#include <chrono>
#include <ctime>



CDltWrapper* CDltWrapper::mpDLTWrapper = NULL;
pthread_mutex_t CDltWrapper::mMutex = PTHREAD_MUTEX_INITIALIZER;

std::string CDltWrapper::now()
{
       std::time_t t(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
       struct tm * timeinfo(localtime(&t));
       char buffer[80];
       std::strftime(buffer,80,"%D %T ",timeinfo);
       return (std::string(buffer));
}

CDltWrapper* CDltWrapper::instanctiateOnce(const char *appid, const char * description, const bool debugEnabled, const logDestination logDest, const std::string Filename,bool onlyError)
{
       if (!mpDLTWrapper)
       {
               mpDLTWrapper = new CDltWrapper(appid,description,debugEnabled,logDest,Filename,onlyError);
       }
       return (mpDLTWrapper);
}

CDltWrapper* CDltWrapper::instance()
{
       if (!mpDLTWrapper)
       {
               // an application seems not to use our CDltWrapper class therefore create default
               std::ostringstream description;
               description << "PID=" << getpid() << " _=" << getenv("_");
               mpDLTWrapper = new CDltWrapper("AMDL", description.str().c_str());
               std::cerr << "Application doesn't call CDltWrapper::instanciateOnce!!!" << std::endl;
               std::cerr << "-> CDltWrapper::instance registers DLT application [ AMDL | " << description.str() << " ]" << std::endl;
       }
       return mpDLTWrapper;
}

bool CDltWrapper::getEnabled()
{
       return (mDebugEnabled);
}

bool CDltWrapper::initNoDlt(DltLogLevelType loglevel, DltContext* context)
{
       if (mlogDestination==logDestination::COMMAND_LINE)
       {
               if (!context)
               {
                       switch (loglevel)
                       {
                               case DLT_LOG_OFF :
                               case DLT_LOG_FATAL :
                               case DLT_LOG_ERROR :
                                       mNoDltContextData.buffer << "\033[0;31m"<<"[DEF] [Erro] \033[0m";
                                       mLogOn=true;
                                       break;
                               case DLT_LOG_WARN :
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer << "\033[0;33m"<<"[DEF] [Warn] \033[0m";
                                       }
                                       else
                                               mLogOn=false;
                                       break;
                               case DLT_LOG_INFO :
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer << "\033[0;36m"<<"[DEF] [Info] \033[0m";
                                       }
                                       else
                                               mLogOn=false;
                                       break;
                               default:
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer << "\033[0;32m"<<"[DEF] [Defa] \033[0m";
                                       }
                                       else
                                               mLogOn=false;
                       }
               }
               else
               {
                       std::string con(mMapContext.at(context));
                       switch (loglevel)
                       {
                               case DLT_LOG_OFF :
                               case DLT_LOG_FATAL :
                               case DLT_LOG_ERROR :
                                       mNoDltContextData.buffer << "\033[0;31m["<<con<<"] [Erro] \033[0m";
                                       mLogOn=true;
                                       break;
                               case DLT_LOG_WARN :
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer << "\033[0;33m["<<con<<"] [Warn] \033[0m";
                                       }
                                       else
                                               mLogOn=false;
                                       break;
                               case DLT_LOG_INFO :
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer << "\033[0;36m["<<con<<"]  [Info] \033[0m";
                                       }
                                       else
                                               mLogOn=false;

                                       break;
                               default:
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer << "\033[0;32m["<<con<<"]  [Defa] \033[0m";
                                       }
                                       else
                                               mLogOn=false;
                       }
               }
               return true;
       }
       else
       {
               if (!context)
               {
                       switch (loglevel)
                       {
                               case DLT_LOG_OFF :
                               case DLT_LOG_FATAL :
                               case DLT_LOG_ERROR :
                                       mNoDltContextData.buffer <<"[DEF] [Erro] ";
                                       mLogOn=true;
                                        break;
                               case DLT_LOG_WARN :
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer <<"[DEF] [Warn] ";
                                       }
                                       else
                                               mLogOn=false;
                                       break;
                               case DLT_LOG_INFO :
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer <<"[DEF] [Info] ";
                                       }
                                       else
                                               mLogOn=false;
                                       break;
                               default:
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer <<"[DEF] [Defa] ";
                                       }
                                       else
                                               mLogOn=false;

                       }
               }
               else
               {
                       std::string con(mMapContext.at(context));
                       switch (loglevel)
                       {
                               case DLT_LOG_OFF :
                               case DLT_LOG_FATAL :
                               case DLT_LOG_ERROR :
                                       mNoDltContextData.buffer << "["<<con<<"] [Erro] ";
                                       mLogOn=true;
                                        break;
                               case DLT_LOG_WARN :
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer << "["<<con<<"] [Warn] ";
                                       }
                                       else
                                               mLogOn=false;
                                       break;
                               case DLT_LOG_INFO :
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer << "["<<con<<"] [Info] ";
                                       }
                                       else
                                               mLogOn=false;

                                       break;
                               default:
                                       if (!mOnlyError)
                                       {
                                               mNoDltContextData.buffer << "["<<con<<"] [Defa] ";
                                       }
                                       else
                                               mLogOn=false;
                       }
               }
               return true;
       }
}

CDltWrapper::CDltWrapper(const char *appid, const char * description, const bool debugEnabled, const logDestination logDest, const std::string Filename,bool onlyError) :
       mDebugEnabled(debugEnabled), //
       mlogDestination(logDest), //
       mFilename(NULL), //
       mOnlyError(onlyError), //
       mLogOn(true)
{
       if (mDebugEnabled && mlogDestination==logDestination::DAEMON)
       {
               dlt_register_app(appid, description);
               //register a default context
               dlt_register_context(&mDltContext, "DEF", "Default Context registered by DLTWrapper Class");
       }
       else if (mDebugEnabled)
       {
               if (mlogDestination==logDestination::COMMAND_LINE)
                       std::cout << "\033[0;36m[DLT] Registering AppID " << appid << " , " << description << "\033[0m"<< std::endl;
               else
               {
                       mFilename.open(Filename, std::ofstream::out | std::ofstream::trunc);
                       if (!mFilename.is_open())
                       {
                               throw std::runtime_error("Cannot open file for logging");
                       }
                       mFilename << now() << "[DLT] Registering AppID " << appid << " , " << description << std::endl;
               }
       }
}

CDltWrapper::~CDltWrapper()
{
       if (mpDLTWrapper && mDebugEnabled && mlogDestination==logDestination::DAEMON)
       {
               mpDLTWrapper->unregisterContext(mDltContext);
               delete mpDLTWrapper;
       }
       else if (mpDLTWrapper && mDebugEnabled && mlogDestination==logDestination::COMMAND_LINE)
       {
               mFilename.close();
       }
}

void CDltWrapper::unregisterContext(DltContext & handle)
{
       if (mDebugEnabled && mlogDestination==logDestination::DAEMON)
       {
               dlt_unregister_context(&handle);
       }
}

void CDltWrapper::deinit()
{
       if (mDebugEnabled)
       {
               unregisterContext(mDltContext);
       }
}

void CDltWrapper::registerContext(DltContext& handle, const char *contextid, const char *description)
{
       if (mDebugEnabled && mlogDestination==logDestination::DAEMON)
       {
               dlt_register_context(&handle, contextid, description);
       }
       else if (mDebugEnabled)
       {
               mMapContext.emplace(&handle,std::string(contextid));

               if (mlogDestination==logDestination::COMMAND_LINE)
                       std::cout << "\033[0;36m[DLT] Registering Context " << contextid << " , " << description << "\033[0m"<< std::endl;
               else
                       mFilename << now() << "[DLT] Registering Context " << contextid << " , " << description << std::endl;
       }
}

void CDltWrapper::registerContext(DltContext& handle, const char *contextid, const char * description,const DltLogLevelType level, const DltTraceStatusType status)
{
       if (mDebugEnabled && mlogDestination==logDestination::DAEMON)
       {
               dlt_register_context_ll_ts(&handle, contextid, description, level, status);
       }
       else if (mDebugEnabled)
       {
               mMapContext.emplace(&handle,std::string(contextid));

               if (mlogDestination==logDestination::COMMAND_LINE)
                       std::cout << "\033[0;36m[DLT] Registering Context " << contextid << " , " << description << "\033[0m"<< std::endl;
               else
                       mFilename << now() << " [DLT] Registering Context " << contextid << " , " << description << std::endl;
       }
}

bool CDltWrapper::init(DltLogLevelType loglevel, DltContext* context)
{
       pthread_mutex_lock(&mMutex);
       if (mlogDestination==logDestination::DAEMON)
       {
               if (!context)
                       context = &mDltContext;

               if(dlt_user_log_write_start(context, &mDltContextData, loglevel) <= 0)
               {
                       pthread_mutex_unlock(&mMutex);
                       return false;
               }
       }
       else
       {
               initNoDlt(loglevel,context);
       }
       return true;
}

void CDltWrapper::send()
{
       if (mlogDestination==logDestination::DAEMON)
       {
               dlt_user_log_write_finish(&mDltContextData);
       }
       else
       {
               if (mlogDestination==logDestination::COMMAND_LINE && mLogOn)
                       std::cout << mNoDltContextData.buffer.str().c_str() << std::endl;
               else if (mLogOn)
                       mFilename << now() << mNoDltContextData.buffer.str().c_str() << std::endl;

               mNoDltContextData.buffer.str("");
               mNoDltContextData.buffer.clear();
       }
       pthread_mutex_unlock(&mMutex);
}

void CDltWrapper::append(const int8_t value)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_int8(&mDltContextData, value);
       else
               appendNoDLT(value);
}

void CDltWrapper::append(const uint8_t value)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_uint8(&mDltContextData, value);
       else
               appendNoDLT(value);
}

void CDltWrapper::append(const int16_t value)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_int16(&mDltContextData, value);
       else
               appendNoDLT(value);
}

void CDltWrapper::append(const uint16_t value)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_uint16(&mDltContextData, value);
       else
               appendNoDLT(value);
}

void CDltWrapper::append(const int32_t value)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_int32(&mDltContextData, value);
       else
               appendNoDLT(value);
}

void CDltWrapper::append(const uint32_t value)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_uint32(&mDltContextData, value);
       else
               appendNoDLT(value);
}

void CDltWrapper::append(const std::string& value)
{
       append(value.c_str());
}

void CDltWrapper::append(const bool value)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_bool(&mDltContextData, static_cast<uint8_t>(value));
       else
               appendNoDLT(value);
}

void CDltWrapper::append(const int64_t value)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_int64(&mDltContextData, value);
       else
               appendNoDLT(value);
}

void CDltWrapper::append(const uint64_t value)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_uint64(&mDltContextData, value);
       else
               appendNoDLT(value);
}

void CDltWrapper::append(const std::vector<uint8_t> & data)
{
       if (mlogDestination==logDestination::DAEMON)
               dlt_user_log_write_raw(&mDltContextData,(void*)data.data(),data.size());
       else
               mNoDltContextData.buffer << data.data();
}
