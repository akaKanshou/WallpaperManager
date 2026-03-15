#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include <string>
#include <vector>

#include <ShObjIdl.h>
#include <windows.h>

namespace COMHelper {

void getHResult(HRESULT hr);

template <typename T> class CORleasePointer {
  private:
    T *ptr;

  public:
    CORleasePointer(T *p = NULL) : ptr(p) {
        if (ptr) ptr->AddRef();
    }

    CORleasePointer(CORleasePointer &sp) : ptr(sp.ptr) {
        if (sp.get()) sp->AddRef();
    }

    CORleasePointer &operator=(CORleasePointer &sp) {
        ptr = sp.ptr;
        if (sp.get()) sp->AddRef();
        return *this;
    }

    T *get() const { return ptr; }

    T *operator->() { return ptr; }

    T **operator&() { return &ptr; }

    ~CORleasePointer() {
        if (ptr) {
            ptr->Release();
        }
    }
};

template <typename T> class COFreePointer {
  private:
    T ptr;

  public:
    COFreePointer(T p = NULL) : ptr(p) { p = NULL; }

    COFreePointer(COFreePointer &cfp) : ptr(cfp.ptr) { cfp.ptr = NULL; }

    COFreePointer &operator=(const COFreePointer &cfp) {
        ptr = cfp.ptr;
        cfp.ptr = NULL;
        return *this;
    }

    void swap(T p) {
        if (ptr) CoTaskMemFree(ptr);
        ptr = p;
    }

    T get() const { return ptr; }

    T *operator&() { return &ptr; }

    ~COFreePointer() {
        if (ptr) {
            CoTaskMemFree(ptr);
        }
    }
};

} // namespace COMHelper
