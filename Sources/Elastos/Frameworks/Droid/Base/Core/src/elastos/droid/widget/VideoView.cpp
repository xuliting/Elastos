//=========================================================================
// Copyright (C) 2012 The Elastos Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//=========================================================================

#include "elastos/droid/widget/VideoView.h"
#include "elastos/droid/app/CAlertDialogBuilder.h"
#include "elastos/droid/content/CIntent.h"
#include "Elastos.CoreLibrary.IO.h"
#include "elastos/droid/media/CMediaPlayer.h"
#include "elastos/droid/media/CSubtitleController.h"
#include "elastos/droid/media/CWebVttRenderer.h"
#include "elastos/droid/media/CTtmlRenderer.h"
#include "elastos/droid/media/CClosedCaptionRenderer.h"
#include "elastos/droid/net/Uri.h"
#include "elastos/droid/os/Looper.h"
#include "elastos/droid/R.h"
#include "elastos/droid/utility/CPair.h"
#include <elastos/core/Math.h>
#include <elastos/utility/logging/Logger.h>

using Elastos::Droid::View::Accessibility::IAccessibilityRecord;
using Elastos::Droid::App::IAlertDialog;
using Elastos::Droid::App::IAlertDialogBuilder;
using Elastos::Droid::App::CAlertDialogBuilder;
using Elastos::Droid::Content::IIntent;
using Elastos::Droid::Content::CIntent;
using Elastos::Droid::Content::EIID_IDialogInterfaceOnClickListener;
using Elastos::Droid::Media::IMetadata;
using Elastos::Droid::Media::CMediaPlayer;
using Elastos::Droid::Media::CWebVttRenderer;
using Elastos::Droid::Media::CTtmlRenderer;
using Elastos::Droid::Media::CSubtitleController;
using Elastos::Droid::Media::CClosedCaptionRenderer;
using Elastos::Droid::Media::IAudioManager;
using Elastos::Droid::Media::EIID_IMediaPlayerOnPreparedListener;
using Elastos::Droid::Media::EIID_IMediaPlayerOnCompletionListener;
using Elastos::Droid::Media::EIID_IMediaPlayerOnErrorListener;
using Elastos::Droid::Media::EIID_IMediaPlayerOnInfoListener;
using Elastos::Droid::Media::EIID_IMediaPlayerOnVideoSizeChangedListener;
using Elastos::Droid::Media::EIID_IMediaPlayerOnBufferingUpdateListener;
using Elastos::Droid::Media::EIID_ISubtitleControllerAnchor;
using Elastos::Droid::Media::EIID_ISubtitleTrackRenderingWidgetOnChangedListener;
using Elastos::Droid::Media::IMediaTimeProvider;
using Elastos::Droid::Media::ISubtitleController;
using Elastos::Droid::Media::ISubtitleControllerListener;
using Elastos::Droid::Media::ISubtitleControllerRenderer;
using Elastos::Droid::Net::Uri;
using Elastos::Droid::Os::Looper;
using Elastos::Droid::R;
using Elastos::Droid::Utility::CPair;
using Elastos::Droid::Utility::IPair;
using Elastos::Droid::View::EIID_IView;
using Elastos::Droid::View::EIID_ISurfaceHolderCallback;
using Elastos::Droid::View::IViewParent;
using Elastos::Droid::View::IKeyEvent;
using Elastos::Droid::View::IViewOnLayoutChangeListener;
using Elastos::Droid::View::EIID_IViewOnLayoutChangeListener;
using Elastos::Core::CString;
using Elastos::Utility::CVector;
using Elastos::Utility::Logging::Logger;

