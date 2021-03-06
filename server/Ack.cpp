/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 ts=2 et ft=cpp : */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Ack.h"
#include "HelpFunctions.h"
#include "config.h"
#include <cstring>

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

Ack::Ack(char *aBuf, PRIntervalTime aRecv, int aLargeAck, uint64_t aRate)
{
  if (aLargeAck) {
    if (aLargeAck < 512) {
      aLargeAck = 512;
    }
    mBufLen = aLargeAck;
  } else if (aRate) {
    mBufLen = PKT_ID_LEN + TIMESTAMP_LEN + TIMESTAMP_RECEIVED_LEN +
              TIMESTAMP_ACK_SENT_LEN + RATE_RECEIVING_PKT_LEN;
  } else {
    mBufLen = PKT_ID_LEN + TIMESTAMP_LEN + TIMESTAMP_RECEIVED_LEN +
              TIMESTAMP_ACK_SENT_LEN;
  }
  mBuf = new char[mBufLen];
  memcpy(mBuf, aBuf, mBufLen);
  if (aRate) {
    uint64_t rate = htonll(aRate);
    memcpy(mBuf + RATE_RECEIVING_PKT_START, &rate, RATE_RECEIVING_PKT_LEN);
  }
  uint32_t usec = htonl(PR_IntervalToMilliseconds(aRecv));
  memcpy(mBuf + TIMESTAMP_RECEIVED_START, &usec, TIMESTAMP_RECEIVED_LEN);
}

Ack::~Ack()
{
  delete [] mBuf;
}

Ack::Ack(const Ack &other)
{
  mBufLen = other.mBufLen;
  mBuf = new char[mBufLen];
  memcpy(mBuf, other.mBuf, mBufLen);
}

Ack&
Ack::operator= (const Ack &other)
{
  if (this != &other) {
    mBufLen = other.mBufLen;
    delete []mBuf;
    mBuf = new char[mBufLen];
    memcpy(mBuf, other.mBuf, mBufLen);
  }
  return *this;
}

int
Ack::SendPkt(PRFileDesc *aFd, PRNetAddr *aNetAddr)
{
  uint32_t usec = htonl(PR_IntervalToMilliseconds(PR_IntervalNow()));
  memcpy(mBuf + TIMESTAMP_ACK_SENT_START, &usec, TIMESTAMP_ACK_SENT_LEN);
  int write = PR_SendTo(aFd, mBuf, mBufLen, 0, aNetAddr,
                        PR_INTERVAL_NO_WAIT);
  if (write < 1) {
    PRErrorCode code = PR_GetError();
    if (code == PR_WOULD_BLOCK_ERROR) {
      return code;
    }
    return LogErrorWithCode(code, "UDP");
  }
  return 0;
}
