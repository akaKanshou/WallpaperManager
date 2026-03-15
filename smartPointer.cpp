#include "smartPointer.h"

void COMHelper::getHResult(HRESULT hr) {
    static int call = 0;
    if (FAILED(hr)) {
        std::wstring show(L"ERROR CODE: ");
        show += std::to_wstring((UINT)hr);

        show += L"\nFAILED ON CALL:";
        show += std::to_wstring(call);

        MessageBoxW(NULL, show.c_str(), L"HRESULT ERROR", MB_OK);
        exit(hr);
    }
    call++;
}