namespace Elastos {
namespace Droid {
namespace Widget {

//==============================================================================
//          VideoView::VVOnVideoSizeChangedListener
//==============================================================================
CAR_INTERFACE_IMPL(VideoView::VVOnVideoSizeChangedListener, Object, IMediaPlayerOnVideoSizeChangedListener);
VideoView::VVOnVideoSizeChangedListener::VVOnVideoSizeChangedListener(
    /* [in] */ VideoView* host)
    : mHost(host)
{}

ECode VideoView::VVOnVideoSizeChangedListener::OnVideoSizeChanged(
    /* [in] */ IMediaPlayer* mp,
    /* [in] */ Int32 width,
    /* [in] */ Int32 height)
{
    mp->GetVideoWidth(&mHost->mVideoWidth);
    mp->GetVideoHeight(&mHost->mVideoHeight);
    if (mHost->mVideoWidth != 0 &&mHost-> mVideoHeight != 0) {
        AutoPtr<ISurfaceHolder> holder;
        mHost->GetHolder((ISurfaceHolder**)&holder);
        holder->SetFixedSize(
                    mHost->mVideoWidth, mHost->mVideoHeight);

        mHost->RequestLayout();
    }

    return NOERROR;
}

//==============================================================================
//          VideoView::VVOnPreparedListener
//==============================================================================
CAR_INTERFACE_IMPL(VideoView::VVOnPreparedListener, Object, IMediaPlayerOnPreparedListener);
VideoView::VVOnPreparedListener::VVOnPreparedListener(
    /* [in] */ VideoView* host)
    : mHost(host)
{}

ECode VideoView::VVOnPreparedListener::OnPrepared(
    /* [in] */ IMediaPlayer* mp)
{
    mHost->mCurrentState = STATE_PREPARED;

    // Get the capabilities of the player for this stream
    AutoPtr<IMetadata> data;
    mp->GetMetadata(IMediaPlayer::METADATA_ALL,
                    IMediaPlayer::BYPASS_METADATA_FILTER, (IMetadata**)&data);

    if (data != NULL) {
        Boolean hasRes, boolRes;
        mHost->mCanPause = (data->Has(IMetadata::PAUSE_AVAILABLE, &hasRes), !hasRes)
                || (data->GetBoolean(IMetadata::PAUSE_AVAILABLE, &boolRes), boolRes);
        mHost->mCanSeekBack = (data->Has(IMetadata::SEEK_BACKWARD_AVAILABLE, &hasRes), !hasRes)
                || (data->GetBoolean(IMetadata::SEEK_BACKWARD_AVAILABLE, &boolRes), boolRes);
        mHost->mCanSeekForward = (data->Has(IMetadata::SEEK_FORWARD_AVAILABLE, &hasRes), !hasRes)
                || (data->GetBoolean(IMetadata::SEEK_FORWARD_AVAILABLE, &boolRes), boolRes);
        data->RecycleParcel();
    }
    else {
        mHost->mCanPause = mHost->mCanSeekBack = mHost->mCanSeekForward = TRUE;
    }

    if (mHost->mOnPreparedListener != NULL) {
        mHost->mOnPreparedListener->OnPrepared(mHost->mMediaPlayer);
    }
    if (mHost->mMediaController != NULL) {
        IView::Probe(mHost->mMediaController)->SetEnabled(TRUE);
    }
    mp->GetVideoWidth(&mHost->mVideoWidth);
    mp->GetVideoHeight(&mHost->mVideoHeight);

    Int32 seekToPosition = mHost->mSeekWhenPrepared;  // mSeekWhenPrepared may be changed after seekTo() call
    if (seekToPosition != 0) {
        mHost->SeekTo(seekToPosition);
    }
    if (mHost->mVideoWidth != 0 && mHost->mVideoHeight != 0) {
        AutoPtr<ISurfaceHolder> holder;
        mHost->GetHolder((ISurfaceHolder**)&holder);
        //Log.i("@@@@", "video size: " + mVideoWidth +"/"+ mVideoHeight);
        holder->SetFixedSize(mHost->mVideoWidth, mHost->mVideoHeight);
        if (mHost->mSurfaceWidth == mHost->mVideoWidth &&
            mHost->mSurfaceHeight == mHost->mVideoHeight) {
            // We didn't actually change the size (it was already at the size
            // we need), so we won't get a "surface changed" callback, so
            // start the video here instead of in the callback.
            Boolean playing = FALSE;
            Int32 pos = 0;
            if (mHost->mTargetState == STATE_PLAYING) {
                mHost->Start();
                if (mHost->mMediaController != NULL) {
                    mHost->mMediaController->Show();
                }
            }
            else if ((mHost->IsPlaying(&playing), !playing) &&
                    (seekToPosition != 0 || (mHost->GetCurrentPosition(&pos), pos) > 0)) {
                if (mHost->mMediaController != NULL) {
                    // Show the media controls when we're paused into a video and make 'em stick.
                    mHost->mMediaController->Show(0);
                }
            }
        }
    }
    else {
        // We don't know the video size yet, but should start anyway.
        // The video size might be reported to us later.
        if (mHost->mTargetState == STATE_PLAYING) {
            mHost->Start();
        }
    }

    return NOERROR;
}

//==============================================================================
//          VideoView::VVOnCompletionListener
//==============================================================================
CAR_INTERFACE_IMPL(VideoView::VVOnCompletionListener, Object, IMediaPlayerOnCompletionListener);
VideoView::VVOnCompletionListener::VVOnCompletionListener(
    /* [in] */ VideoView* host)
    : mHost(host)
{}

ECode VideoView::VVOnCompletionListener::OnCompletion(
    /* [in] */ IMediaPlayer* mp)
{
    mHost->mCurrentState = STATE_PLAYBACK_COMPLETED;
    mHost->mTargetState = STATE_PLAYBACK_COMPLETED;
    if (mHost->mMediaController != NULL) {
        mHost->mMediaController->Hide();
    }
    if (mHost->mOnCompletionListener != NULL) {
        mHost->mOnCompletionListener->OnCompletion(mHost->mMediaPlayer);
    }

    return NOERROR;
}

//==============================================================================
//          VideoView::VVOnErrorListener
//==============================================================================
CAR_INTERFACE_IMPL(VideoView::VVOnErrorListener, Object, IMediaPlayerOnErrorListener)
VideoView::VVOnErrorListener::VVOnErrorListener(
    /* [in] */ VideoView* host)
    : mHost(host)
{}

ECode VideoView::VVOnErrorListener::OnError(
    /* [in] */ IMediaPlayer* mp,
    /* [in] */ Int32 what,
    /* [in] */ Int32 extra,
    /* [out] */ Boolean* res)
{
    // Log.d(TAG, "Error: " + framework_err + "," + impl_err);
    mHost->mCurrentState = STATE_ERROR;
    mHost->mTargetState = STATE_ERROR;
    if (mHost->mMediaController != NULL) {
        mHost->mMediaController->Hide();
    }

    /* If an error handler has been supplied, use it and finish. */
    if (mHost->mOnErrorListener != NULL) {
        Boolean result;
        mHost->mOnErrorListener->OnError(mHost->mMediaPlayer, what, extra, &result);
        if (result) {
            *res = TRUE;
            return NOERROR;
        }
    }

    /* Otherwise, pop up an error dialog so the user knows that
     * something bad has happened. Only try and pop up the dialog
     * if we're attached to a window. When we're going away and no
     * longer have a window, don't bother showing the user an error.
     */
    AutoPtr<IBinder> token;
    if ((mHost->GetWindowToken((IBinder**)&token), token.Get()) != NULL) {
        AutoPtr<IResources> r;
        FAIL_RETURN(mHost->mContext->GetResources((IResources**)&r));
        Int32 messageId;

        if (what == IMediaPlayer::MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK) {
            messageId = R::string::VideoView_error_text_invalid_progressive_playback;
        }
        else {
            messageId = R::string::VideoView_error_text_unknown;
        }


        AutoPtr<IAlertDialogBuilder> builder;
        FAIL_RETURN(CAlertDialogBuilder::New(
                mHost->mContext, (IAlertDialogBuilder**)&builder));
        //builder->SetTitle(R::string::VideoView_error_title);
        builder->SetMessage(messageId);
        builder->SetCancelable(FALSE);

        AutoPtr<VVDialogOnClickListener> listener = new VVDialogOnClickListener(mHost);
        builder->SetPositiveButton(R::string::VideoView_error_button, listener);

        AutoPtr<IAlertDialog> dialog;
        builder->Show((IAlertDialog**)&dialog);
    }

    *res = TRUE;
    return NOERROR;
}

//==============================================================================
//          VideoView::VVDialogOnClickListener
//==============================================================================
CAR_INTERFACE_IMPL(VideoView::VVDialogOnClickListener, Object, IDialogInterfaceOnClickListener);
VideoView::VVDialogOnClickListener::VVDialogOnClickListener(
    /* [in] */ VideoView* host)
    : mHost(host)
{}

ECode VideoView::VVDialogOnClickListener::OnClick(
    /* [in] */ IDialogInterface* dialog,
    /* [in] */ Int32 which)
{
    /* If we get here, there is no onError listener, so
     * at least inform them that the video is over.
     */
    if (mHost->mOnCompletionListener != NULL) {
        mHost->mOnCompletionListener->OnCompletion(mHost->mMediaPlayer);
    }

    return NOERROR;
}

//==============================================================================
//          VideoView::VVOnBufferingUpdateListener
//==============================================================================
CAR_INTERFACE_IMPL(VideoView::VVOnBufferingUpdateListener, Object, IMediaPlayerOnBufferingUpdateListener);
VideoView::VVOnBufferingUpdateListener::VVOnBufferingUpdateListener(
    /* [in] */ VideoView* host)
    : mHost(host)
{}

ECode VideoView::VVOnBufferingUpdateListener::OnBufferingUpdate(
    /* [in] */ IMediaPlayer* mp,
    /* [in] */ Int32 percent)
{
    mHost->mCurrentBufferPercentage = percent;

    return NOERROR;
}

//==============================================================================
//          VideoView::VVSurfaceHodlerCallback
//==============================================================================
CAR_INTERFACE_IMPL(VideoView::VVSurfaceHodlerCallback, Object, ISurfaceHolderCallback);
VideoView::VVSurfaceHodlerCallback::VVSurfaceHodlerCallback(
    /* [in] */ VideoView* host)
    : mHost(host)
{}

ECode VideoView::VVSurfaceHodlerCallback::SurfaceChanged(
    /* [in] */ ISurfaceHolder* holder,
    /* [in] */ Int32 format,
    /* [in] */ Int32 width,
    /* [in] */ Int32 height)
{
    mHost->mSurfaceWidth = width;
    mHost->mSurfaceHeight = height;
    Boolean isValidState =  (mHost->mTargetState == STATE_PLAYING);
    Boolean hasValidSize = (mHost->mVideoWidth == width &&
                            mHost->mVideoHeight == height);
    if (mHost->mMediaPlayer != NULL && isValidState && hasValidSize) {
        if (mHost->mSeekWhenPrepared != 0) {
            mHost->SeekTo(mHost->mSeekWhenPrepared);
        }

        mHost->Start();
    }

    return NOERROR;
}

ECode VideoView::VVSurfaceHodlerCallback::SurfaceCreated(
    /* [in] */ ISurfaceHolder* holder)
{
    mHost->mSurfaceHolder = holder;
    //resume() was called before surfaceCreated()
            // if current state is suspended, call resume() to init the decoders again
    if (mHost->mCurrentState == STATE_SUSPENDED && mHost->mMediaPlayer != NULL) {
        mHost->mMediaPlayer->SetDisplay(mHost->mSurfaceHolder);
        Boolean resume;
        if (mHost->mMediaPlayer->Resume(&resume), resume) {
            mHost->mCurrentState = STATE_PREPARED;
            if (mHost->mSeekWhenPrepared != 0) {
                // seek if necessary
                mHost->SeekTo(mHost->mSeekWhenPrepared);
            }
            if (mHost->mTargetState == STATE_PLAYING) {
                mHost->Start();
            }
            return NOERROR;
        } else {
            mHost->ReleaseResources(FALSE);
        }
    }
    mHost->OpenVideo();

    return NOERROR;
}

ECode VideoView::VVSurfaceHodlerCallback::SurfaceDestroyed(
    /* [in] */ ISurfaceHolder* holder)
{
    // after we return from this we can't use the surface any more
    mHost->mSurfaceHolder = NULL;
    if (mHost->mMediaController != NULL) {
        mHost->mMediaController->Hide();
    }
    if (mHost->IsHTTPStreaming(mHost->mUri) && mHost->mCurrentState == STATE_SUSPENDED) {
            // don't call release() while suspending
        return NOERROR;
    }
    mHost->ReleaseResources(TRUE);

    return NOERROR;
}

//==============================================================================
//          VideoView::InfoListener
//==============================================================================
CAR_INTERFACE_IMPL(VideoView::InfoListener, Object, IMediaPlayerOnInfoListener)

VideoView::InfoListener::InfoListener(
    /* [in] */ VideoView* host)
    : mHost(host)
{}

ECode VideoView::InfoListener::OnInfo(
    /* [in] */ IMediaPlayer* mp,
    /* [in] */ Int32 arg1,
    /* [in] */ Int32 arg2,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    if (mHost->mOnInfoListener != NULL) {
        mHost->mOnInfoListener->OnInfo(mp, arg1, arg2, result);
    }
    *result = TRUE;
    return NOERROR;
}

//==============================================================================
//          VideoView::ChangedListener
//==============================================================================
CAR_INTERFACE_IMPL(VideoView::ChangedListener, Object, ISubtitleTrackRenderingWidgetOnChangedListener);
VideoView::ChangedListener::ChangedListener(
    /* [in] */ VideoView* host)
    : mHost(host)
{}

ECode VideoView::ChangedListener::OnChanged(
    /* [in] */ ISubtitleTrackRenderingWidget* renderingWidget)
{
    mHost->Invalidate();
    return NOERROR;
}


//==============================================================================
//          VideoView
//==============================================================================
const String VideoView::TAG("VideoView");
const Int32 VideoView::STATE_ERROR = -1;
const Int32 VideoView::STATE_IDLE = 0;
const Int32 VideoView::STATE_PREPARING = 1;
const Int32 VideoView::STATE_PREPARED = 2;
const Int32 VideoView::STATE_PLAYING = 3;
const Int32 VideoView::STATE_PAUSED = 4;
const Int32 VideoView::STATE_PLAYBACK_COMPLETED = 5;
const Int32 VideoView::STATE_SUSPENDED = 6;


CAR_INTERFACE_IMPL(VideoView, SurfaceView, IVideoView, IMediaPlayerControl, ISubtitleControllerAnchor);
VideoView::VideoView()
    : mCurrentState(STATE_IDLE)
    , mTargetState(STATE_IDLE)
    , mAudioSession(0)
    , mVideoWidth(0)
    , mVideoHeight(0)
    , mSurfaceWidth(0)
    , mSurfaceHeight(0)
    , mCurrentBufferPercentage(0)
    , mSeekWhenPrepared(0)
    , mCanPause(FALSE)
    , mCanSeekBack(FALSE)
    , mCanSeekForward(FALSE)
{
    mSizeChangedListener = new VVOnVideoSizeChangedListener(this);
    mPreparedListener = new VVOnPreparedListener(this);
    mCompletionListener = new VVOnCompletionListener(this);
    mErrorListener = new VVOnErrorListener(this);
    mBufferingUpdateListener = new VVOnBufferingUpdateListener(this);
    mSHCallback = new VVSurfaceHodlerCallback(this);
    mInfoListener = new InfoListener(this);
}

ECode VideoView::constructor(
    /* [in] */ IContext* context)
{
    SurfaceView::constructor(context);
    InitVideoView();
    return NOERROR;
}

ECode VideoView::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs)
{
    constructor(context, attrs, 0);
    InitVideoView();
    return NOERROR;
}

ECode VideoView::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs,
    /* [in] */ Int32 defStyleAttr)
{
    return constructor(context, attrs, defStyleAttr, 0);
}

ECode VideoView::constructor(
    /* [in] */ IContext* context,
    /* [in] */ IAttributeSet* attrs,
    /* [in] */ Int32 defStyleAttr,
    /* [in] */ Int32 defStyleRes)
{
    SurfaceView::constructor(context, attrs, defStyleAttr, defStyleRes);
    InitVideoView();
    return NOERROR;
}

VideoView::~VideoView()
{}

void VideoView::InitVideoView()
{
    AutoPtr<ISurfaceHolder> holder;
    GetHolder((ISurfaceHolder**)&holder);
    holder->AddCallback(mSHCallback);
    holder->SetType(ISurfaceHolder::SURFACE_TYPE_PUSH_BUFFERS);
    SetFocusable(TRUE);
    SetFocusableInTouchMode(TRUE);
    Boolean tmp = FALSE;
    RequestFocus(&tmp);
    CVector::New((IVector**)&mPendingSubtitleTracks);
    mCurrentState = STATE_IDLE;
    mTargetState  = STATE_IDLE;
}

ECode VideoView::OnMeasure(
    /* [in] */ Int32 widthMeasureSpec,
    /* [in] */ Int32 heightMeasureSpec)
{
    //Log.i("@@@@", "onMeasure(" + MeasureSpec.toString(widthMeasureSpec) + ", "
    //        + MeasureSpec.toString(heightMeasureSpec) + ")");

    Int32 width = GetDefaultSize(mVideoWidth, widthMeasureSpec);
    Int32 height = GetDefaultSize(mVideoHeight, heightMeasureSpec);
    if (mVideoWidth > 0 && mVideoHeight > 0) {
        Int32 widthSpecMode = View::MeasureSpec::GetMode(widthMeasureSpec);
        Int32 widthSpecSize = View::MeasureSpec::GetSize(widthMeasureSpec);
        Int32 heightSpecMode = View::MeasureSpec::GetMode(heightMeasureSpec);
        Int32 heightSpecSize = View::MeasureSpec::GetSize(heightMeasureSpec);

        if (widthSpecMode == View::MeasureSpec::EXACTLY && heightSpecMode == View::MeasureSpec::EXACTLY) {
            // the size is fixed
            width = widthSpecSize;
            height = heightSpecSize;

            // for compatibility, we adjust size based on aspect ratio
            if ( mVideoWidth * height  < width * mVideoHeight ) {
                //Log.i("@@@", "image too wide, correcting");
                width = height * mVideoWidth / mVideoHeight;
            }
            else if ( mVideoWidth * height  > width * mVideoHeight ) {
                //Log.i("@@@", "image too tall, correcting");
                height = width * mVideoHeight / mVideoWidth;
            }
        }
        else if (widthSpecMode == View::MeasureSpec::EXACTLY) {
            // only the width is fixed, adjust the height to match aspect ratio if possible
            width = widthSpecSize;
            height = width * mVideoHeight / mVideoWidth;
            if (heightSpecMode == View::MeasureSpec::AT_MOST && height > heightSpecSize) {
                // couldn't match aspect ratio within the constraints
                height = heightSpecSize;
            }
        }
        else if (heightSpecMode == View::MeasureSpec::EXACTLY) {
            // only the height is fixed, adjust the width to match aspect ratio if possible
            height = heightSpecSize;
            width = height * mVideoWidth / mVideoHeight;
            if (widthSpecMode == View::MeasureSpec::AT_MOST && width > widthSpecSize) {
                // couldn't match aspect ratio within the constraints
                width = widthSpecSize;
            }
        }
        else {
            // neither the width nor the height are fixed, try to use actual video size
            width = mVideoWidth;
            height = mVideoHeight;
            if (heightSpecMode == View::MeasureSpec::AT_MOST && height > heightSpecSize) {
                // too tall, decrease both width and height
                height = heightSpecSize;
                width = height * mVideoWidth / mVideoHeight;
            }
            if (widthSpecMode == View::MeasureSpec::AT_MOST && width > widthSpecSize) {
                // too wide, decrease both width and height
                width = widthSpecSize;
                height = width * mVideoHeight / mVideoWidth;
            }
        }
    }
    else {
        // no size yet, just adopt the given spec sizes
    }
    SetMeasuredDimension(width, height);
    return NOERROR;
}

ECode VideoView::OnInitializeAccessibilityEvent(
    /* [in] */ IAccessibilityEvent* event)
{
    SurfaceView::OnInitializeAccessibilityEvent(event);

    AutoPtr<ICharSequence> clsName;
    CString::New(String("CVideoView"), (ICharSequence**)&clsName);
    IAccessibilityRecord::Probe(event)->SetClassName(clsName);
    return NOERROR;
}

ECode VideoView::OnInitializeAccessibilityNodeInfo(
    /* [in] */ IAccessibilityNodeInfo* info)
{
    SurfaceView::OnInitializeAccessibilityNodeInfo(info);
    AutoPtr<ICharSequence> clsName;
    CString::New(String("CVideoView"), (ICharSequence**)&clsName);
    info->SetClassName(clsName);
    return NOERROR;
}

ECode VideoView::ResolveAdjustedSize(
    /* [in] */ Int32 desiredSize,
    /* [in] */ Int32 measureSpec,
    /* [out] */ Int32* size)
{
    VALIDATE_NOT_NULL(size);
    *size = GetDefaultSize(desiredSize, measureSpec);
    return NOERROR;
}

ECode VideoView::SetVideoPath(
    /* [in] */ const String& path)
{
    AutoPtr<IUri> uri;
    FAIL_RETURN(Uri::Parse(path, (IUri**)&uri));
    return SetVideoURI(uri);
}

ECode VideoView::SetVideoURI(
    /* [in] */ IUri* uri)
{
    return SetVideoURI(uri, NULL);
}

ECode VideoView::SetVideoURI(
    /* [in] */ IUri* uri,
    /* [in] */  IMap* headers)
{
    mUri = uri;
    mHeaders = headers;
    mSeekWhenPrepared = 0;
    OpenVideo();
    FAIL_RETURN(RequestLayout());
    return Invalidate();
}

ECode VideoView::AddSubtitleSource(
    /* [in] */ IInputStream* is,
    /* [in] */ IMediaFormat* format)
{
    if (mMediaPlayer == NULL) {
        AutoPtr<IPair> pair;
        CPair::Create(is, format, (IPair**)&pair);
        mPendingSubtitleTracks->Add(pair);
    }
    else {
        ECode ec = mMediaPlayer->AddSubtitleSource(is, format);
        if (ec == (ECode)E_ILLEGAL_STATE_EXCEPTION) {
            Boolean tmp = FALSE;
            mInfoListener->OnInfo(
                mMediaPlayer, IMediaPlayer::MEDIA_INFO_UNSUPPORTED_SUBTITLE, 0, &tmp);
        }
    }
    return NOERROR;
}

ECode VideoView::StopPlayback()
{
    Logger::I(TAG, "Playback Stop begin");
    if (mMediaPlayer != NULL) {
        mMediaPlayer->Stop();
        mMediaPlayer->ReleaseResources();
        mMediaPlayer = NULL;
        mCurrentState = STATE_IDLE;
        mTargetState  = STATE_IDLE;
    }
    Logger::I(TAG, "Playback Stop end");

    return NOERROR;
}

void VideoView::OpenVideo()
{
    if (mUri == NULL || mSurfaceHolder == NULL) {
        // not ready for playback just yet, will try again later
        return;
    }

    Logger::I(TAG, "Open Video");
    AutoPtr<IInterface> service;
    mContext->GetSystemService(IContext::AUDIO_SERVICE, (IInterface**)&service);
    AutoPtr<IAudioManager> am = IAudioManager::Probe(service);
    Int32 result = 0;
    am->RequestAudioFocus(NULL, IAudioManager::STREAM_MUSIC, IAudioManager::AUDIOFOCUS_GAIN, &result);

    // we shouldn't clear the target state, because somebody might have
    // called start() previously
    ReleaseResources(FALSE);
    //try {
    CMediaPlayer::New((IMediaPlayer**)&mMediaPlayer);
    // TODO: create SubtitleController in MediaPlayer, but we need
    // a context for the subtitle renderers
    AutoPtr<IContext> context;
    GetContext((IContext**)&context);
    AutoPtr<ISubtitleController> controller;
    AutoPtr<IMediaTimeProvider> provider;
    AutoPtr<ISubtitleControllerRenderer> webRenderer;
    AutoPtr<ISubtitleControllerRenderer> tmlRenderer;
    AutoPtr<ISubtitleControllerRenderer> closedRenderer;
    Boolean tmp = FALSE;
    Int32 size = 0;
    ECode ec = mMediaPlayer->GetMediaTimeProvider((IMediaTimeProvider**)&provider);
    FAIL_GOTO(ec, exit);
    CSubtitleController::New(context, provider, ISubtitleControllerListener::Probe(mMediaPlayer)
            , (ISubtitleController**)&controller);
    CWebVttRenderer::New(context, (ISubtitleControllerRenderer**)&webRenderer);
    controller->RegisterRenderer(webRenderer);
    CTtmlRenderer::New(context, (ISubtitleControllerRenderer**)&tmlRenderer);
    controller->RegisterRenderer(tmlRenderer);

    CClosedCaptionRenderer::New(context, (ISubtitleControllerRenderer**)&closedRenderer);
    controller->RegisterRenderer(closedRenderer);
    //TODO : recycle ref?
    mMediaPlayer->SetSubtitleAnchor(controller, this);

    if (mAudioSession != 0) {
        mMediaPlayer->SetAudioSessionId(mAudioSession);
    }
    else {
        mMediaPlayer->GetAudioSessionId(&mAudioSession);
    }
    mMediaPlayer->SetOnPreparedListener(mPreparedListener);
    mMediaPlayer->SetOnVideoSizeChangedListener(mSizeChangedListener);
    mMediaPlayer->SetOnCompletionListener(mCompletionListener);
    mMediaPlayer->SetOnErrorListener(mErrorListener);
    mMediaPlayer->SetOnInfoListener(mInfoListener);
    mMediaPlayer->SetOnBufferingUpdateListener(mBufferingUpdateListener);
    mCurrentBufferPercentage = 0;

    Logger::I(TAG, "SetDataSource");

    ec = mMediaPlayer->SetDataSource(mContext, mUri, mHeaders);
    FAIL_GOTO(ec, exit);
    ec = mMediaPlayer->SetDisplay(mSurfaceHolder);
    FAIL_GOTO(ec, exit);
    ec = mMediaPlayer->SetAudioStreamType(IAudioManager::STREAM_MUSIC);
    FAIL_GOTO(ec, exit);
    ec = mMediaPlayer->SetScreenOnWhilePlaying(TRUE);
    FAIL_GOTO(ec, exit);
    ec = mMediaPlayer->PrepareAsync();
    FAIL_GOTO(ec, exit);

    mPendingSubtitleTracks->GetSize(&size);
    for (Int32 i = 0; i < size; i++) {
        AutoPtr<IInterface> obj;
        mPendingSubtitleTracks->Get(i, (IInterface**)&obj);
        AutoPtr<IPair> pending = IPair::Probe(obj);

        AutoPtr<IInterface> first;
        pending->GetFirst((IInterface**)&first);
        AutoPtr<IInterface> second;
        pending->GetSecond((IInterface**)&second);
        ec = mMediaPlayer->AddSubtitleSource(IInputStream::Probe(first), IMediaFormat::Probe(second));
        if (ec == (ECode)E_ILLEGAL_STATE_EXCEPTION) {
            ec = mInfoListener->OnInfo(mMediaPlayer, IMediaPlayer::MEDIA_INFO_UNSUPPORTED_SUBTITLE, 0, &tmp);
            FAIL_GOTO(ec, exit);
        }
    }

    // we don't set the target state here either, but preserve the
    // target state that was there before.
    mCurrentState = STATE_PREPARING;
    AttachMediaController();

exit:
    if (ec == (ECode)E_IO_EXCEPTION) {
        String log;
        if (mUri != NULL) {
            IObject::Probe(mUri)->ToString(&log);
        }
        Logger::W(TAG, String("Unable to open content: ") + log);
        mCurrentState = STATE_ERROR;
        mTargetState = STATE_ERROR;
        Boolean result;
        mErrorListener->OnError(mMediaPlayer, IMediaPlayer::MEDIA_ERROR_UNKNOWN, 0, &result);
    }
    else if (ec == (ECode)E_ILLEGAL_ARGUMENT_EXCEPTION) {
        String log;
        if (mUri != NULL) {
            IObject::Probe(mUri)->ToString(&log);
        }
        Logger::W(TAG, String("Unable to open content: ") + log);
        mCurrentState = STATE_ERROR;
        mTargetState = STATE_ERROR;
        Boolean result;
        mErrorListener->OnError(mMediaPlayer, IMediaPlayer::MEDIA_ERROR_UNKNOWN, 0, &result);
    }
    mPendingSubtitleTracks->Clear();
}

ECode VideoView::SetMediaController(
    /* [in] */ IMediaController* controller)
{
    if (mMediaController != NULL) {
        mMediaController->Hide();
    }
    mMediaController = controller;
    AttachMediaController();

    return NOERROR;
}

void VideoView::AttachMediaController()
{
    if (mMediaPlayer != NULL && mMediaController != NULL) {
        mMediaController->SetMediaPlayer(this);

        AutoPtr<IViewParent> parent;
        GetParent((IViewParent**)&parent);
        IView* anchorView = IView::Probe(parent) != NULL ?
                IView::Probe(parent) : this;
        mMediaController->SetAnchorView(anchorView);
        IView::Probe(mMediaController)->SetEnabled(IsInPlaybackState());
    }
}

ECode VideoView::SetOnPreparedListener(
    /* [in] */ IMediaPlayerOnPreparedListener* l)
{
    mOnPreparedListener = l;
    return NOERROR;
}

ECode VideoView::SetOnCompletionListener(
    /* [in] */ IMediaPlayerOnCompletionListener* l)
{
    mOnCompletionListener = l;
    return NOERROR;
}

ECode VideoView::SetOnErrorListener(
    /* [in] */ IMediaPlayerOnErrorListener* l)
{
    mOnErrorListener = l;
    return NOERROR;
}

ECode VideoView::SetOnInfoListener(
    /* [in] */ IMediaPlayerOnInfoListener* l)
{
    mOnInfoListener = l;
    return NOERROR;
}

void VideoView::ReleaseResources(
    /* [in] */ Boolean cleartargetstate)
{
    if (mMediaPlayer != NULL) {
        mMediaPlayer->Reset();
        mMediaPlayer->ReleaseResources();
        mMediaPlayer = NULL;
        mPendingSubtitleTracks->Clear();
        mCurrentState = STATE_IDLE;
        if (cleartargetstate) {
            mTargetState  = STATE_IDLE;
        }
    }
}

ECode VideoView::OnTouchEvent(
    /* [in] */ IMotionEvent* event,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    if (IsInPlaybackState() && mMediaController != NULL) {
        ToggleMediaControlsVisiblity();
    }
    *result = FALSE;
    return NOERROR;
}

ECode VideoView::OnTrackballEvent(
    /* [in] */ IMotionEvent* event,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    if (IsInPlaybackState() && mMediaController != NULL) {
        ToggleMediaControlsVisiblity();
    }
    *result = FALSE;
    return NOERROR;
}

ECode VideoView::OnKeyDown(
    /* [in] */ Int32 keyCode,
    /* [in] */ IKeyEvent* event,
    /* [out] */ Boolean* result)
{
    VALIDATE_NOT_NULL(result);
    *result = TRUE;
    Boolean isKeyCodeSupported = keyCode != IKeyEvent::KEYCODE_BACK &&
                                 keyCode != IKeyEvent::KEYCODE_VOLUME_UP &&
                                 keyCode != IKeyEvent::KEYCODE_VOLUME_DOWN &&
                                 keyCode != IKeyEvent::KEYCODE_VOLUME_MUTE &&
                                 keyCode != IKeyEvent::KEYCODE_MENU &&
                                 keyCode != IKeyEvent::KEYCODE_CALL &&
                                 keyCode != IKeyEvent::KEYCODE_ENDCALL;
    if (IsInPlaybackState() && isKeyCodeSupported && mMediaController != NULL) {
        Boolean isPlaying;
        if (keyCode == IKeyEvent::KEYCODE_HEADSETHOOK ||
            keyCode == IKeyEvent::KEYCODE_MEDIA_PLAY_PAUSE) {
            if (mMediaPlayer->IsPlaying(&isPlaying), isPlaying) {
                Pause();
                mMediaController->Show();
            }
            else {
                Start();
                mMediaController->Hide();
            }
            return NOERROR;
        }
        else if (keyCode == IKeyEvent::KEYCODE_MEDIA_PLAY) {
            if (mMediaPlayer->IsPlaying(&isPlaying), !isPlaying) {
                Start();
                mMediaController->Hide();
            }
            return NOERROR;
        }
        else if (keyCode == IKeyEvent::KEYCODE_MEDIA_STOP|| keyCode == IKeyEvent::KEYCODE_MEDIA_PAUSE) {
            if (mMediaPlayer->IsPlaying(&isPlaying), isPlaying) {
                Pause();
                mMediaController->Show();
            }

            return NOERROR;
        }
        else {
            ToggleMediaControlsVisiblity();
        }
    }

    return SurfaceView::OnKeyDown(keyCode, event, result);
}

void VideoView::ToggleMediaControlsVisiblity()
{
    Boolean isShowing;
    mMediaController->IsShowing(&isShowing);
    if (isShowing) {
        mMediaController->Hide();
    }
    else {
        mMediaController->Show();
    }
}

ECode VideoView::Start()
{
    if (IsInPlaybackState()) {
        mMediaPlayer->Start();
        mCurrentState = STATE_PLAYING;
    }
    mTargetState = STATE_PLAYING;

    return NOERROR;
}

ECode VideoView::Pause()
{
    if (IsInPlaybackState()) {
        Boolean isPlaying;
        if (mMediaPlayer->IsPlaying(&isPlaying), isPlaying) {
            mMediaPlayer->Pause();
            mCurrentState = STATE_PAUSED;
        }
    }
    mTargetState = STATE_PAUSED;

    return NOERROR;
}


ECode VideoView::Suspend()
{
    Logger::I(TAG, "Playback Suspend begin");
    // HTTP streaming will call MediaPlayer::suspend() for suspend operation, others will call release()
    if (IsHTTPStreaming(mUri) && mCurrentState > STATE_PREPARING &&
            mCurrentState < STATE_PLAYBACK_COMPLETED && mMediaPlayer != NULL) {
        Boolean suspend;
        if (mMediaPlayer->Suspend(&suspend), suspend) {
            mTargetState = mCurrentState;
            mCurrentState = STATE_SUSPENDED;
            return NOERROR;
        }
    }
    ReleaseResources(FALSE);
    Logger::I(TAG, "Playback Suspend end");
    return NOERROR;
}

ECode VideoView::Resume()
{
    // HTTP streaming (with suspended status) will call MediaPlayer::resume(), others will call openVideo()
    if (mCurrentState == STATE_SUSPENDED && mMediaPlayer != NULL) {
        if (mSurfaceHolder != NULL) {
            Boolean resume;
            if (mMediaPlayer->Resume(&resume), resume) {
                mCurrentState = STATE_PREPARED;
                if (mSeekWhenPrepared != 0) {
                    // seek if necessary
                    SeekTo(mSeekWhenPrepared);
                }
                if (mTargetState == STATE_PLAYING) {
                    Start();
                }
                return NOERROR;
            } else {
                // resume failed, so call release() before openVideo()
                ReleaseResources(FALSE);
            }
        } else {
            // the surface has been destroyed, resume() will be called after surface created
            return NOERROR;
        }
    }
    OpenVideo();
    return NOERROR;
}

ECode VideoView::GetDuration(
    /* [out] */ Int32* duration)
{
    VALIDATE_NOT_NULL(duration);
    if (IsInPlaybackState()) {
        return mMediaPlayer->GetDuration(duration);
    }

    *duration = -1;
    return NOERROR;
}

ECode VideoView::GetCurrentPosition(
    /* [out] */ Int32* pos)
{
    VALIDATE_NOT_NULL(pos);
    if (IsInPlaybackState()) {
        return mMediaPlayer->GetCurrentPosition(pos);
    }
    *pos = 0;
    return NOERROR;
}

ECode VideoView::SeekTo(
    /* [in] */ Int32 msec)
{
    if (IsInPlaybackState()) {
        mMediaPlayer->SeekTo(msec);
        mSeekWhenPrepared = 0;
    }
    else {
        mSeekWhenPrepared = msec;
    }

    return NOERROR;
}

ECode VideoView::IsPlaying(
    /* [out] */ Boolean* isPlaying)
{
    VALIDATE_NOT_NULL(isPlaying);
    Boolean tmp = FALSE;
    *isPlaying = IsInPlaybackState() && (mMediaPlayer->IsPlaying(&tmp), tmp);
    return NOERROR;
}

ECode VideoView::GetBufferPercentage(
    /* [out] */ Int32* percentage)
{
    VALIDATE_NOT_NULL(percentage);
    if (mMediaPlayer != NULL) {
        *percentage = mCurrentBufferPercentage;
        return NOERROR;
    }
    *percentage = 0;
    return NOERROR;
}

Boolean VideoView::IsInPlaybackState()
{
    return (mMediaPlayer != NULL &&
            mCurrentState != STATE_ERROR &&
            mCurrentState != STATE_IDLE &&
            mCurrentState != STATE_PREPARING &&
            mCurrentState != STATE_SUSPENDED);
}

ECode VideoView::CanPause(
    /* [out] */ Boolean* can)
{
    VALIDATE_NOT_NULL(can);
    *can = mCanPause;
    return NOERROR;
}

ECode VideoView::CanSeekBackward(
    /* [out] */ Boolean* can)
{
    VALIDATE_NOT_NULL(can);
    *can = mCanSeekBack;
    return NOERROR;
}

ECode VideoView::CanSeekForward(
    /* [out] */ Boolean* can)
{
    VALIDATE_NOT_NULL(can);
    *can = mCanSeekForward;
    return NOERROR;
}

ECode VideoView::GetAudioSessionId(
    /* [out] */ Int32* id)
{
    VALIDATE_NOT_NULL(id);
    if (mAudioSession == 0) {
        AutoPtr<IMediaPlayer> foo;
        CMediaPlayer::New((IMediaPlayer**)&foo);
        foo->GetAudioSessionId(&mAudioSession);
        foo->ReleaseResources();
    }
    *id = mAudioSession;
    return NOERROR;
}

ECode VideoView::OnAttachedToWindow()
{
    SurfaceView::OnAttachedToWindow();

    if (mSubtitleWidget != NULL) {
        mSubtitleWidget->OnAttachedToWindow();
    }
    return NOERROR;
}

ECode VideoView::OnDetachedFromWindow()
{
    SurfaceView::OnDetachedFromWindow();

    if (mSubtitleWidget != NULL) {
        mSubtitleWidget->OnDetachedFromWindow();
    }
    return NOERROR;
}

ECode VideoView::OnLayout(
    /* [in] */ Boolean changed,
    /* [in] */ Int32 left,
    /* [in] */ Int32 top,
    /* [in] */ Int32 right,
    /* [in] */ Int32 bottom)
{
    SurfaceView::OnLayout(changed, left, top, right, bottom);

    if (mSubtitleWidget != NULL) {
        MeasureAndLayoutSubtitleWidget();
    }
    return NOERROR;
}

ECode VideoView::Draw(
    /* [in] */ ICanvas* canvas)
{
    SurfaceView::Draw(canvas);

    if (mSubtitleWidget != NULL) {
        Int32 saveCount = 0;
        canvas->Save(&saveCount);
        Int32 left = 0, top = 0;
        GetPaddingLeft(&left);
        GetPaddingTop(&top);
        canvas->Translate(left, top);
        mSubtitleWidget->Draw(canvas);
        canvas->RestoreToCount(saveCount);
    }
    return NOERROR;
}

void VideoView::MeasureAndLayoutSubtitleWidget()
{
    Int32 v1 = 0, v2 = 0, v3 = 0;
    GetWidth(&v1);
    GetPaddingLeft(&v2);
    GetPaddingRight(&v3);
    const Int32 width = v1 - v2 - v3;
    GetHeight(&v1);
    GetPaddingTop(&v2);
    GetPaddingBottom(&v3);
    const Int32 height = v1 - v2 - v3;

    mSubtitleWidget->SetSize(width, height);
}

Boolean VideoView::IsHTTPStreaming(
    /* [in] */ IUri* uri)
{
    if (uri != NULL) {
        String scheme;
        uri->GetScheme(&scheme);
        if ((scheme != NULL) && (scheme.Equals("http") || scheme.Equals("https"))) {
            String path;
            uri->GetPath(&path);
            if (path == NULL || path.EndWith(".m3u8") || path.EndWith(".m3u")
                    || path.EndWith(".mpd")) {
                // HLS or DASH streaming source
                return FALSE;
            }
            // HTTP progressive download streaming source
            return TRUE;
        }
    }
    return FALSE;
}

ECode VideoView::SetSubtitleWidget(
    /* [in] */ ISubtitleTrackRenderingWidget* subtitleWidget)
{
    if (mSubtitleWidget.Get() == subtitleWidget) {
        return NOERROR;
    }

    Boolean attachedToWindow = FALSE;
    IsAttachedToWindow(&attachedToWindow);
    if (mSubtitleWidget != NULL) {
        if (attachedToWindow) {
            mSubtitleWidget->OnDetachedFromWindow();
        }

        mSubtitleWidget->SetOnChangedListener(NULL);
    }

    mSubtitleWidget = subtitleWidget;

    if (subtitleWidget != NULL) {
        if (mSubtitlesChangedListener == NULL) {
            mSubtitlesChangedListener = new ChangedListener(this);
        }

        SetWillNotDraw(FALSE);
        subtitleWidget->SetOnChangedListener(mSubtitlesChangedListener);

        if (attachedToWindow) {
            subtitleWidget->OnAttachedToWindow();
            RequestLayout();
        }
    }
    else {
        SetWillNotDraw(TRUE);
    }

    Invalidate();
    return NOERROR;
}

ECode VideoView::GetSubtitleLooper(
    /* [out] */ ILooper** looper)
{
    VALIDATE_NOT_NULL(looper);
    AutoPtr<ILooper> l = Looper::GetMainLooper();
    *looper = l;
    REFCOUNT_ADD(*looper);
    return NOERROR;
}

}// namespace Widget
}// namespace Droid
}// namespace Elastos

