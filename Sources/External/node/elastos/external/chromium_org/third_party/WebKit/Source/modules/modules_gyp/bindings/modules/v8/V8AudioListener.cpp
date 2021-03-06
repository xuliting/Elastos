// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py. DO NOT MODIFY!

#include "config.h"
#if ENABLE(WEB_AUDIO)
#include "V8AudioListener.h"

#include "bindings/v8/ExceptionState.h"
#include "bindings/v8/V8DOMConfiguration.h"
#include "bindings/v8/V8HiddenValue.h"
#include "bindings/v8/V8ObjectConstructor.h"
#include "core/dom/ContextFeatures.h"
#include "core/dom/Document.h"
#include "platform/RuntimeEnabledFeatures.h"
#include "platform/TraceEvent.h"
#include "wtf/GetPtr.h"
#include "wtf/RefPtr.h"

namespace WebCore {

static void initializeScriptWrappableForInterface(AudioListener* object)
{
    if (ScriptWrappable::wrapperCanBeStoredInObject(object))
        ScriptWrappable::fromObject(object)->setTypeInfo(&V8AudioListener::wrapperTypeInfo);
    else
        ASSERT_NOT_REACHED();
}

} // namespace WebCore

void webCoreInitializeScriptWrappableForInterface(WebCore::AudioListener* object)
{
    WebCore::initializeScriptWrappableForInterface(object);
}

namespace WebCore {
const WrapperTypeInfo V8AudioListener::wrapperTypeInfo = { gin::kEmbedderBlink, V8AudioListener::domTemplate, V8AudioListener::derefObject, 0, 0, 0, V8AudioListener::installPerContextEnabledMethods, 0, WrapperTypeObjectPrototype, WillBeGarbageCollectedObject };

namespace AudioListenerV8Internal {

template <typename T> void V8_USE(T) { }

static void dopplerFactorAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    AudioListener* impl = V8AudioListener::toNative(holder);
    v8SetReturnValue(info, impl->dopplerFactor());
}

static void dopplerFactorAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    AudioListenerV8Internal::dopplerFactorAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void dopplerFactorAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    AudioListener* impl = V8AudioListener::toNative(holder);
    TONATIVE_VOID(float, cppValue, static_cast<float>(v8Value->NumberValue()));
    impl->setDopplerFactor(cppValue);
}

static void dopplerFactorAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    AudioListenerV8Internal::dopplerFactorAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void speedOfSoundAttributeGetter(const v8::PropertyCallbackInfo<v8::Value>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    AudioListener* impl = V8AudioListener::toNative(holder);
    v8SetReturnValue(info, impl->speedOfSound());
}

static void speedOfSoundAttributeGetterCallback(v8::Local<v8::String>, const v8::PropertyCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMGetter");
    AudioListenerV8Internal::speedOfSoundAttributeGetter(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void speedOfSoundAttributeSetter(v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    v8::Handle<v8::Object> holder = info.Holder();
    AudioListener* impl = V8AudioListener::toNative(holder);
    TONATIVE_VOID(float, cppValue, static_cast<float>(v8Value->NumberValue()));
    impl->setSpeedOfSound(cppValue);
}

static void speedOfSoundAttributeSetterCallback(v8::Local<v8::String>, v8::Local<v8::Value> v8Value, const v8::PropertyCallbackInfo<void>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMSetter");
    AudioListenerV8Internal::speedOfSoundAttributeSetter(v8Value, info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void setPositionMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 3)) {
        throwMinimumArityTypeErrorForMethod("setPosition", "AudioListener", 3, info.Length(), info.GetIsolate());
        return;
    }
    AudioListener* impl = V8AudioListener::toNative(info.Holder());
    float x;
    float y;
    float z;
    {
        v8::TryCatch block;
        V8RethrowTryCatchScope rethrow(block);
        TONATIVE_VOID_INTERNAL(x, static_cast<float>(info[0]->NumberValue()));
        TONATIVE_VOID_INTERNAL(y, static_cast<float>(info[1]->NumberValue()));
        TONATIVE_VOID_INTERNAL(z, static_cast<float>(info[2]->NumberValue()));
    }
    impl->setPosition(x, y, z);
}

static void setPositionMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    AudioListenerV8Internal::setPositionMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void setOrientationMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 6)) {
        throwMinimumArityTypeErrorForMethod("setOrientation", "AudioListener", 6, info.Length(), info.GetIsolate());
        return;
    }
    AudioListener* impl = V8AudioListener::toNative(info.Holder());
    float x;
    float y;
    float z;
    float xUp;
    float yUp;
    float zUp;
    {
        v8::TryCatch block;
        V8RethrowTryCatchScope rethrow(block);
        TONATIVE_VOID_INTERNAL(x, static_cast<float>(info[0]->NumberValue()));
        TONATIVE_VOID_INTERNAL(y, static_cast<float>(info[1]->NumberValue()));
        TONATIVE_VOID_INTERNAL(z, static_cast<float>(info[2]->NumberValue()));
        TONATIVE_VOID_INTERNAL(xUp, static_cast<float>(info[3]->NumberValue()));
        TONATIVE_VOID_INTERNAL(yUp, static_cast<float>(info[4]->NumberValue()));
        TONATIVE_VOID_INTERNAL(zUp, static_cast<float>(info[5]->NumberValue()));
    }
    impl->setOrientation(x, y, z, xUp, yUp, zUp);
}

static void setOrientationMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    AudioListenerV8Internal::setOrientationMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

