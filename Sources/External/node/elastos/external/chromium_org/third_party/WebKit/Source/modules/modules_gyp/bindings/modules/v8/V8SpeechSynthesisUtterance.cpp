// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py. DO NOT MODIFY!

#include "config.h"
#include "V8SpeechSynthesisUtterance.h"

#include "bindings/modules/v8/V8SpeechSynthesisVoice.h"
#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/V8AbstractEventListener.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8EventListenerList.h"
#include "bindings/v8/V8HiddenValue.h"
#include "bindings/v8/V8ObjectConstructor.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "core/frame/LocalDOMWindow.h"
#include "platform/RuntimeEnabledFeatures.h"
#include "platform/TraceEvent.h"
#include "wtf/GetPtr.h"
#include "wtf/RefPtr.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(SpeechSynthesisUtterance* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::fromObject(object)->setTypeInfo(&V8SpeechSynthesisUtterance::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

void webCoreInitializeScriptWrappableForInterface(WebCore::SpeechSynthesisUtterance* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8SpeechSynthesisUtterance::wrapperTypeInfo = { gin::kEmbedderBlink, V8SpeechSynthesisUtterance::domTemplate, V8SpeechSynthesisUtterance::derefObject, 0, V8SpeechSynthesisUtterance::toEventTarget, 0, V8SpeechSynthesisUtterance::installPerContextEnabledMethods, &V8EventTarget::wrapperTypeInfo, WrapperTypeObjectPrototype, GarbageCollectedObject };

namespace SpeechSynthesisUtteranceV8Internal {

template <typename T> void V8_USE(T) { }

static void textAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    v8SetReturnValueString(info, impl->text(), info.GetIsolate());
}

static void textAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::textAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void textAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    TOSTRING_VOID(V8StringResource<>, cppValue, v8Value);
    impl->setText(cppValue);
}

static void textAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::textAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void langAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    v8SetReturnValueString(info, impl->lang(), info.GetIsolate());
}

static void langAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::langAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void langAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    TOSTRING_VOID(V8StringResource<>, cppValue, v8Value);
    impl->setLang(cppValue);
}

static void langAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::langAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void voiceAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    v8SetReturnValueFast(info, WTF::getPtr(impl->voice()), impl);
}

static void voiceAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::voiceAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void voiceAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    TONATIVE_VOID(SpeechSynthesisVoice*, cppValue, V8SpeechSynthesisVoice::toNativeWithTypeCheck(info.GetIsolate(), v8Value));
    impl->setVoice(WTF::getPtr(cppValue));
}

static void voiceAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::voiceAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void volumeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    v8SetReturnValue(info, impl->volume());
}

static void volumeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::volumeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void volumeAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    TONATIVE_VOID(float, cppValue, static_cast<float>(v8Value->NumberValue()));
    impl->setVolume(cppValue);
}

static void volumeAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::volumeAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void rateAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    v8SetReturnValue(info, impl->rate());
}

static void rateAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::rateAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void rateAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    TONATIVE_VOID(float, cppValue, static_cast<float>(v8Value->NumberValue()));
    impl->setRate(cppValue);
}

static void rateAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::rateAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void pitchAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    v8SetReturnValue(info, impl->pitch());
}

static void pitchAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::pitchAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void pitchAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    TONATIVE_VOID(float, cppValue, static_cast<float>(v8Value->NumberValue()));
    impl->setPitch(cppValue);
}

static void pitchAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::pitchAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onstartAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    EventListener* v8Value = impl->onstart();
    v8SetReturnValue(info, v8Value ? v8::Handle<v8::Value>(V8AbstractEventListener::cast(v8Value)->getListenerObject(impl->executionContext())) : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
}

static void onstartAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::onstartAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onstartAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    moveEventListenerToNewWrapper(holder, impl->onstart(), v8Value, V8SpeechSynthesisUtterance::eventListenerCacheIndex, info.GetIsolate());
    impl->setOnstart(V8EventListenerList::getEventListener(ScriptState::current(info.GetIsolate()), v8Value, true, ListenerFindOrCreate));
}

static void onstartAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::onstartAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onendAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    EventListener* v8Value = impl->onend();
    v8SetReturnValue(info, v8Value ? v8::Handle<v8::Value>(V8AbstractEventListener::cast(v8Value)->getListenerObject(impl->executionContext())) : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
}

static void onendAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::onendAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onendAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    moveEventListenerToNewWrapper(holder, impl->onend(), v8Value, V8SpeechSynthesisUtterance::eventListenerCacheIndex, info.GetIsolate());
    impl->setOnend(V8EventListenerList::getEventListener(ScriptState::current(info.GetIsolate()), v8Value, true, ListenerFindOrCreate));
}

static void onendAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::onendAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onerrorAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    EventListener* v8Value = impl->onerror();
    v8SetReturnValue(info, v8Value ? v8::Handle<v8::Value>(V8AbstractEventListener::cast(v8Value)->getListenerObject(impl->executionContext())) : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
}

