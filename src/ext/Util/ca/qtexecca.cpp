// Copyright (c) .NET Foundation and contributors. All rights reserved. Licensed under the Microsoft Reciprocal License. See LICENSE.TXT file in the project root for full license information.

#include "precomp.h"

#define OUTPUT_BUFFER 1024

// These old "CA" prefix names are deprecated, and intended to go away in wix 4.0, only staying now for compatibility reasons
const LPCWSTR CAQUIET_TIMEOUT_PROPERTY = L"QtExecCmdTimeout";
const LPCWSTR CAQUIET_ARGUMENTS_PROPERTY = L"QtExecCmdLine";
const LPCWSTR CAQUIET64_ARGUMENTS_PROPERTY = L"QtExec64CmdLine";
// end deprecated section

// WixCA name quiet commandline argument properties
const LPCWSTR WIX_QUIET_ARGUMENTS_PROPERTY = L"WixQuietExecCmdLine";
const LPCWSTR WIX_QUIET64_ARGUMENTS_PROPERTY = L"WixQuietExec64CmdLine";

// WixCA quiet timeout properties
const LPCWSTR WIX_QUIET_TIMEOUT_PROPERTY = L"WixQuietExecCmdTimeout";
const LPCWSTR WIX_QUIET64_TIMEOUT_PROPERTY = L"WixQuietExec64CmdTimeout";

// WixCA silent commandline argument properties
const LPCWSTR WIX_SILENT_ARGUMENTS_PROPERTY = L"WixSilentExecCmdLine";
const LPCWSTR WIX_SILENT64_ARGUMENTS_PROPERTY = L"WixSilentExec64CmdLine";

// WixCA silent timeout properties
const LPCWSTR WIX_SILENT_TIMEOUT_PROPERTY = L"WixSilentExecCmdTimeout";
const LPCWSTR WIX_SILENT64_TIMEOUT_PROPERTY = L"WixSilentExec64CmdTimeout";

HRESULT BuildCommandLine(
    __in LPCWSTR wzProperty,
    __out LPWSTR *ppwzCommand
    )
{
    Assert(ppwzCommand);

    HRESULT hr = S_OK;
    BOOL fScheduled = ::MsiGetMode(WcaGetInstallHandle(), MSIRUNMODE_SCHEDULED);
    BOOL fRollback = ::MsiGetMode(WcaGetInstallHandle(), MSIRUNMODE_ROLLBACK);
    BOOL fCommit = ::MsiGetMode(WcaGetInstallHandle(), MSIRUNMODE_COMMIT);

    if (fScheduled || fRollback || fCommit)
    {
        if (WcaIsPropertySet("CustomActionData"))
        {
            hr = WcaGetProperty( L"CustomActionData", ppwzCommand);
            ExitOnFailure(hr, "Failed to get CustomActionData");
        }
    }
    else if (WcaIsUnicodePropertySet(wzProperty))
    {
        hr = WcaGetFormattedProperty(wzProperty, ppwzCommand);
        ExitOnFailure(hr, "Failed to get %ls", wzProperty);
        hr = WcaSetProperty(wzProperty, L""); // clear out the property now that we've read it
        ExitOnFailure(hr, "Failed to set %ls", wzProperty);
    }

    if (!*ppwzCommand)
    {
        ExitOnFailure(hr = E_INVALIDARG, "Failed to get command line data");
    }

    if (L'"' != **ppwzCommand)
    {
        WcaLog(LOGMSG_STANDARD, "Command string must begin with quoted application name.");
        ExitOnFailure(hr = E_INVALIDARG, "invalid command line property value");
    }

LExit:
    return hr;
}

#define ONEMINUTE 60000

DWORD GetTimeout(LPCWSTR wzPropertyName)
{
    DWORD dwTimeout = ONEMINUTE;
    HRESULT hr = S_OK;

    LPWSTR pwzData = NULL;

    if (WcaIsUnicodePropertySet(wzPropertyName))
    {
        hr = WcaGetProperty(wzPropertyName, &pwzData);
        ExitOnFailure(hr, "Failed to get %ls", wzPropertyName);

        if ((dwTimeout = (DWORD)_wtoi(pwzData)) == 0)
        {
            dwTimeout = ONEMINUTE;
        }
    }

LExit:
    ReleaseStr(pwzData);

    return dwTimeout;

}

