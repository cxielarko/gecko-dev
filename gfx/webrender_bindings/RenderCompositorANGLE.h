/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_GFX_RENDERCOMPOSITOR_ANGLE_H
#define MOZILLA_GFX_RENDERCOMPOSITOR_ANGLE_H

#include "mozilla/Maybe.h"
#include "mozilla/webrender/RenderCompositor.h"

struct ID3D11DeviceContext;
struct ID3D11Device;
struct ID3D11Query;
struct IDXGISwapChain;

namespace mozilla {

namespace wr {

class RenderCompositorANGLE : public RenderCompositor
{
public:
  static UniquePtr<RenderCompositor> Create(RefPtr<widget::CompositorWidget>&& aWidget);

  explicit RenderCompositorANGLE(RefPtr<widget::CompositorWidget>&& aWidget);
  virtual ~RenderCompositorANGLE();
  bool Initialize();

  bool BeginFrame() override;
  void EndFrame() override;
  void Pause() override;
  bool Resume() override;

  gl::GLContext* gl() const override { return mGL; }

  bool UseANGLE() const override { return true; }

  LayoutDeviceIntSize GetBufferSize() override;

protected:
  void InsertPresentWaitQuery();
  void WaitForPreviousPresentQuery();
  bool ResizeBufferIfNeeded();
  void DestroyEGLSurface();

  RefPtr<gl::GLContext> mGL;
  EGLConfig mEGLConfig;
  EGLSurface mEGLSurface;

  RefPtr<ID3D11Device> mDevice;
  RefPtr<ID3D11DeviceContext> mCtx;
  RefPtr<IDXGISwapChain> mSwapChain;

  RefPtr<ID3D11Query> mWaitForPresentQuery;
  RefPtr<ID3D11Query> mNextWaitForPresentQuery;

  Maybe<LayoutDeviceIntSize> mBufferSize;
};

} // namespace wr
} // namespace mozilla

#endif
