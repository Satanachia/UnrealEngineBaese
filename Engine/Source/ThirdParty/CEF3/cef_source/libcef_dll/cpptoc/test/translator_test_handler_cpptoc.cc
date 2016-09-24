// Copyright (c) 2016 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//

#include "libcef_dll/cpptoc/test/translator_test_handler_cpptoc.h"
#include "libcef_dll/cpptoc/test/translator_test_handler_child_cpptoc.h"


namespace {

// MEMBER FUNCTIONS - Body may be edited by hand.

int CEF_CALLBACK translator_test_handler_get_value(
    struct _cef_translator_test_handler_t* self) {
  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  DCHECK(self);
  if (!self)
    return 0;

  // Execute
  int _retval = CefTranslatorTestHandlerCppToC::Get(self)->GetValue();

  // Return type: simple
  return _retval;
}

}  // namespace


// CONSTRUCTOR - Do not edit by hand.

CefTranslatorTestHandlerCppToC::CefTranslatorTestHandlerCppToC() {
  GetStruct()->get_value = translator_test_handler_get_value;
}

template<> CefRefPtr<CefTranslatorTestHandler> CefCppToC<CefTranslatorTestHandlerCppToC,
    CefTranslatorTestHandler, cef_translator_test_handler_t>::UnwrapDerived(
    CefWrapperType type, cef_translator_test_handler_t* s) {
  if (type == WT_TRANSLATOR_TEST_HANDLER_CHILD) {
    return CefTranslatorTestHandlerChildCppToC::Unwrap(
        reinterpret_cast<cef_translator_test_handler_child_t*>(s));
  }
  NOTREACHED() << "Unexpected class type: " << type;
  return NULL;
}

#ifndef NDEBUG
template<> base::AtomicRefCount CefCppToC<CefTranslatorTestHandlerCppToC,
    CefTranslatorTestHandler, cef_translator_test_handler_t>::DebugObjCt = 0;
#endif

template<> CefWrapperType CefCppToC<CefTranslatorTestHandlerCppToC,
    CefTranslatorTestHandler, cef_translator_test_handler_t>::kWrapperType =
    WT_TRANSLATOR_TEST_HANDLER;