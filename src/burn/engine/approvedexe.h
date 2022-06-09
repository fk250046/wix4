#pragma once
// Copyright (c) .NET Foundation and contributors. All rights reserved. Licensed under the Microsoft Reciprocal License. See LICENSE.TXT file in the project root for full license information.


#if defined(__cplusplus)
extern "C" {
#endif

// forward declare

enum BURN_MODE;
typedef struct _BOOTSTRAPPER_ENGINE_CONTEXT BOOTSTRAPPER_ENGINE_CONTEXT;
typedef struct _BURN_CACHE BURN_CACHE;
typedef struct _BURN_DEPENDENCIES BURN_DEPENDENCIES;
typedef struct _BURN_ENGINE_COMMAND BURN_ENGINE_COMMAND;
typedef struct  _BURN_LOGGING BURN_LOGGING;

// structs

typedef struct _BURN_APPROVED_EXE
{
    LPWSTR sczId;
    LPWSTR sczKey;
    LPWSTR sczValueName;
    BOOL fWin64;
} BURN_APPROVED_EXE;

typedef struct _BURN_APPROVED_EXES
{
    BURN_APPROVED_EXE* rgApprovedExes;
    DWORD cApprovedExes;
} BURN_APPROVED_EXES;

typedef struct _BURN_LAUNCH_APPROVED_EXE
{
    HWND hwndParent;
    LPWSTR sczId;
    LPWSTR sczExecutablePath;
    LPWSTR sczArguments;
    DWORD dwWaitForInputIdleTimeout;
} BURN_LAUNCH_APPROVED_EXE;


// function declarations

HRESULT ApprovedExesParseFromXml(
    __in BURN_APPROVED_EXES* pApprovedExes,
    __in IXMLDOMNode* pixnBundle
    );

void ApprovedExesUninitialize(
    __in BURN_APPROVED_EXES* pApprovedExes
    );
void ApprovedExesUninitializeLaunch(
    __in BURN_LAUNCH_APPROVED_EXE* pLaunchApprovedExe
    );
HRESULT ApprovedExesFindById(
    __in BURN_APPROVED_EXES* pApprovedExes,
    __in_z LPCWSTR wzId,
    __out BURN_APPROVED_EXE** ppApprovedExe
    );
HRESULT ApprovedExesLaunch(
    __in BURN_VARIABLES* pVariables,
    __in BURN_LAUNCH_APPROVED_EXE* pLaunchApprovedExe,
    __out DWORD* pdwProcessId
    );
HRESULT ApprovedExesVerifySecureLocation(
    __in BURN_CACHE* pCache,
    __in BURN_VARIABLES* pVariables,
    __in LPCWSTR wzExecutablePath
    );


#if defined(__cplusplus)
}
#endif
