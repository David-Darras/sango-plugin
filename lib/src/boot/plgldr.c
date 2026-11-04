/*
 * This file is from libctrpf (plgldr.c). It has only the functions that the
 * minimal boot needs. Altered source version.
 */

#include "plgldr.h"

#include <string.h>

static Handle plgLdrHandle;
static Handle plgLdrArbiter = 0;
static s32* plgEvent;
static s32* plgReply;
static int plgLdrRefCount;

static Result PLGLDR__GetArbiter(void) {
  Result res = 0;
  u32* cmdbuf = getThreadCommandBuffer();

  cmdbuf[0] = IPC_MakeHeader(9, 0, 0);
  if (R_SUCCEEDED((res = svcSendSyncRequest(plgLdrHandle)))) {
    res = cmdbuf[1];
    plgLdrArbiter = cmdbuf[3];
  }
  return res;
}

Result plgLdrInit(void) {
  Result res = 0;

  if (AtomicPostIncrement(&plgLdrRefCount) == 0)
    res = svcConnectToPort(&plgLdrHandle, "plg:ldr");

  if (R_SUCCEEDED(res)) {
    u32 res2 = PLGLDR__GetArbiter();
    // Succeeded, or not implemented (Citra)
    if (R_SUCCEEDED(res2) || res2 == 0xE0E01BF4) {
      plgEvent = PLUGIN_HEADER->plgldrEvent;
      plgReply = PLUGIN_HEADER->plgldrReply;
    } else {
      plgLdrExit();
    }
  } else {
    plgLdrExit();
  }
  return res;
}

void plgLdrExit(void) {
  if (AtomicDecrement(&plgLdrRefCount)) return;

  if (plgLdrHandle) svcCloseHandle(plgLdrHandle);
  if (plgLdrArbiter) svcCloseHandle(plgLdrArbiter);
  plgLdrHandle = plgLdrArbiter = 0;
}

Result PLGLDR__DisplayErrMessage(const char* title, const char* body,
                                 u32 error) {
  Result res = 0;
  u32* cmdbuf = getThreadCommandBuffer();

  cmdbuf[0] = IPC_MakeHeader(7, 1, 4);
  cmdbuf[1] = error;
  cmdbuf[2] = IPC_Desc_Buffer(strlen(title), IPC_BUFFER_R);
  cmdbuf[3] = (u32)title;
  cmdbuf[4] = IPC_Desc_Buffer(strlen(body), IPC_BUFFER_R);
  cmdbuf[5] = (u32)body;

  if (R_SUCCEEDED((res = svcSendSyncRequest(plgLdrHandle)))) res = cmdbuf[1];
  return res;
}

static s32 ReadExclusive(s32* address) {
  s32 value;
  do
    value = __ldrex(address);
  while (__strex(address, value));
  return value;
}

static void WriteExclusive(s32* address, s32 value) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  s32 old;
  do
    old = __ldrex(address);
  while (__strex(address, value));
#pragma GCC diagnostic pop
}

s32 PLGLDR__FetchEvent(void) { return ReadExclusive(plgEvent); }

void PLGLDR__Reply(s32 event) {
  WriteExclusive(plgReply, PLG_OK);

  if (event < PLG_ABOUT_TO_SWAP) {
    WriteExclusive(plgEvent, PLG_OK);
    return;
  }
  svcArbitrateAddress(plgLdrArbiter, (u32)plgReply, ARBITRATION_SIGNAL, 1, 0);
  if (event == PLG_ABOUT_TO_SWAP) {
    WriteExclusive(plgEvent, PLG_WAIT);
    svcArbitrateAddress(plgLdrArbiter, (u32)plgEvent,
                        ARBITRATION_WAIT_IF_LESS_THAN, PLG_OK, 0);
  } else if (event == PLG_ABOUT_TO_EXIT) {
    plgLdrExit();
    svcExitThread();
  } else {
    WriteExclusive(plgEvent, PLG_OK);
  }
}
