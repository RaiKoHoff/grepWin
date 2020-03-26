// grepWin - regex search and replace for Windows

// Copyright (C) 2012-2013, 2016-2019 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "resource.h"
#include "maxpath.h"
#include "Settings.h"
#include "BrowseFolder.h"
#include "DirFileEnum.h"
#include <Commdlg.h>


CSettingsDlg::CSettingsDlg(HWND hParent)
    : m_hParent(hParent)
    , m_regEditorCmd(_T("Software\\grepWinNP3\\editorcmd"))
    , m_regEsc(_T("Software\\grepWinNP3\\escclose"), FALSE)
{
}

CSettingsDlg::~CSettingsDlg(void)
{
}

const wchar_t* const stdEditorCmd = _T(".\\Notepad3.exe /%mode% \"%pattern%\" /g %line% - %path%");

LRESULT CSettingsDlg::DlgFunc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            InitDialog(hwndDlg, IDI_GREPWIN);

            CLanguage::Instance().TranslateWindow(*this);
            AddToolTip(IDC_ONLYONE, TranslatedString(hResource, IDS_ONLYONE_TT).c_str());

            std::wstring editorCmd = bPortable ? g_iniFile.GetValue(L"global", L"editorcmd", L"") : std::wstring(m_regEditorCmd);
            if (editorCmd.empty()) 
                editorCmd = stdEditorCmd;

            SetDlgItemText(hwndDlg, IDC_EDITORCMD, editorCmd.c_str());

            wchar_t modulepath[MAX_PATH] = {0};
            GetModuleFileName(NULL, modulepath, MAX_PATH);
            PathRemoveFileSpec(modulepath);
            std::wstring path = modulepath;
            bool bRecurse = false;
            bool bIsDirectory = false;
            std::wstring sPath;
            CRegStdString regLang(L"Software\\grepWinNP3\\languagefile");
            std::wstring  setLang = regLang;

            if (bPortable)
            {
                const std::wstring& languagefile = g_iniFile.GetValue(L"global", L"languagefile", L"");

                if (PathIsRelative(languagefile.c_str()))
                {
                    PathAppend(modulepath, languagefile.c_str());
                    setLang = modulepath;
                }
                else
                    setLang = languagefile;

                // need to adapt file enumerator path
                if (!languagefile.empty())
                {
                    lstrcpyn(modulepath, setLang.c_str(), MAX_PATH);
                    PathRemoveFileSpec(modulepath);
                }
                path = modulepath;
            }

            int index = 1;
            int langIndex = 0;
            SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)L"English (United States) [en-US]");

            CDirFileEnum fileEnumerator(path.c_str());
            while (fileEnumerator.NextFile(sPath, &bIsDirectory, bRecurse))
            {
                size_t dotpos = sPath.find_last_of('.');
                if (dotpos == std::wstring::npos)
                    continue;
                std::wstring ext = sPath.substr(dotpos);
                if (ext.compare(L".lang"))
                    continue;
                m_langpaths.push_back(sPath);
                if (sPath.compare(setLang)==0)
                    langIndex = index;
                size_t slashpos = sPath.find_last_of('\\');
                if (slashpos == std::wstring::npos)
                    continue;
                sPath = sPath.substr(slashpos+1);
                dotpos = sPath.find_last_of('.');
                sPath = sPath.substr(0, dotpos);
                SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)sPath.c_str());
                ++index;
            }

            SendDlgItemMessage(hwndDlg, IDC_LANGUAGE, CB_SETCURSEL, langIndex, 0);
            SendDlgItemMessage(hwndDlg, IDC_ESCKEY, BM_SETCHECK, bPortable ? g_iniFile.GetBoolValue(L"settings", L"escclose", false) : !!DWORD(CRegStdDWORD(L"Software\\grepWinNP3\\escclose", FALSE)) ? BST_CHECKED : BST_UNCHECKED, 0);
            SendDlgItemMessage(hwndDlg, IDC_BACKUPINFOLDER, BM_SETCHECK, bPortable ? g_iniFile.GetBoolValue(L"settings", L"backupinfolder", false) : !!DWORD(CRegStdDWORD(L"Software\\grepWinNP3\\backupinfolder", FALSE)) ? BST_CHECKED : BST_UNCHECKED, 0);
            SendDlgItemMessage(hwndDlg, IDC_ONLYONE, BM_SETCHECK, bPortable ? g_iniFile.GetBoolValue(L"global", L"onlyone", false) : !!DWORD(CRegStdDWORD(L"Software\\grepWinNP3\\onlyone", FALSE)) ? BST_CHECKED : BST_UNCHECKED, 0);

            AddToolTip(IDC_BACKUPINFOLDER, TranslatedString(hResource, IDS_BACKUPINFOLDER_TT).c_str());

            m_resizer.Init(hwndDlg);
            m_resizer.AddControl(hwndDlg, IDC_EDITORGROUP, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_RESETDEFAULT, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_EDITORCMD, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_SEARCHPATHBROWSE, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC1, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC2, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC3, RESIZER_TOPLEFT);
            m_resizer.AddControl(hwndDlg, IDC_STATIC4, RESIZER_TOPLEFT);
            m_resizer.AddControl(hwndDlg, IDC_LANGUAGE, RESIZER_TOPRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_ESCKEY, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_BACKUPINFOLDER, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_ONLYONE, RESIZER_TOPLEFTRIGHT);
            m_resizer.AddControl(hwndDlg, IDC_DWM, RESIZER_BOTTOMLEFT);
            m_resizer.AddControl(hwndDlg, IDOK, RESIZER_BOTTOMRIGHT);
            m_resizer.AddControl(hwndDlg, IDCANCEL, RESIZER_BOTTOMRIGHT);
        }
        return TRUE;
    case WM_COMMAND:
        return DoCommand(LOWORD(wParam), HIWORD(wParam));
    case WM_SIZE:
        {
            m_resizer.DoResize(LOWORD(lParam), HIWORD(lParam));
        }
        break;
    case WM_GETMINMAXINFO:
        {
            MINMAXINFO * mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = m_resizer.GetDlgRectScreen()->right;
            mmi->ptMinTrackSize.y = m_resizer.GetDlgRectScreen()->bottom;
            return 0;
        }
        break;
    default:
        return FALSE;
    }
    return FALSE;
}

