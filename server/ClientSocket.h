/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 ts=2 et ft=cpp : */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CLIENTSOCKET_H__
#define CLIENTSOCKET_H__

#include "Ack.h"
#include "config.h"
#include "FileWriter.h"
#include "prnetdb.h"
#include <vector>

class ClientSocket
{
public:
  ClientSocket(PRNetAddr *aAddr);
  ~ClientSocket();
  int MaybeSendSomethingOrCheckFinish(PRFileDesc *aFd,
                                      bool &aClientFinished);
  int SendAcks(PRFileDesc *aFd);
  bool IsThisSocket(const PRNetAddr *aAddr);
  int NewPkt(int32_t aCount, char *aBuf);
  int NoDataForTooLong();
  int WaitForFinishTimeout();
  int RunTestSend(PRFileDesc *aFd);
  int SendFinishPacket(PRFileDesc *aFd);

private:
  void FormatStartPkt(uint32_t aTS);
  void FormatDataPkt(uint32_t aTS);
  void FormatFinishPkt();
  uint32_t ReadACKPktAndLog(char *aBuf, uint32_t aTS);
  void LogLogFormat();
  int FirstPacket(int32_t aCount, char *aBuf, PRIntervalTime received);

private:
  PRNetAddr mNetAddr;
  int mTestType;
  char mSendBuf[PAYLOADSIZE];
  char mRecvBuf[PAYLOADSIZE];
  int mReplySize;
  PRIntervalTime mFirstPktSent;
  PRIntervalTime mFirstPktReceived;
  PRIntervalTime mNextTimeToDoSomething;
  uint64_t mSentBytes;
  uint64_t mRecvBytes;
  std::vector<Ack> mAcksToSend;
  int mNumberOfRetransFinish;
  uint64_t mPktPerSec;
  double mPktInterval;
  double mNextToSendInns;
  char mPktIdFirstPkt[4];
  uint64_t mPktPerSecObserved;
  uint32_t mNextPktId;
  uint32_t mLastPktId;
  PRIntervalTime mNodataTimeout;
  PRIntervalTime mLastReceivedTimeout;
  bool mError;

  FileWriter mLogFile;
  char mLogFileName[FILE_NAME_LEN];

  enum PHASE {
    START_TEST,
    RUN_TEST,
    FINISH_PACKET,
    WAIT_FINISH_TIMEOUT,
    TEST_FINISHED
  };

  enum PHASE mPhase;
  char mLogstr[80];
};

#endif