static void onerrorAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::onerrorAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onerrorAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    moveEventListenerToNewWrapper(holder, impl->onerror(), v8Value, V8SpeechSynthesisUtterance::eventListenerCacheIndex, info.GetIsolate());
    impl->setOnerror(V8EventListenerList::getEventListener(ScriptState::current(info.GetIsolate()), v8Value, true, ListenerFindOrCreate));
}

static void onerrorAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::onerrorAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onpauseAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    EventListener* v8Value = impl->onpause();
    v8SetReturnValue(info, v8Value ? v8::Handle<v8::Value>(V8AbstractEventListener::cast(v8Value)->getListenerObject(impl->executionContext())) : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
}

static void onpauseAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::onpauseAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onpauseAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    moveEventListenerToNewWrapper(holder, impl->onpause(), v8Value, V8SpeechSynthesisUtterance::eventListenerCacheIndex, info.GetIsolate());
    impl->setOnpause(V8EventListenerList::getEventListener(ScriptState::current(info.GetIsolate()), v8Value, true, ListenerFindOrCreate));
}

static void onpauseAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::onpauseAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onresumeAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    EventListener* v8Value = impl->onresume();
    v8SetReturnValue(info, v8Value ? v8::Handle<v8::Value>(V8AbstractEventListener::cast(v8Value)->getListenerObject(impl->executionContext())) : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
}

static void onresumeAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::onresumeAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onresumeAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    moveEventListenerToNewWrapper(holder, impl->onresume(), v8Value, V8SpeechSynthesisUtterance::eventListenerCacheIndex, info.GetIsolate());
    impl->setOnresume(V8EventListenerList::getEventListener(ScriptState::current(info.GetIsolate()), v8Value, true, ListenerFindOrCreate));
}

static void onresumeAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::onresumeAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onmarkAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    EventListener* v8Value = impl->onmark();
    v8SetReturnValue(info, v8Value ? v8::Handle<v8::Value>(V8AbstractEventListener::cast(v8Value)->getListenerObject(impl->executionContext())) : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
}

static void onmarkAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::onmarkAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onmarkAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    moveEventListenerToNewWrapper(holder, impl->onmark(), v8Value, V8SpeechSynthesisUtterance::eventListenerCacheIndex, info.GetIsolate());
    impl->setOnmark(V8EventListenerList::getEventListener(ScriptState::current(info.GetIsolate()), v8Value, true, ListenerFindOrCreate));
}

static void onmarkAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::onmarkAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onboundaryAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    EventListener* v8Value = impl->onboundary();
    v8SetReturnValue(info, v8Value ? v8::Handle<v8::Value>(V8AbstractEventListener::cast(v8Value)->getListenerObject(impl->executionContext())) : v8::Handle<v8::Value>(v8::Null(info.GetIsolate())));
}

static void onboundaryAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    SpeechSynthesisUtteranceV8Internal::onboundaryAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void onboundaryAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    SpeechSynthesisUtterance* impl = V8SpeechSynthesisUtterance::toNative(holder);
    moveEventListenerToNewWrapper(holder, impl->onboundary(), v8Value, V8SpeechSynthesisUtterance::eventListenerCacheIndex, info.GetIsolate());
    impl->setOnboundary(V8EventListenerList::getEventListener(ScriptState::current(info.GetIsolate()), v8Value, true, ListenerFindOrCreate));
}

static void onboundaryAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    SpeechSynthesisUtteranceV8Internal::onboundaryAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void constructor(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate* isolate = info.GetIsolate();
    V8StringResource<> text;
    {
        TOSTRING_VOID_INTERNAL(text, argumentOrNull(info, 0));
    }
    ExecutionContext* executionContext = currentExecutionContext(isolate);
    RawPtr<SpeechSynthesisUtterance> impl = SpeechSynthesisUtterance::create(executionContext, text);

    v8::Handle<v8::Object> wrapper = info.Holder();
    V8DOMWrapper::associateObjectWithWrapper<V8SpeechSynthesisUtterance>(impl.release(), &V8SpeechSynthesisUtterance::wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    v8SetReturnValue(info, wrapper);
}

} // namespace SpeechSynthesisUtteranceV8Internal

static const V8DOMConfiguration::AttributeConfiguration V8SpeechSynthesisUtteranceAttributes[] = {
    {"text", SpeechSynthesisUtteranceV8Internal::textAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::textAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"lang", SpeechSynthesisUtteranceV8Internal::langAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::langAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"voice", SpeechSynthesisUtteranceV8Internal::voiceAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::voiceAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"volume", SpeechSynthesisUtteranceV8Internal::volumeAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::volumeAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"rate", SpeechSynthesisUtteranceV8Internal::rateAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::rateAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"pitch", SpeechSynthesisUtteranceV8Internal::pitchAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::pitchAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"onstart", SpeechSynthesisUtteranceV8Internal::onstartAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::onstartAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"onend", SpeechSynthesisUtteranceV8Internal::onendAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::onendAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"onerror", SpeechSynthesisUtteranceV8Internal::onerrorAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::onerrorAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"onpause", SpeechSynthesisUtteranceV8Internal::onpauseAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::onpauseAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"onresume", SpeechSynthesisUtteranceV8Internal::onresumeAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::onresumeAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"onmark", SpeechSynthesisUtteranceV8Internal::onmarkAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::onmarkAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"onboundary", SpeechSynthesisUtteranceV8Internal::onboundaryAttributeGetterCallback, SpeechSynthesisUtteranceV8Internal::onboundaryAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
};

void V8SpeechSynthesisUtterance::constructorCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SCOPED_SAMPLING_STATE("Blink", "DOMConstructor");
    if (!info.IsConstructCall()) {
        throwTypeError(ExceptionMessages::constructorNotCallableAsFunction("SpeechSynthesisUtterance"), info.GetIsolate());
        return;
    }

    if (ConstructorMode::current(info.GetIsolate()) == ConstructorMode::WrapExistingObject) {
        v8SetReturnValue(info, info.Holder());
        return;
    }

    SpeechSynthesisUtteranceV8Internal::constructor(info);
}

static void configureV8SpeechSynthesisUtteranceTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    if (!RuntimeEnabledFeatures::speechSynthesisEnabled())
        defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "", V8EventTarget::domTemplate(isolate), V8SpeechSynthesisUtterance::internalFieldCount, 0, 0, 0, 0, 0, 0, isolate);
    else
        defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "SpeechSynthesisUtterance", V8EventTarget::domTemplate(isolate), V8SpeechSynthesisUtterance::internalFieldCount,
            V8SpeechSynthesisUtteranceAttributes, WTF_ARRAY_LENGTH(V8SpeechSynthesisUtteranceAttributes),
            0, 0,
            0, 0,
            isolate);
    functionTemplate->SetCallHandler(V8SpeechSynthesisUtterance::constructorCallback);
    functionTemplate->SetLength(0);
    v8::Local<v8::ObjectTemplate> instanceTemplate ALLOW_UNUSED = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> prototypeTemplate ALLOW_UNUSED = functionTemplate->PrototypeTemplate();

    // Custom toString template
    functionTemplate->Set(v8AtomicString(isolate, "toString"), V8PerIsolateData::from(isolate)->toStringTemplate());
}

v8::Handle<v8::FunctionTemplate> V8SpeechSynthesisUtterance::domTemplate(v8::Isolate* isolate)
{
    return V8DOMConfiguration::domClassTemplate(isolate, const_cast<WrapperTypeInfo*>(&wrapperTypeInfo), configureV8SpeechSynthesisUtteranceTemplate);
}

bool V8SpeechSynthesisUtterance::hasInstance(v8::Handle<v8::Value> v8Value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, v8Value);
}

v8::Handle<v8::Object> V8SpeechSynthesisUtterance::findInstanceInPrototypeChain(v8::Handle<v8::Value> v8Value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->findInstanceInPrototypeChain(&wrapperTypeInfo, v8Value);
}

SpeechSynthesisUtterance* V8SpeechSynthesisUtterance::toNativeWithTypeCheck(v8::Isolate* isolate, v8::Handle<v8::Value> value)
{
    return hasInstance(value, isolate) ? fromInternalPointer(v8::Handle<v8::Object>::Cast(value)->GetAlignedPointerFromInternalField(v8DOMWrapperObjectIndex)) : 0;
}

EventTarget* V8SpeechSynthesisUtterance::toEventTarget(v8::Handle<v8::Object> object)
{
    return toNative(object);
}

v8::Handle<v8::Object> wrap(SpeechSynthesisUtterance* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8SpeechSynthesisUtterance>(impl, isolate));
    return V8SpeechSynthesisUtterance::createWrapper(impl, creationContext, isolate);
}

v8::Handle<v8::Object> V8SpeechSynthesisUtterance::createWrapper(RawPtr<SpeechSynthesisUtterance> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8SpeechSynthesisUtterance>(impl.get(), isolate));
    if (ScriptWrappable::wrapperCanBeStoredInObject(impl.get())) {
        const WrapperTypeInfo* actualInfo = ScriptWrappable::fromObject(impl.get())->typeInfo();
        // Might be a XXXConstructor::wrapperTypeInfo instead of an XXX::wrapperTypeInfo. These will both have
        // the same object de-ref functions, though, so use that as the basis of the check.
        RELEASE_ASSERT_WITH_SECURITY_IMPLICATION(actualInfo->derefObjectFunction == wrapperTypeInfo.derefObjectFunction);
    }

    v8::Handle<v8::Object> wrapper = V8DOMWrapper::createWrapper(creationContext, &wrapperTypeInfo, toInternalPointer(impl.get()), isolate);
    if (UNLIKELY(wrapper.IsEmpty()))
        return wrapper;

    installPerContextEnabledProperties(wrapper, impl.get(), isolate);
    V8DOMWrapper::associateObjectWithWrapper<V8SpeechSynthesisUtterance>(impl, &wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}

void V8SpeechSynthesisUtterance::derefObject(void* object)
{
}

template<>
v8::Handle<v8::Value> toV8NoInline(SpeechSynthesisUtterance* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl, creationContext, isolate);
}

} // namespace WebCore