static void setVelocityMethod(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    if (UNLIKELY(info.Length() < 3)) {
        throwMinimumArityTypeErrorForMethod("setVelocity", "AudioListener", 3, info.Length(), info.GetIsolate());
        return;
    }
    AudioListener* impl = V8AudioListener::toNative(info.Holder());
    float x;
    float y;
    float z;
    {
        v8::TryCatch block;
        V8RethrowTryCatchScope rethrow(block);
        TONATIVE_VOID_INTERNAL(x, static_cast<float>(info[0]->NumberValue()));
        TONATIVE_VOID_INTERNAL(y, static_cast<float>(info[1]->NumberValue()));
        TONATIVE_VOID_INTERNAL(z, static_cast<float>(info[2]->NumberValue()));
    }
    impl->setVelocity(x, y, z);
}

static void setVelocityMethodCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    TRACE_EVENT_SET_SAMPLING_STATE("Blink", "DOMMethod");
    AudioListenerV8Internal::setVelocityMethod(info);
    TRACE_EVENT_SET_SAMPLING_STATE("V8", "V8Execution");
}

} // namespace AudioListenerV8Internal

static const V8DOMConfiguration::AttributeConfiguration V8AudioListenerAttributes[] = {
    {"dopplerFactor", AudioListenerV8Internal::dopplerFactorAttributeGetterCallback, AudioListenerV8Internal::dopplerFactorAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
    {"speedOfSound", AudioListenerV8Internal::speedOfSoundAttributeGetterCallback, AudioListenerV8Internal::speedOfSoundAttributeSetterCallback, 0, 0, 0, static_cast<v8::AccessControl>(v8::DEFAULT), static_cast<v8::PropertyAttribute>(v8::None), 0 /* on instance */},
};

static const V8DOMConfiguration::MethodConfiguration V8AudioListenerMethods[] = {
    {"setPosition", AudioListenerV8Internal::setPositionMethodCallback, 0, 3},
    {"setOrientation", AudioListenerV8Internal::setOrientationMethodCallback, 0, 6},
    {"setVelocity", AudioListenerV8Internal::setVelocityMethodCallback, 0, 3},
};

static void configureV8AudioListenerTemplate(v8::Handle<v8::FunctionTemplate> functionTemplate, v8::Isolate* isolate)
{
    functionTemplate->ReadOnlyPrototype();

    v8::Local<v8::Signature> defaultSignature;
    defaultSignature = V8DOMConfiguration::installDOMClassTemplate(functionTemplate, "AudioListener", v8::Local<v8::FunctionTemplate>(), V8AudioListener::internalFieldCount,
        V8AudioListenerAttributes, WTF_ARRAY_LENGTH(V8AudioListenerAttributes),
        0, 0,
        V8AudioListenerMethods, WTF_ARRAY_LENGTH(V8AudioListenerMethods),
        isolate);
    v8::Local<v8::ObjectTemplate> instanceTemplate ALLOW_UNUSED = functionTemplate->InstanceTemplate();
    v8::Local<v8::ObjectTemplate> prototypeTemplate ALLOW_UNUSED = functionTemplate->PrototypeTemplate();

    // Custom toString template
    functionTemplate->Set(v8AtomicString(isolate, "toString"), V8PerIsolateData::from(isolate)->toStringTemplate());
}

v8::Handle<v8::FunctionTemplate> V8AudioListener::domTemplate(v8::Isolate* isolate)
{
    return V8DOMConfiguration::domClassTemplate(isolate, const_cast<WrapperTypeInfo*>(&wrapperTypeInfo), configureV8AudioListenerTemplate);
}

bool V8AudioListener::hasInstance(v8::Handle<v8::Value> v8Value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->hasInstance(&wrapperTypeInfo, v8Value);
}

v8::Handle<v8::Object> V8AudioListener::findInstanceInPrototypeChain(v8::Handle<v8::Value> v8Value, v8::Isolate* isolate)
{
    return V8PerIsolateData::from(isolate)->findInstanceInPrototypeChain(&wrapperTypeInfo, v8Value);
}

AudioListener* V8AudioListener::toNativeWithTypeCheck(v8::Isolate* isolate, v8::Handle<v8::Value> value)
{
    return hasInstance(value, isolate) ? fromInternalPointer(v8::Handle<v8::Object>::Cast(value)->GetAlignedPointerFromInternalField(v8DOMWrapperObjectIndex)) : 0;
}

v8::Handle<v8::Object> wrap(AudioListener* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8AudioListener>(impl, isolate));
    return V8AudioListener::createWrapper(impl, creationContext, isolate);
}

v8::Handle<v8::Object> V8AudioListener::createWrapper(PassRefPtrWillBeRawPtr<AudioListener> impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    ASSERT(impl);
    ASSERT(!DOMDataStore::containsWrapper<V8AudioListener>(impl.get(), isolate));
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
    V8DOMWrapper::associateObjectWithWrapper<V8AudioListener>(impl, &wrapperTypeInfo, wrapper, isolate, WrapperConfiguration::Independent);
    return wrapper;
}

void V8AudioListener::derefObject(void* object)
{
#if !ENABLE(OILPAN)
    fromInternalPointer(object)->deref();
#endif // !ENABLE(OILPAN)
}

template<>
v8::Handle<v8::Value> toV8NoInline(AudioListener* impl, v8::Handle<v8::Object> creationContext, v8::Isolate* isolate)
{
    return toV8(impl, creationContext, isolate);
}

} // namespace WebCore
#endif // ENABLE(WEB_AUDIO)