HRESULT ExecCommon(
    __in LPCWSTR wzArgumentsProperty,
    __in LPCWSTR wzTimeoutProperty,
    __in BOOL fLogCommand,
    __in BOOL fLogOutput
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzCommand = NULL;
    DWORD dwTimeout = 0;

    hr = BuildCommandLine(wzArgumentsProperty, &pwzCommand);
    ExitOnFailure(hr, "Failed to get Command Line");

    dwTimeout = GetTimeout(wzTimeoutProperty);

    hr = QuietExec(pwzCommand, dwTimeout, fLogCommand, fLogOutput);
    ExitOnFailure(hr, "QuietExec Failed");

LExit:
    ReleaseStr(pwzCommand);

    return hr;
}

HRESULT ExecCommon64(
    __in LPCWSTR wzArgumentsProperty,
    __in LPCWSTR wzTimeoutProperty,
    __in BOOL fLogCommand,
    __in BOOL fLogOutput
    )
{
    HRESULT hr = S_OK;
    LPWSTR pwzCommand = NULL;
    DWORD dwTimeout = 0;
#ifndef _WIN64
    BOOL fIsWow64Initialized = FALSE;
    BOOL fRedirected = FALSE;

    hr = WcaInitializeWow64();
    if (S_FALSE == hr)
    {
        hr = TYPE_E_DLLFUNCTIONNOTFOUND;
    }
    ExitOnFailure(hr, "Failed to intialize WOW64.");
    fIsWow64Initialized = TRUE;

    hr = WcaDisableWow64FSRedirection();
    ExitOnFailure(hr, "Failed to enable filesystem redirection.");
    fRedirected = TRUE;
#endif

    hr = BuildCommandLine(wzArgumentsProperty, &pwzCommand);
    ExitOnFailure(hr, "Failed to get Command Line");

    dwTimeout = GetTimeout(wzTimeoutProperty);

    hr = QuietExec(pwzCommand, dwTimeout, fLogCommand, fLogOutput);
    ExitOnFailure(hr, "QuietExec64 Failed");

LExit:
    ReleaseStr(pwzCommand);

#ifndef _WIN64
    if (fRedirected)
    {
        WcaRevertWow64FSRedirection();
    }

    if (fIsWow64Initialized)
    {
        WcaFinalizeWow64();
    }
#endif

    return hr;
}

extern "C" UINT __stdcall WixQuietExec(
    __in MSIHANDLE hInstall
    )
{
    Assert(hInstall);
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, "WixQuietExec");
    ExitOnFailure(hr, "Failed to initialize");

    hr = ExecCommon(WIX_QUIET_ARGUMENTS_PROPERTY, WIX_QUIET_TIMEOUT_PROPERTY, TRUE, TRUE);
    ExitOnFailure(hr, "Failed in ExecCommon method");

LExit:
    if (FAILED(hr))
    {
        er = ERROR_INSTALL_FAILURE;
    }

    return WcaFinalize(er); 
}

extern "C" UINT __stdcall WixQuietExec64(
    __in MSIHANDLE hInstall
    )
{
    Assert(hInstall);
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, "WixQuietExec64");
    ExitOnFailure(hr, "Failed to initialize");

    hr = ExecCommon64(WIX_QUIET64_ARGUMENTS_PROPERTY, WIX_QUIET64_TIMEOUT_PROPERTY, TRUE, TRUE);
    ExitOnFailure(hr, "Failed in ExecCommon method");

LExit:
    if (FAILED(hr))
    {
        er = ERROR_INSTALL_FAILURE;
    }

    return WcaFinalize(er);
}

extern "C" UINT __stdcall WixSilentExec(
    __in MSIHANDLE hInstall
    )
{
    Assert(hInstall);
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, "WixSilentExec");
    ExitOnFailure(hr, "Failed to initialize");

    hr = ExecCommon(WIX_SILENT_ARGUMENTS_PROPERTY, WIX_SILENT_TIMEOUT_PROPERTY, FALSE, FALSE);
    ExitOnFailure(hr, "Failed in ExecCommon method");

LExit:
    if (FAILED(hr))
    {
        er = ERROR_INSTALL_FAILURE;
    }

    return WcaFinalize(er); 
}

extern "C" UINT __stdcall WixSilentExec64(
    __in MSIHANDLE hInstall
    )
{
    Assert(hInstall);
    HRESULT hr = S_OK;
    UINT er = ERROR_SUCCESS;

    hr = WcaInitialize(hInstall, "WixSilentExec64");
    ExitOnFailure(hr, "Failed to initialize");

    hr = ExecCommon64(WIX_SILENT64_ARGUMENTS_PROPERTY, WIX_SILENT64_TIMEOUT_PROPERTY, FALSE, FALSE);
    ExitOnFailure(hr, "Failed in ExecCommon method");

LExit:
    if (FAILED(hr))
    {
        er = ERROR_INSTALL_FAILURE;
    }

    return WcaFinalize(er);
}