LRESULT CSettingsDlg::DoCommand(int id, int /*msg*/)
{
    switch (id)
    {
    case IDC_RESETDEFAULT:
            SetDlgItemText(*this, IDC_EDITORCMD, stdEditorCmd);
        break;
    case IDOK:
        {
            auto buf = GetDlgItemText(IDC_EDITORCMD);
            if (bPortable)
                g_iniFile.SetValue(L"global", L"editorcmd", buf.get());
            else
                m_regEditorCmd = buf.get();
            int langIndex = (int)SendDlgItemMessage(*this, IDC_LANGUAGE, CB_GETCURSEL, 0, 0);
            std::wstring langpath = langIndex==0 ? L"" : m_langpaths[langIndex-1];
            if (bPortable)
            {
                TCHAR wchLngPath[MAX_PATH] = {L'\0'};
                GetModuleFileName(NULL, wchLngPath, MAX_PATH);
                TCHAR wchAppPath[MAX_PATH] = {L'\0'};
                PathCanonicalize(wchAppPath, wchLngPath);
                if (PathRelativePathTo(wchLngPath, wchAppPath, FILE_ATTRIBUTE_NORMAL, langpath.c_str(), FILE_ATTRIBUTE_NORMAL))
                    g_iniFile.SetValue(L"global", L"languagefile", wchLngPath);
                else
                    g_iniFile.SetValue(L"global", L"languagefile", langpath.c_str());
            }
            else
            {
                CRegStdString regLang(L"Software\\grepWinNP3\\languagefile");
                if (langIndex==0)
                {
                    regLang.removeValue();
                }
                else
                {
                    regLang = langpath;
                }
            }
            CLanguage::Instance().LoadFile(langpath);
            CLanguage::Instance().TranslateWindow(::GetParent(*this));

            if (bPortable)
            {
                g_iniFile.SetBoolValue(L"settings", L"escclose", (IsDlgButtonChecked(*this, IDC_ESCKEY) == BST_CHECKED));
                g_iniFile.SetBoolValue(L"settings", L"backupinfolder", (IsDlgButtonChecked(*this, IDC_BACKUPINFOLDER) == BST_CHECKED));
                g_iniFile.SetBoolValue(L"global", L"onlyone", (IsDlgButtonChecked(*this, IDC_ONLYONE) == BST_CHECKED));
            }
            else
            {
                CRegStdDWORD esc(L"Software\\grepWinNP3\\escclose", FALSE);
                esc = (IsDlgButtonChecked(*this, IDC_ESCKEY) == BST_CHECKED);
                CRegStdDWORD backup(L"Software\\grepWinNP3\\backupinfolder", FALSE);
                backup = (IsDlgButtonChecked(*this, IDC_BACKUPINFOLDER) == BST_CHECKED);
                CRegStdDWORD regOnlyOne(L"Software\\grepWinNP3\\onlyone", FALSE);
                regOnlyOne = (IsDlgButtonChecked(*this, IDC_ONLYONE) == BST_CHECKED);
            }
    }
        // fall through
    case IDCANCEL:
        EndDialog(*this, id);
        break;
    case IDC_SEARCHPATHBROWSE:
        {
            OPENFILENAME ofn = {0};     // common dialog box structure
            TCHAR szFile[MAX_PATH] = {0};  // buffer for file name
            // Initialize OPENFILENAME
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = *this;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = _countof(szFile);
            std::wstring sTitle = TranslatedString(hResource, IDS_SELECTEDITOR);
            ofn.lpstrTitle = sTitle.c_str();
            ofn.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_PATHMUSTEXIST|OFN_DONTADDTORECENT;
            auto sProgs = TranslatedString(hResource, IDS_PROGRAMS);
            auto sAllFiles = TranslatedString(hResource, IDS_ALLFILES);
            auto sFilter = sProgs;
            sFilter.append(L"\0*.exe;*.com\0", _countof(L"\0*.exe;*.com\0")-1);
            sFilter.append(sAllFiles);
            sFilter.append(L"\0*.*\0\0", _countof(L"\0*.*\0\0")-1);
            ofn.lpstrFilter = sFilter.c_str();
            ofn.nFilterIndex = 1;
            // Display the Open dialog box.
            if (GetOpenFileName(&ofn)==TRUE)
            {
                SetDlgItemText(*this, IDC_EDITORCMD, szFile);
            }
        }
        break;
    }
    return 1;
}

