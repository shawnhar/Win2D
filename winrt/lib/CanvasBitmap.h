// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use these files except in compliance with the License. You may obtain
// a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.

#pragma once

#include "CanvasImage.h"
#include "PolymorphicBitmapmanager.h"

namespace ABI { namespace Microsoft { namespace Graphics { namespace Canvas
{
    using namespace ::Microsoft::WRL;
    using namespace ABI::Microsoft::Graphics::Canvas::Effects;

    class CanvasBitmapManager;

    class ICanvasBitmapResourceCreationAdapter
    {
    public:
        virtual ComPtr<IWICFormatConverter> CreateWICFormatConverter(HSTRING fileName) = 0;
    };
    

    [uuid(4684FA78-C721-4531-8CCE-BEA927F95E5D)]
    class ICanvasBitmapInternal : public IUnknown
    {
    public:
        virtual ComPtr<ID2D1Bitmap1> GetD2DBitmap() = 0;
    };


    class CanvasBitmapFactory :
        public ActivationFactory<
            ICanvasBitmapFactory, 
            ICanvasBitmapStatics,
            CloakedIid<ICanvasDeviceResourceFactoryNative>>,
        public PerApplicationPolymorphicBitmapManager
    {
        InspectableClassStatic(RuntimeClass_Microsoft_Graphics_Canvas_CanvasBitmap, BaseTrust);

    public:
        CanvasBitmapFactory();

        //
        // ICanvasBitmapStatics
        //

        IFACEMETHOD(LoadAsync)(
            ICanvasResourceCreator* resourceCreator,
            HSTRING fileName,
            ABI::Windows::Foundation::IAsyncOperation<CanvasBitmap*>** canvasBitmap) override;

        //
        // ICanvasDeviceResourceFactoryNative
        //
        
        IFACEMETHOD(GetOrCreate)(
            ICanvasDevice* device,
            IUnknown* resource,
            IInspectable** wrapper) override;
    };


    struct CanvasBitmapTraits
    {
        typedef ID2D1Bitmap1 resource_t;
        typedef CanvasBitmap wrapper_t;
        typedef ICanvasBitmap wrapper_interface_t;
        typedef CanvasBitmapManager manager_t;
    };


    template<typename TRAITS>
    class CanvasBitmapImpl 
        : public Implements<
        RuntimeClassFlags<WinRtClassicComMix>,
        ICanvasBitmap,
        ICanvasImage,
        IEffectInput,
        CloakedIid<ICanvasImageInternal>,
        CloakedIid<ICanvasBitmapInternal>,
        ChainInterfaces<MixIn<CanvasBitmapImpl<TRAITS>, ResourceWrapper<TRAITS>>, ABI::Windows::Foundation::IClosable, CloakedIid<ICanvasResourceWrapperNative>>>
        , public ResourceWrapper<TRAITS>
    {
    protected:
        CanvasBitmapImpl(std::shared_ptr<typename TRAITS::manager_t> manager, ID2D1Bitmap1* resource)
            : ResourceWrapper(manager, resource)
        {}

    public:
        IFACEMETHODIMP get_SizeInPixels(_Out_ ABI::Windows::Foundation::Size* size) override
        {
            return ExceptionBoundary(
                [&]
                {
                    CheckInPointer(size);
                    
                    auto& resource = GetResource();
                    D2D1_SIZE_U d2dSize = resource->GetPixelSize();
                    size->Height = static_cast<float>(d2dSize.height);
                    size->Width = static_cast<float>(d2dSize.width);
                });
        }
        
        IFACEMETHODIMP get_Size(_Out_ ABI::Windows::Foundation::Size* size) override
        {
            return ExceptionBoundary(
                [&]
                {
                    CheckInPointer(size);

                    auto& resource = GetResource();
                    D2D1_SIZE_F d2dSize = resource->GetSize();
                    size->Height = d2dSize.height;
                    size->Width = d2dSize.width;
                });
        }

        IFACEMETHODIMP get_Bounds(_Out_ ABI::Windows::Foundation::Rect* bounds) override
        {
            return ExceptionBoundary(
                [&]
                {
                    CheckInPointer(bounds);

                    auto& resource = GetResource();
                    D2D1_SIZE_F d2dSize = resource->GetSize();
                    bounds->X = 0;
                    bounds->Y = 0;
                    bounds->Width = d2dSize.width;
                    bounds->Height = d2dSize.height;
                });
        }

        // ICanvasImageInternal
        virtual ComPtr<ID2D1Image> GetD2DImage(ID2D1DeviceContext* deviceContext, uint64_t* realizationId) override
        {
            CheckInPointer(deviceContext);

            if (realizationId)
                *realizationId = 0;

            return GetResource();
        }

        // ICanvasBitmapInternal
        virtual ComPtr<ID2D1Bitmap1> GetD2DBitmap() override
        {
            return GetResource();
        }
    };


    class CanvasBitmap :
        public RuntimeClass<                                    
            RuntimeClassFlags<WinRtClassicComMix>,              
            MixIn<CanvasBitmap, CanvasBitmapImpl<CanvasBitmapTraits>>>
        , public CanvasBitmapImpl<CanvasBitmapTraits>
    {
        InspectableClass(RuntimeClass_Microsoft_Graphics_Canvas_CanvasBitmap, BaseTrust);

    public:
        CanvasBitmap(
            std::shared_ptr<CanvasBitmapManager> manager,
            ID2D1Bitmap1* bitmap);
    };


    class CanvasBitmapManager : public ResourceManager<CanvasBitmapTraits>
    {
        std::shared_ptr<ICanvasBitmapResourceCreationAdapter> m_adapter;

    public:
        CanvasBitmapManager(std::shared_ptr<ICanvasBitmapResourceCreationAdapter> adapter);

        ComPtr<CanvasBitmap> CreateNew(
            ICanvasDevice* canvasDevice, 
            HSTRING fileName);

        ComPtr<CanvasBitmap> CreateWrapper(
            ID2D1Bitmap1* bitmap);
    };
}}}}
