/*
 * Copyright 2016-2017 Nikolay Aleksiev. All rights reserved.
 * License: https://github.com/naleksiev/mtlpp/blob/master/LICENSE
 */
// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
// Modifications for Unreal Engine

#pragma once

#include "defines.hpp"
#include "ns.hpp"

MTLPP_PROTOCOL(MTLCommandQueue);

namespace mtlpp
{
    class Device;
    class CommandBuffer;

    class CommandQueue : public ns::Object<ns::Protocol<id<MTLCommandQueue>>::type>
    {
    public:
        CommandQueue() { }
        CommandQueue(ns::Protocol<id<MTLCommandQueue>>::type handle) : ns::Object<ns::Protocol<id<MTLCommandQueue>>::type>(handle) { }

        ns::String GetLabel() const;
        Device     GetDevice() const;

        void SetLabel(const ns::String& label);

        class CommandBuffer CommandBufferWithUnretainedReferences();
        class CommandBuffer CommandBuffer();
        void                InsertDebugCaptureBoundary() MTLPP_DEPRECATED(10_11, 10_13, 8_0, 11_0);
    }
    MTLPP_AVAILABLE(10_11, 8_0);
}
