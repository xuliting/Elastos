module.exports = function(aoElastos, aoActivity){
//--------common definition----begin----
    var CObject = aoElastos.CObject;

    var CString = aoElastos.Core.CString;
    var Droid_New = aoElastos.Droid.New;
    var Core_New = aoElastos.Core.New;

    var R = aoElastos.Application.R;

    var oActivity = aoActivity.ActivityInstance;
    var oHandler = aoActivity.ActivityHandler;

    function CActivityListener(host) {
        this.mHost = host;
    }
    var _apt = CActivityListener.prototype;

    var IView__VISIBLE = 0x00000000;
    var IView__INVISIBLE = 0x00000004;
    var IView__GONE = 0x00000008;
    var IMotionEvent__ACTION_DOWN = 0;
    var IMotionEvent__ACTION_UP = 1;
    var IContext__LAYOUT_INFLATER_SERVICE = "layout_inflater";

//--------common definition----end----

//--------.h----begin----

// #ifndef _ELASTOS_DROID_CALCULATOR2_CALCULATOR_H__
// #define _ELASTOS_DROID_CALCULATOR2_CALCULATOR_H__

// #include "_Elastos.Droid.Calculator2.h"
// #include "elastos/droid/calculator2/CalculatorEditText.h"
// #include "elastos/droid/calculator2/CalculatorExpressionEvaluator.h"
// #include "elastos/droid/calculator2/CalculatorExpressionTokenizer.h"
// #include <elastos/droid/animation/AnimatorListenerAdapter.h>
// #include <elastos/droid/app/Activity.h>
// #include <elastos/droid/ext/frameworkext.h>
// #include <Elastos.Droid.Text.h>
// #include <Elastos.Droid.Support.V4.View.h>
// #include <elastos/core/Object.h>

// using Elastos::Droid::Animation::IAnimator;
// using Elastos::Droid::Animation::IAnimatorListener;
// using Elastos::Droid::Animation::IValueAnimator;
// using Elastos::Droid::Animation::IAnimatorUpdateListener;
// using Elastos::Droid::Animation::AnimatorListenerAdapter;
// using Elastos::Droid::App::Activity;
// using Elastos::Droid::Text::IEditable;
// using Elastos::Droid::Text::IEditableFactory;
// using Elastos::Droid::Text::ITextWatcher;
// using Elastos::Droid::View::EIID_IViewOnLongClickListener;
// using Elastos::Droid::View::IKeyEvent;
// using Elastos::Droid::View::IView;
// using Elastos::Droid::View::IViewGroupOverlay;
// using Elastos::Droid::View::IViewOnLongClickListener;
// using Elastos::Droid::View::IViewOnKeyListener;
// using Elastos::Droid::Widget::ITextView;
// using Elastos::Droid::Os::IBundle;
// using Elastos::Droid::Support::V4::View::IViewPager;

// namespace Elastos {
// namespace Droid {
// namespace Calculator2 {

// class Calculator
//     : public Activity
//     , public IOnTextSizeChangeListener
//     , public IEvaluateCallback
//     , public IViewOnLongClickListener
//     , public ICalculator
// {
// private:
//     class MyTextWatcher
//         : public Object
//         , public ITextWatcher
//     {
//         friend class Calculator;

//     public:
//         MyTextWatcher(
//             /* [in] */ Calculator* host);

//         CAR_INTERFACE_DECL()

//         CARAPI BeforeTextChanged(
//             /* [in] */ ICharSequence* charSequence,
//             /* [in] */ Int32 start,
//             /* [in] */ Int32 count,
//             /* [in] */ Int32 after);

//         CARAPI OnTextChanged(
//             /* [in] */ ICharSequence* charSequence,
//             /* [in] */ Int32 start,
//             /* [in] */ Int32 count,
//             /* [in] */ Int32 after);

//         CARAPI AfterTextChanged(
//             /* [in] */ IEditable* editable);

//     private:
//         Calculator* mHost;
//     };

//     class MyOnKeyListener
//         : public Object
//         , public IViewOnKeyListener
//     {
//         friend class Calculator;

//     public:
//         MyOnKeyListener(
//             /* [in] */ Calculator* host);

//         CAR_INTERFACE_DECL()

//         CARAPI OnKey(
//             /* [in] */ IView* view,
//             /* [in] */ Int32 keyCode,
//             /* [in] */ IKeyEvent* keyEvent,
//             /* [out] */ Boolean* result);

//     private:
//         Calculator* mHost;
//     };

//     class MyEditableFactory
//         : public Object
//         , public IEditableFactory
//     {
//         friend class Calculator;

//     public:
//         MyEditableFactory(
//             /* [in] */ Calculator* host);

//         CAR_INTERFACE_DECL()

//         CARAPI NewEditable(
//             /* [in] */ ICharSequence* source,
//             /* [out] */ IEditable** editable);

//     private:
//         Calculator* mHost;
//     };

//     class RevealAnimatorListenerAdapter
//         : public AnimatorListenerAdapter
//     {
//         friend class Calculator;

//     public:
//         RevealAnimatorListenerAdapter(
//             /* [in] */ Calculator* host,
//             /* [in] */ IViewGroupOverlay* vgo,
//             /* [in] */ IView* view);

//         CARAPI OnAnimationEnd(
//             /* [in] */ IAnimator* animation);

//     private:
//         Calculator* mHost;
//         AutoPtr<IViewGroupOverlay> mVgo;
//         AutoPtr<IView> mView;
//     };

//     class OnClearAnimatorListenerAdapter
//         : public AnimatorListenerAdapter
//     {
//         friend class Calculator;

//     public:
//         OnClearAnimatorListenerAdapter(
//             /* [in] */ Calculator* host);

//         CARAPI OnAnimationEnd(
//             /* [in] */ IAnimator* animation);

//     private:
//         Calculator* mHost;
//     };

//     class OnErrorAnimatorListenerAdapter
//         : public AnimatorListenerAdapter
//     {
//         friend class Calculator;

//     public:
//         OnErrorAnimatorListenerAdapter(
//             /* [in] */ Calculator* host,
//             /* [in] */ Int32 errorResourceId);

//         CARAPI OnAnimationEnd(
//             /* [in] */ IAnimator* animation);

//     private:
//         Calculator* mHost;
//         Int32 mErrorResId;
//     };

//     class MyAnimatorUpdateListener
//         : public Object
//         , public IAnimatorUpdateListener
//     {
//         friend class Calculator;

//     public:
//         MyAnimatorUpdateListener(
//             /* [in] */ Calculator* host);

//         CAR_INTERFACE_DECL()

//         CARAPI OnAnimationUpdate(
//             /* [in] */ IValueAnimator* valueAnimator);

//     private:
//         Calculator* mHost;
//     };

//     class OnResultAnimatorListenerAdapter
//         : public AnimatorListenerAdapter
//     {
//         friend class Calculator;

//     public:
//         OnResultAnimatorListenerAdapter(
//             /* [in] */ Calculator* host,
//             /* [in] */ const String& result,
//             /* [in] */ Int32 color);

//         CARAPI OnAnimationStart(
//             /* [in] */ IAnimator* animation);

//         CARAPI OnAnimationEnd(
//             /* [in] */ IAnimator* animation);

//     private:
//         Calculator* mHost;
//         String mResult;
//         Int32 mColor;
//     };

// public:
//     Calculator();

//     CARAPI constructor();

//     virtual ~Calculator();

//     CAR_INTERFACE_DECL()

//     CARAPI OnBackPressed();

//     CARAPI OnUserInteraction();

//     CARAPI OnButtonClick(
//         /* [in] */ IView* iview);

//     CARAPI OnLongClick(
//         /* [in] */ IView* iview,
//         /* [out] */ Boolean* result);

//     CARAPI OnEvaluate(
//         /* [in] */ const String& expr,
//         /* [in] */ const String& result,
//         /* [in] */ Int32 errorResourceId);

//     CARAPI OnTextSizeChanged(
//         /* [in] */ ITextView* textView,
//         /* [in] */ Float oldSize);

// protected:
//     CARAPI OnCreate(
//         /* [in] */ IBundle* savedInstanceState);

//     CARAPI OnSaveInstanceState(
//         /*[in] */ /*@NonNull */IBundle* outState);

// private:
//     CARAPI_(void) SetState(
//         /*[in] */ CalculatorState state);

//     CARAPI_(void) OnEquals();

//     CARAPI_(void) OnDelete();

//     CARAPI_(void) Reveal(
//         /* [in] */ IView* sourceView,
//         /* [in] */ Int32 colorRes,
//         /* [in] */ IAnimatorListener* listener);

//     CARAPI_(void) OnClear();

//     CARAPI_(void) OnError(
//         /* [in] */ Int32 errorResourceId);

//     CARAPI_(void) OnResult(
//         /* [in] */ const String& result);

// private:
//     static const String NAME;

//     // instance state keys
//     static const String KEY_CURRENT_STATE;
//     static const String KEY_CURRENT_EXPRESSION;

//     AutoPtr<ITextWatcher> mFormulaTextWatcher;
//     AutoPtr<IViewOnKeyListener> mFormulaOnKeyListener;
//     AutoPtr<IEditableFactory> mFormulaEditableFactory;

//     CalculatorState mCurrentState;
//     AutoPtr<ICalculatorExpressionTokenizer> mTokenizer;
//     AutoPtr<ICalculatorExpressionEvaluator> mEvaluator;

//     AutoPtr<IView> mDisplayView;
//     AutoPtr<ICalculatorEditText> mFormulaEditText;
//     AutoPtr<ICalculatorEditText> mResultEditText;
//     AutoPtr<IViewPager> mPadViewPager;
//     AutoPtr<IView> mDeleteButton;
//     AutoPtr<IView> mEqualButton;
//     AutoPtr<IView> mClearButton;

//     AutoPtr<IView> mCurrentButton;
//     AutoPtr<IAnimator> mCurrentAnimator;
// };

// } // namespace Calculator2
// } // namespace Droid
// } // namespace Elastos

// #endif // _ELASTOS_DROID_CALCULATOR2_CALCULATOR_H__

//--------.h----end----

//--------.cpp----begin----

// #include "elastos/droid/calculator2/Calculator.h"
// #include "elastos/droid/calculator2/CalculatorExpressionBuilder.h"
// #include "Elastos.Droid.Animation.h"
// #include "Elastos.Droid.Content.h"
// #include "Elastos.Droid.Graphics.h"
// #include "Elastos.Droid.Os.h"
// #include "Elastos.Droid.View.h"
// #include "Elastos.Droid.Widget.h"
// #include "R.h"
// #include <elastos/droid/R.h>
// #include <elastos/droid/text/TextUtils.h>
// #include <elastos/droid/view/View.h>
// #include <elastos/core/Math.h>
// #include <elastos/core/StringUtils.h>
// #include <elastos/core/CoreUtils.h>
// #include <elastos/utility/logging/Slogger.h>

// using Elastos::Utility::Logging::Slogger;
// using Elastos::Droid::Animation::CAnimatorSet;
// using Elastos::Droid::Animation::CArgbEvaluator;
// using Elastos::Droid::Animation::CObjectAnimatorHelper;
// using Elastos::Droid::Animation::CValueAnimatorHelper;
// using Elastos::Droid::Animation::EIID_IAnimatorUpdateListener;
// using Elastos::Droid::Animation::ITypeEvaluator;
// using Elastos::Droid::Animation::IAnimatorSet;
// using Elastos::Droid::Animation::ITimeInterpolator;
// using Elastos::Droid::Animation::IObjectAnimator;
// using Elastos::Droid::Animation::IObjectAnimatorHelper;
// using Elastos::Droid::Animation::IAnimatorSetBuilder;
// using Elastos::Droid::Animation::IValueAnimatorHelper;
// using Elastos::Droid::Animation::IAnimatorUpdateListener;
// using Elastos::Droid::Animation::IValueAnimator;
// using Elastos::Droid::Content::Res::IResources;
// using Elastos::Droid::Graphics::CRect;
// using Elastos::Droid::Graphics::IRect;
// using Elastos::Droid::Os::CBundleHelper;
// using Elastos::Droid::Os::IBundle;
// using Elastos::Droid::Os::IBundleHelper;
// using Elastos::Droid::Text::EIID_IEditableFactory;
// using Elastos::Droid::Text::EIID_ITextWatcher;
// using Elastos::Droid::Text::TextUtils;
// using Elastos::Droid::View::Animation::CAccelerateDecelerateInterpolator;
// using Elastos::Droid::View::CViewAnimationUtilsHelper;
// using Elastos::Droid::View::EIID_IViewOnKeyListener;
// using Elastos::Droid::View::IKeyEvent;
// using Elastos::Droid::View::IViewAnimationUtilsHelper;
// using Elastos::Droid::View::IViewGroupOverlay;
// using Elastos::Droid::View::IViewOverlay;
// using Elastos::Droid::View::IWindow;
// using Elastos::Droid::View::CView;
// using Elastos::Droid::View::View;
// using Elastos::Core::CoreUtils;
// using Elastos::Core::StringUtils;
// using Elastos::Core::Math;

// namespace Elastos {
// namespace Droid {
// namespace Calculator2 {

// const String Calculator::NAME("Calculator");
// const String Calculator::KEY_CURRENT_STATE = NAME + "_currentState";
// const String Calculator::KEY_CURRENT_EXPRESSION = NAME + "_currentExpression";

// //----------------------------------------------------------------
// //           Calculator::MyTextWatcher
// //----------------------------------------------------------------
// Calculator::MyTextWatcher::MyTextWatcher(
//     /* [in] */ Calculator* host)
//     : mHost(host)
// {}

// CAR_INTERFACE_IMPL(Calculator::MyTextWatcher, Object, ITextWatcher)

// ECode Calculator::MyTextWatcher::BeforeTextChanged(
//     /* [in] */ ICharSequence* charSequence,
//     /* [in] */ Int32 start,
//     /* [in] */ Int32 count,
//     /* [in] */ Int32 after)
// {
//     return NOERROR;
// }

// ECode Calculator::MyTextWatcher::OnTextChanged(
//     /* [in] */ ICharSequence* charSequence,
//     /* [in] */ Int32 start,
//     /* [in] */ Int32 count,
//     /* [in] */ Int32 after)
// {
//     return NOERROR;
// }

// ECode Calculator::MyTextWatcher::AfterTextChanged(
//     /* [in] */ IEditable* editable)
// {
//     mHost->SetState(INPUT);
//     return mHost->mEvaluator->Evaluate(ICharSequence::Probe(editable), IEvaluateCallback::Probe(mHost));
// }

// //----------------------------------------------------------------
// //           Calculator::MyOnKeyListener
// //----------------------------------------------------------------
// Calculator::MyOnKeyListener::MyOnKeyListener(
//     /* [in] */ Calculator* host)
//     : mHost(host)
// {}

// CAR_INTERFACE_IMPL(Calculator::MyOnKeyListener, Object, IViewOnKeyListener)

// ECode Calculator::MyOnKeyListener::OnKey(
//     /* [in] */ IView* view,
//     /* [in] */ Int32 keyCode,
//     /* [in] */ IKeyEvent* keyEvent,
//     /* [out] */ Boolean* result)
// {
//     VALIDATE_NOT_NULL(result)
//     switch (keyCode) {
//         case IKeyEvent::KEYCODE_NUMPAD_ENTER:
//         case IKeyEvent::KEYCODE_ENTER: {
//             Int32 action;
//             keyEvent->GetAction(&action);
//             if (action == IKeyEvent::ACTION_UP) {
//                 mHost->mCurrentButton = mHost->mEqualButton;
//                 mHost->OnEquals();
//             }
//             // ignore all other actions
//             *result = TRUE;
//             return NOERROR;
//         }
//     }
//     *result = FALSE;
//     return NOERROR;
// }

// //----------------------------------------------------------------
// //           Calculator::MyEditableFactory
// //----------------------------------------------------------------

// Calculator::MyEditableFactory::MyEditableFactory(
//     /* [in] */ Calculator* host)
//     : mHost(host)
// {}

// CAR_INTERFACE_IMPL(Calculator::MyEditableFactory, Object, IEditableFactory)

// ECode Calculator::MyEditableFactory::NewEditable(
//     /* [in] */ ICharSequence* source,
//     /* [out] */ IEditable** editable)
// {
//     VALIDATE_NOT_NULL(editable);

//     Boolean isEdited = mHost->mCurrentState == INPUT
//                     || mHost->mCurrentState == ERROR;
//     AutoPtr<CalculatorExpressionBuilder> etl = new CalculatorExpressionBuilder();
//     etl->constructor(source, ICalculatorExpressionTokenizer::Probe(mHost->mTokenizer), isEdited);
//     *editable = IEditable::Probe(etl);
//     REFCOUNT_ADD(*editable);
//     return NOERROR;
// }

// //----------------------------------------------------------------
// //           Calculator::RevealAnimatorListenerAdapter
// //----------------------------------------------------------------
// Calculator::RevealAnimatorListenerAdapter::RevealAnimatorListenerAdapter(
//     /* [in] */ Calculator* host,
//     /* [in] */ IViewGroupOverlay* vgo,
//     /* [in] */ IView* view)
//     : mHost(host)
//     , mVgo(vgo)
//     , mView(view)
// {}

// ECode Calculator::RevealAnimatorListenerAdapter::OnAnimationEnd(
//     /* [in] */ IAnimator* animation)
// {
//     mVgo->Remove(mView);
//     mHost->mCurrentAnimator = NULL;
//     return NOERROR;
// }

// //----------------------------------------------------------------
// //           Calculator::OnClearAnimatorListenerAdapter
// //----------------------------------------------------------------
// Calculator::OnClearAnimatorListenerAdapter::OnClearAnimatorListenerAdapter(
//     /* [in] */ Calculator* host)
//     : mHost(host)
// {}

// ECode Calculator::OnClearAnimatorListenerAdapter::OnAnimationEnd(
//     /* [in] */ IAnimator* animation)
// {
//     AutoPtr<IEditable> editable;
//     ITextView::Probe(mHost->mFormulaEditText)->GetEditableText((IEditable**)&editable);
//     return editable->Clear();
// }

// //----------------------------------------------------------------
// //           Calculator::OnErrorAnimatorListenerAdapter
// //----------------------------------------------------------------
// Calculator::OnErrorAnimatorListenerAdapter::OnErrorAnimatorListenerAdapter(
//     /* [in] */ Calculator* host,
//     /* [in] */ Int32 errorResourceId)
//     : mHost(host)
//     , mErrorResId(errorResourceId)
// {}

// ECode Calculator::OnErrorAnimatorListenerAdapter::OnAnimationEnd(
//     /* [in] */ IAnimator* animation)
// {
//     mHost->SetState(ERROR);
//     return ITextView::Probe(mHost->mResultEditText)->SetText(
//             StringUtils::ParseCharSequence(StringUtils::ToString(mErrorResId)));
// }

// //----------------------------------------------------------------
// //           Calculator::MyAnimatorUpdateListener
// //----------------------------------------------------------------
// Calculator::MyAnimatorUpdateListener::MyAnimatorUpdateListener(
//     /* [in] */ Calculator* host)
//     : mHost(host)
// {}

// CAR_INTERFACE_IMPL(Calculator::MyAnimatorUpdateListener, Object, IAnimatorUpdateListener)

// ECode Calculator::MyAnimatorUpdateListener::OnAnimationUpdate(
//     /* [in] */ IValueAnimator* valueAnimator)
// {
//     AutoPtr<IInterface> obj;
//     valueAnimator->GetAnimatedValue((IInterface**)&obj);
//     AutoPtr<IInteger32> ivol = IInteger32::Probe(obj);
//     Int32 vol;
//     ivol->GetValue(&vol);
//     return ITextView::Probe(mHost->mResultEditText)->SetTextColor(vol);
// }

// //----------------------------------------------------------------
// //           Calculator::OnResultAnimatorListenerAdapter
// //----------------------------------------------------------------
// Calculator::OnResultAnimatorListenerAdapter::OnResultAnimatorListenerAdapter(
//     /* [in] */ Calculator* host,
//     /* [in] */ const String& result,
//     /* [in] */ Int32 color)
//     : mHost(host)
//     , mResult(result)
//     , mColor(color)
// {}

// ECode Calculator::OnResultAnimatorListenerAdapter::OnAnimationStart(
//     /* [in] */ IAnimator* animation)
// {
//     return ITextView::Probe(mHost->mResultEditText)->SetText(StringUtils::ParseCharSequence(mResult));
// }

// ECode Calculator::OnResultAnimatorListenerAdapter::OnAnimationEnd(
//     /* [in] */ IAnimator* animation)
// {
//     // Reset all of the values modified during the animation.
//     ITextView::Probe(mHost->mResultEditText)->SetTextColor(mColor);
//     IView* ret = IView::Probe(mHost->mResultEditText);
//     ret->SetScaleX(1.0f);
//     ret->SetScaleY(1.0f);
//     ret->SetTranslationX(0.0f);
//     ret->SetTranslationY(0.0f);
//     IView::Probe(mHost->mFormulaEditText)->SetTranslationY(0.0f);

//     // Finally update the formula to use the current result.
//     ITextView::Probe(mHost->mFormulaEditText)->SetText(StringUtils::ParseCharSequence(mResult));
//     mHost->SetState(RESULT);

//     mHost->mCurrentAnimator = NULL;
//     return NOERROR;
// }
// //----------------------------------------------------------------
// //           Calculator
// //----------------------------------------------------------------
// Calculator::Calculator()
// {
//     mFormulaTextWatcher = new MyTextWatcher(this);
//     mFormulaOnKeyListener = new MyOnKeyListener(this);
//     mFormulaEditableFactory = new MyEditableFactory(this);
// }

// Calculator::~Calculator()
// {}

// ECode Calculator::constructor()
// {
//     return Activity::constructor();
// }

// CAR_INTERFACE_IMPL(Calculator, Activity, IOnTextSizeChangeListener, IEvaluateCallback, IViewOnLongClickListener, ICalculator)

// ECode Calculator::OnCreate(
//     /* [in] */ IBundle* savedInstanceState)
// {
//     Activity::OnCreate(savedInstanceState);
//     SetContentView(R::layout::activity_calculator);

//     mDisplayView = FindViewById(R::id::display);
//     mFormulaEditText = ICalculatorEditText::Probe(FindViewById(R::id::formula));
//     mResultEditText = ICalculatorEditText::Probe(FindViewById(R::id::result));
//     AutoPtr<IView> tmpView = FindViewById(R::id::pad_pager);
//     mPadViewPager = IViewPager::Probe(FindViewById(R::id::pad_pager));
//     mDeleteButton = FindViewById(R::id::del);
//     mClearButton = FindViewById(R::id::clr);

//     tmpView = FindViewById(R::id::pad_numeric);
//     tmpView->FindViewById(R::id::eq, (IView**)&mEqualButton);
//     Int32 visibility;
//     ;
//     if (mEqualButton == NULL || (mEqualButton->GetVisibility(&visibility), visibility != IView::VISIBLE)) {
//         tmpView = NULL;
//         tmpView = FindViewById(R::id::pad_operator);
//         tmpView->FindViewById(R::id::eq, (IView**)&mEqualButton);
//     }

//     AutoPtr<CalculatorExpressionTokenizer> cet = new CalculatorExpressionTokenizer();
//     cet->constructor(IContext::Probe(this));
//     mTokenizer = ICalculatorExpressionTokenizer::Probe(cet);

//     AutoPtr<CalculatorExpressionEvaluator> elr = new CalculatorExpressionEvaluator();
//     elr->constructor(ICalculatorExpressionTokenizer::Probe(mTokenizer));
//     mEvaluator = ICalculatorExpressionEvaluator::Probe(elr);

//     AutoPtr<IBundleHelper> bhl;
//     CBundleHelper::AcquireSingleton((IBundleHelper**)&bhl);
//     AutoPtr<IBundle> empty;
//     bhl->GetEMPTY((IBundle**)&empty);
//     savedInstanceState = savedInstanceState == NULL ? empty.Get() : savedInstanceState;
//     Int32 vol;
//     savedInstanceState->GetInt32(KEY_CURRENT_STATE, INPUT, &vol);
//     SetState(vol);
//     String str;
//     savedInstanceState->GetString(KEY_CURRENT_EXPRESSION, String(""), &str);
//     String text;
//     mTokenizer->GetLocalizedExpression(str, &text);
//     AutoPtr<ITextView> textView = ITextView::Probe(mFormulaEditText);
//     textView->SetText(StringUtils::ParseCharSequence(text));
//     AutoPtr<ICharSequence> cs;
//     textView->GetText((ICharSequence**)&cs);
//     mEvaluator->Evaluate(cs, IEvaluateCallback::Probe(this));

//     textView->SetEditableFactory(mFormulaEditableFactory);
//     textView->AddTextChangedListener(mFormulaTextWatcher);
//     IView::Probe(mFormulaEditText)->SetOnKeyListener(IViewOnKeyListener::Probe(mFormulaOnKeyListener));
//     mFormulaEditText->SetOnTextSizeChangeListener(IOnTextSizeChangeListener::Probe(this));
//     return mDeleteButton->SetOnLongClickListener(IViewOnLongClickListener::Probe(this));
// }

// ECode Calculator::OnSaveInstanceState(
//     /*[in] */ /*@NonNull */IBundle* outState)
// {
//     VALIDATE_NOT_NULL(outState);

//     // If there's an animation in progress, cancel it first to ensure our state is up-to-date.
//     if (mCurrentAnimator != NULL) {
//         mCurrentAnimator->Cancel();
//     }

//     Activity::OnSaveInstanceState(outState);

//     outState->PutInt32(KEY_CURRENT_STATE, mCurrentState);
//     AutoPtr<ICharSequence> cs;
//     ITextView::Probe(mFormulaEditText)->GetText((ICharSequence**)&cs);
//     String str;
//     cs->ToString(&str);
//     String ret;
//     mTokenizer->GetNormalizedExpression(str, &ret);
//     return outState->PutString(KEY_CURRENT_EXPRESSION, ret);
// }

// void Calculator::SetState(
//     /*[in] */ CalculatorState state)
// {
//     if (mCurrentState != state) {
//         mCurrentState = state;

//         if (state == RESULT || state == ERROR) {
//             mDeleteButton->SetVisibility(IView::GONE);
//             mClearButton->SetVisibility(IView::VISIBLE);
//         }
//         else {
//             mDeleteButton->SetVisibility(IView::VISIBLE);
//             mClearButton->SetVisibility(IView::GONE);
//         }

//         AutoPtr<IResources> res;
//         GetResources((IResources**)&res);
//         AutoPtr<IWindow> window;
//         GetWindow((IWindow**)&window);
//         if (state == ERROR) {
//             Int32 errorColor;
//             res->GetColor(R::color::calculator_error_color, &errorColor);
//             ITextView::Probe(mFormulaEditText)->SetTextColor(errorColor);
//             ITextView::Probe(mResultEditText)->SetTextColor(errorColor);
//             window->SetStatusBarColor(errorColor);
//         }
//         else {
//             Int32 color;
//             res->GetColor(R::color::display_formula_text_color, &color);
//             ITextView::Probe(mFormulaEditText)->SetTextColor(color);
//             res->GetColor(R::color::display_result_text_color, &color);
//             ITextView::Probe(mResultEditText)->SetTextColor(color);
//             res->GetColor(R::color::calculator_accent_color, &color);
//             window->SetStatusBarColor(color);
//         }
//     }
// }

// ECode Calculator::OnBackPressed()
// {
//     Int32 currentItem;
//     if (mPadViewPager == NULL || (mPadViewPager->GetCurrentItem(&currentItem), currentItem == 0)) {
//         // If the user is currently looking at the first pad (or the pad is not paged),
//         // allow the system to handle the Back button.
//         Activity::OnBackPressed();
//     }
//     else {
//         // Otherwise, select the previous pad.
//         mPadViewPager->GetCurrentItem(&currentItem);
//         mPadViewPager->SetCurrentItem(currentItem - 1);
//     }
//     return NOERROR;
// }

// ECode Calculator::OnUserInteraction()
// {
//     Activity::OnUserInteraction();

//     // If there's an animation in progress, cancel it so the user interaction can be handled
//     // immediately.
//     if (mCurrentAnimator != NULL) {
//         mCurrentAnimator->Cancel();
//     }
//     return NOERROR;
// }

// ECode Calculator::OnButtonClick(
//     /* [in] */ IView* iview)
// {
//     mCurrentButton = iview;

//     Int32 id;
//     iview->GetId(&id);
//     switch (id) {
//         case R::id::eq:
//             OnEquals();
//             break;
//         case R::id::del:
//             OnDelete();
//             break;
//         case R::id::clr:
//             OnClear();
//             break;
//         case R::id::fun_cos:
//         case R::id::fun_ln:
//         case R::id::fun_log:
//         case R::id::fun_sin:
//         case R::id::fun_tan:
//             {
//                 // Add left parenthesis after functions.
//                 AutoPtr<ICharSequence> cs;
//                 ITextView::Probe(iview)->GetText((ICharSequence**)&cs);
//                 ITextView::Probe(mFormulaEditText)->Append(cs);
//                 ITextView::Probe(mFormulaEditText)->Append(StringUtils::ParseCharSequence(String("(")));
//                 break;
//             }

//         default:
//             {
//                 AutoPtr<ICharSequence> cs;
//                 ITextView::Probe(iview)->GetText((ICharSequence**)&cs);
//                 ITextView::Probe(mFormulaEditText)->Append(cs);
//                 break;
//             }
//     }
//     return NOERROR;
// }

// ECode Calculator::OnLongClick(
//     /* [in] */ IView* iview,
//     /* [out] */ Boolean* result)
// {
//     VALIDATE_NOT_NULL(result);

//     mCurrentButton = iview;

//     Int32 id;
//     iview->GetId(&id);
//     if (id == R::id::del) {
//         OnClear();
//         *result = TRUE;
//         return NOERROR;
//     }
//     *result = FALSE;
//     return NOERROR;
// }

// ECode Calculator::OnEvaluate(
//     /* [in] */ const String& expr,
//     /* [in] */ const String& result,
//     /* [in] */ Int32 errorResourceId)
// {
//     if (mCurrentState == INPUT) {
//         ITextView::Probe(mResultEditText)->SetText(StringUtils::ParseCharSequence(result));
//     }
//     else if (errorResourceId != INVALID_RES_ID) {
//         OnError(errorResourceId);
//     }
//     else if (!TextUtils::IsEmpty(result)) {
//         OnResult(result);
//     }
//     else if (mCurrentState == EVALUATE) {
//         // The current expression cannot be evaluated -> return to the input state.
//         SetState(INPUT);
//     }

//     Boolean flag = FALSE;
//     return IView::Probe(mFormulaEditText)->RequestFocus(&flag);
// }

// ECode Calculator::OnTextSizeChanged(
//     /* [in] */ ITextView* textView,
//     /* [in] */ Float oldSize)
// {
//     if (mCurrentState != INPUT) {
//         // Only animate text changes that occur from user input.
//         return NOERROR;
//     }

//     // Calculate the values needed to perform the scale and translation animations,
//     // maintaining the same apparent baseline for the displayed text.
//     Float textSize;
//     textView->GetTextSize(&textSize);
//     Float textScale = oldSize / textSize;

//     AutoPtr<IView> tv = IView::Probe(textView);
//     Int32 width;
//     tv->GetWidth(&width);
//     Int32 paddingEnd;
//     tv->GetPaddingEnd(&paddingEnd);
//     Float translationX = (1.0f - textScale) * (width / 2.0f - paddingEnd);
//     Int32 height;
//     tv->GetHeight(&height);
//     Int32 bottom;
//     tv->GetPaddingBottom(&bottom);
//     Float translationY = (1.0f - textScale) * (height / 2.0f - bottom);

//     AutoPtr<IAnimatorSet> animatorSet;
//     CAnimatorSet::New((IAnimatorSet**)&animatorSet);
//     AutoPtr<IObjectAnimatorHelper> oah;
//     CObjectAnimatorHelper::AcquireSingleton((IObjectAnimatorHelper**)&oah);

//     AutoPtr<ArrayOf<Float> > arr1 = ArrayOf<Float>::Alloc(2);
//     (*arr1)[0] = textScale;
//     (*arr1)[1] = 1.0;
//     AutoPtr<IObjectAnimator> o1;
//     oah->OfFloat(textView, Elastos::Droid::View::View::SCALE_X, arr1, (IObjectAnimator**)&o1);
//     AutoPtr<IObjectAnimator> o2;
//     oah->OfFloat(textView, Elastos::Droid::View::View::SCALE_Y, arr1, (IObjectAnimator**)&o2);

//     AutoPtr<ArrayOf<Float> > arr2 = ArrayOf<Float>::Alloc(2);
//     (*arr2)[0] = translationX;
//     (*arr2)[1] = 0.0;
//     AutoPtr<IObjectAnimator> o3;
//     oah->OfFloat(textView, Elastos::Droid::View::View::TRANSLATION_X, arr2, (IObjectAnimator**)&o3);

//     AutoPtr<ArrayOf<Float> > arr3 = ArrayOf<Float>::Alloc(2);
//     (*arr2)[0] = translationY;
//     (*arr2)[1] = 0.0;
//     AutoPtr<IObjectAnimator> o4;
//     oah->OfFloat(textView, Elastos::Droid::View::View::TRANSLATION_Y, arr3, (IObjectAnimator**)&o4);

//     AutoPtr<ArrayOf<IAnimator*> > animators =  ArrayOf<IAnimator*>::Alloc(4);
//     animators->Set(0, IAnimator::Probe(o1));
//     animators->Set(1, IAnimator::Probe(o2));
//     animators->Set(2, IAnimator::Probe(o3));
//     animators->Set(3, IAnimator::Probe(o4));
//     animatorSet->PlayTogether(animators);

//     AutoPtr<IResources> res;
//     GetResources((IResources**)&res);
//     Int32 integer;
//     res->GetInteger(Elastos::Droid::R::integer::config_mediumAnimTime, &integer);
//     AutoPtr<IAnimator> as = IAnimator::Probe(animatorSet);
//     as->SetDuration((Int64)integer);
//     AutoPtr<ITimeInterpolator> tip;
//     CAccelerateDecelerateInterpolator::New((ITimeInterpolator**)&tip);
//     as->SetInterpolator(tip);
//     return as->Start();
// }

// void Calculator::OnEquals()
// {
//     if (mCurrentState == INPUT) {
//         SetState(EVALUATE);
//         AutoPtr<ICharSequence> cs;
//         ITextView::Probe(mFormulaEditText)->GetText((ICharSequence**)&cs);
//         mEvaluator->Evaluate(cs, IEvaluateCallback::Probe(this));
//     }
// }

// void Calculator::OnDelete()
// {
//     // Delete works like backspace; remove the last character from the expression.
//     AutoPtr<IEditable> formulaText;
//     ITextView::Probe(mFormulaEditText)->GetEditableText((IEditable**)&formulaText);
//     Int32 formulaLength;
//     ICharSequence::Probe(formulaText)->GetLength(&formulaLength);
//     if (formulaLength > 0) {
//         formulaText->Delete(formulaLength - 1, formulaLength);
//     }
// }

// void Calculator::Reveal(
//     /* [in] */ IView* sourceView,
//     /* [in] */ Int32 colorRes,
//     /* [in] */ IAnimatorListener* listener)
// {
//     AutoPtr<IWindow> window;
//     GetWindow((IWindow**)&window);
//     AutoPtr<IView> dView;
//     window->GetDecorView((IView**)&dView);
//     AutoPtr<IViewOverlay> overlay;
//     dView->GetOverlay((IViewOverlay**)&overlay);
//     AutoPtr<IViewGroupOverlay> groupOverlay = IViewGroupOverlay::Probe(overlay);

//     AutoPtr<IRect> displayRect;
//     CRect::New((IRect**)&displayRect);
//     Boolean flag;
//     mDisplayView->GetGlobalVisibleRect(displayRect, &flag);

//     // Make reveal cover the display and status bar.
//     AutoPtr<IView> revealView;
//     CView::New(IContext::Probe(this), (IView**)&revealView);
//     Int32 bottom;
//     displayRect->GetBottom(&bottom);
//     revealView->SetBottom(bottom);
//     Int32 left;
//     displayRect->GetLeft(&left);
//     revealView->SetLeft(left);
//     Int32 right;
//     displayRect->GetRight(&right);
//     revealView->SetRight(right);
//     AutoPtr<IResources> res;
//     GetResources((IResources**)&res);
//     Int32 color;
//     res->GetColor(colorRes, &color);
//     revealView->SetBackgroundColor(color);
//     groupOverlay->Add(revealView);

//     AutoPtr<ArrayOf<Int32> > clearLocation = ArrayOf<Int32>::Alloc(2);
//     sourceView->GetLocationInWindow(clearLocation);
//     Int32 width;
//     sourceView->GetWidth(&width);
//     Int32 height;
//     sourceView->GetHeight(&height);
//     (*clearLocation)[0] += width / 2;
//     (*clearLocation)[1] += height / 2;

//     revealView->GetLeft(&left);
//     Int32 revealCenterX = (*clearLocation)[0] - left;
//     Int32 top;
//     revealView->GetTop(&top);
//     Int32 revealCenterY = (*clearLocation)[1] - top;

//     Double x1_2 = Elastos::Core::Math::Pow((Double)(left - revealCenterX), (Double)2);
//     revealView->GetRight(&right);
//     Double x2_2 = Elastos::Core::Math::Pow((Double)(right - revealCenterX), (Double)2);
//     Double y_2 = Elastos::Core::Math::Pow((Double)(top - revealCenterY), (Double)2);
//     Float revealRadius = (Float)Elastos::Core::Math::Max(
//             Elastos::Core::Math::Sqrt(x1_2 + y_2), Elastos::Core::Math::Sqrt(x2_2 + y_2));

//     AutoPtr<IViewAnimationUtilsHelper> vau;
//     CViewAnimationUtilsHelper::AcquireSingleton((IViewAnimationUtilsHelper**)&vau);
//     AutoPtr<IAnimator> revealAnimator;
//     vau->CreateCircularReveal(revealView, revealCenterX, revealCenterY, 0.0f,
//             revealRadius, (IAnimator**)&revealAnimator);

//     Int32 integer;
//     res->GetInteger(Elastos::Droid::R::integer::config_longAnimTime, &integer);
//     revealAnimator->SetDuration((Int64)integer);
//     revealAnimator->AddListener(listener);

//     AutoPtr<IObjectAnimatorHelper> oah;
//     CObjectAnimatorHelper::AcquireSingleton((IObjectAnimatorHelper**)&oah);

//     AutoPtr<ArrayOf<Float> > arr = ArrayOf<Float>::Alloc(1);
//     (*arr)[0] = 0.0f;
//     AutoPtr<IObjectAnimator> oba;
//     oah->OfFloat(revealView, Elastos::Droid::View::View::View::ALPHA, arr, (IObjectAnimator**)&oba);
//     AutoPtr<IAnimator> alphaAnimator = IAnimator::Probe(oba);
//     res->GetInteger(Elastos::Droid::R::integer::config_mediumAnimTime, &integer);
//     alphaAnimator->SetDuration((Int64)(integer));

//     AutoPtr<IAnimatorSet> animatorSet;
//     CAnimatorSet::New((IAnimatorSet**)&animatorSet);
//     AutoPtr<IAnimatorSetBuilder> asb;
//     animatorSet->Play(revealAnimator, (IAnimatorSetBuilder**)&asb);
//     asb->Before(alphaAnimator);
//     AutoPtr<ITimeInterpolator> tip;
//     CAccelerateDecelerateInterpolator::New((ITimeInterpolator**)&tip);
//     AutoPtr<IAnimator> as = IAnimator::Probe(animatorSet);
//     as->SetInterpolator(tip);

//     AutoPtr<RevealAnimatorListenerAdapter> adapter = new RevealAnimatorListenerAdapter(this, groupOverlay, revealView);
//     as->AddListener(IAnimatorListener::Probe(adapter));

//     mCurrentAnimator = as;
//     as->Start();
// }

// void Calculator::OnClear()
// {
//     AutoPtr<ICharSequence> cs;
//     ITextView::Probe(mFormulaEditText)->GetText((ICharSequence**)&cs);
//     if (TextUtils::IsEmpty(cs)) {
//         return;
//     }

//     AutoPtr<OnClearAnimatorListenerAdapter> msal = new OnClearAnimatorListenerAdapter(this);
//     Reveal(mCurrentButton, R::color::calculator_accent_color, IAnimatorListener::Probe(msal));
// }

// void Calculator::OnError(
//     /* [in] */ Int32 errorResourceId)
// {
//     if (mCurrentState != EVALUATE) {
//         // Only animate error on evaluate.
//         ITextView::Probe(mResultEditText)->SetText(errorResourceId);
//         return;
//     }

//     AutoPtr<OnErrorAnimatorListenerAdapter> adapter = new OnErrorAnimatorListenerAdapter(this, errorResourceId);
//     Reveal(mCurrentButton, R::color::calculator_error_color, IAnimatorListener::Probe(adapter));
// }

// void Calculator::OnResult(
//     /* [in] */ const String& result)
// {
//     // Calculate the values needed to perform the scale and translation animations,
//     // accounting for how the scale will affect the final position of the text.
//     Float formulaSize;
//     mFormulaEditText->GetVariableTextSize(result, &formulaSize);
//     Float resultSize;
//     ITextView::Probe(mResultEditText)->GetTextSize(&resultSize);
//     Float resultScale = formulaSize / resultSize;
//     AutoPtr<IView> resultEditText = IView::Probe(mResultEditText);
//     Int32 width, paddingEnd;
//     resultEditText->GetWidth(&width);
//     resultEditText->GetPaddingEnd(&paddingEnd);
//     Float resultTranslationX = (1.0f - resultScale) * (width / 2.0f - paddingEnd);
//     Int32 height, paddingBottom, bottom;
//     resultEditText->GetHeight(&height);
//     resultEditText->GetPaddingBottom(&paddingBottom);
//     resultEditText->GetBottom(&bottom);
//     AutoPtr<IView> formulaEditText = IView::Probe(mFormulaEditText);
//     Int32 formulaBottom, formulaPaddingBottom;
//     formulaEditText->GetBottom(&formulaBottom);
//     formulaEditText->GetPaddingBottom(&formulaPaddingBottom);
//     Float resultTranslationY = (1.0f - resultScale) * (height / 2.0f - paddingBottom) +
//             (formulaBottom - bottom) + (paddingBottom - formulaPaddingBottom);
//     Float formulaTranslationY = -formulaBottom;

//     // Use a value animator to fade to the final text color over the course of the animation.
//     Int32 resultTextColor;
//     ITextView::Probe(mResultEditText)->GetCurrentTextColor(&resultTextColor);
//     Int32 formulaTextColor;
//     ITextView::Probe(mFormulaEditText)->GetCurrentTextColor(&formulaTextColor);

//     AutoPtr<ITypeEvaluator> te;
//     CArgbEvaluator::New((ITypeEvaluator**)&te);
//     AutoPtr<IValueAnimatorHelper> vah;
//     CValueAnimatorHelper::AcquireSingleton((IValueAnimatorHelper**)&vah);
//     AutoPtr<ArrayOf<IInterface*> > arr =  ArrayOf<IInterface*>::Alloc(2);
//     arr->Set(0, CoreUtils::Convert(resultTextColor));
//     arr->Set(1, CoreUtils::Convert(formulaTextColor));
//     AutoPtr<IValueAnimator> textColorAnimator;
//     vah->OfObject(te, arr, (IValueAnimator**)&textColorAnimator);

//     AutoPtr<IAnimatorUpdateListener> maul = (IAnimatorUpdateListener*)new MyAnimatorUpdateListener(this);
//     textColorAnimator->AddUpdateListener(maul);

//     AutoPtr<IAnimatorSet> animatorSet;
//     CAnimatorSet::New((IAnimatorSet**)&animatorSet);
//     AutoPtr<IObjectAnimatorHelper> oah;
//     CObjectAnimatorHelper::AcquireSingleton((IObjectAnimatorHelper**)&oah);

//     AutoPtr<ArrayOf<Float> > arr1 = ArrayOf<Float>::Alloc(1);
//     (*arr1)[0] = resultScale;
//     AutoPtr<IObjectAnimator> ioa1;
//     oah->OfFloat(mResultEditText, Elastos::Droid::View::View::SCALE_X, arr1, (IObjectAnimator**)&ioa1);

//     AutoPtr<IObjectAnimator> ioa2;
//     oah->OfFloat(mResultEditText, Elastos::Droid::View::View::SCALE_Y, arr1, (IObjectAnimator**)&ioa2);

//     AutoPtr<ArrayOf<Float> > arr2 = ArrayOf<Float>::Alloc(1);
//     (*arr2)[0] = resultTranslationX;
//     AutoPtr<IObjectAnimator> ioa3;
//     oah->OfFloat(mResultEditText, Elastos::Droid::View::View::TRANSLATION_X, arr2, (IObjectAnimator**)&ioa3);

//     AutoPtr<ArrayOf<Float> > arr3 = ArrayOf<Float>::Alloc(1);
//     (*arr3)[0] = resultTranslationY;
//     AutoPtr<IObjectAnimator> ioa4;
//     oah->OfFloat(mResultEditText, Elastos::Droid::View::View::TRANSLATION_Y, arr3, (IObjectAnimator**)&ioa4);

//     AutoPtr<ArrayOf<Float> > arr4 = ArrayOf<Float>::Alloc(1);
//     (*arr4)[0] = formulaTranslationY;
//     AutoPtr<IObjectAnimator> ioa5;
//     oah->OfFloat(mFormulaEditText, Elastos::Droid::View::View::TRANSLATION_Y, arr4, (IObjectAnimator**)&ioa5);

//     AutoPtr<ArrayOf<IAnimator*> > animators = ArrayOf<IAnimator*>::Alloc(6);
//     animators->Set(0, IAnimator::Probe(textColorAnimator));
//     animators->Set(1, IAnimator::Probe(ioa1));
//     animators->Set(2, IAnimator::Probe(ioa2));
//     animators->Set(3, IAnimator::Probe(ioa3));
//     animators->Set(4, IAnimator::Probe(ioa4));
//     animators->Set(5, IAnimator::Probe(ioa5));
//     animatorSet->PlayTogether(animators);

//     AutoPtr<IAnimator> as = IAnimator::Probe(animatorSet);
//     AutoPtr<IResources> res;
//     GetResources((IResources**)&res);
//     Int32 vol;
//     res->GetInteger(Elastos::Droid::R::integer::config_longAnimTime, &vol);
//     as->SetDuration((Int64)vol);
//     AutoPtr<ITimeInterpolator> tip;
//     CAccelerateDecelerateInterpolator::New((ITimeInterpolator**)&tip);
//     as->SetInterpolator(tip);
//     AutoPtr<OnResultAnimatorListenerAdapter> adapter = new OnResultAnimatorListenerAdapter(this, result, resultTextColor);
//     as->AddListener(IAnimatorListener::Probe(adapter));

//     mCurrentAnimator = as;
//     as->Start();
// }

// } // namespace Calculator2
// } // namespace Droid
// } // namespace Elastos

//--------.cpp----end----

//--------.java----begin----

/*
//  * Copyright (C) 2014 The Android Open Source Project
//  *
//  * Licensed under the Apache License, Version 2.0 (the "License");
//  * you may not use this file except in compliance with the License.
//  * You may obtain a copy of the License at
//  *
//  *   http://www.apache.org/licenses/LICENSE-2.0
//  *
//  * Unless required by applicable law or agreed to in writing, software
//  * distributed under the License is distributed on an "AS IS" BASIS,
//  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  * See the License for the specific language governing permissions and
//  * limitations under the License.
//  */

// package com.android.calculator2;

// import android.animation.Animator;
// import android.animation.Animator.AnimatorListener;
// import android.animation.AnimatorListenerAdapter;
// import android.animation.AnimatorSet;
// import android.animation.ArgbEvaluator;
// import android.animation.ObjectAnimator;
// import android.animation.ValueAnimator;
// import android.animation.ValueAnimator.AnimatorUpdateListener;
// import android.app.Activity;
// import android.graphics.Rect;
// import android.os.Bundle;
// import android.support.annotation.NonNull;
// import android.support.v4.view.ViewPager;
// import android.text.Editable;
// import android.text.TextUtils;
// import android.text.TextWatcher;
// import android.view.KeyEvent;
// import android.view.View;
// import android.view.View.OnKeyListener;
// import android.view.View.OnLongClickListener;
// import android.view.ViewAnimationUtils;
// import android.view.ViewGroupOverlay;
// import android.view.animation.AccelerateDecelerateInterpolator;
// import android.widget.Button;
// import android.widget.TextView;

// import com.android.calculator2.CalculatorEditText.OnTextSizeChangeListener;
// import com.android.calculator2.CalculatorExpressionEvaluator.EvaluateCallback;

// public class Calculator extends Activity
//         implements OnTextSizeChangeListener, EvaluateCallback, OnLongClickListener {

//     private static final String NAME = Calculator.class.getName();

//     // instance state keys
//     private static final String KEY_CURRENT_STATE = NAME + "_currentState";
//     private static final String KEY_CURRENT_EXPRESSION = NAME + "_currentExpression";

//     /**
//      * Constant for an invalid resource id.
//      */
//     public static final int INVALID_RES_ID = -1;

//     private enum CalculatorState {
//         INPUT, EVALUATE, RESULT, ERROR
//     }

//     private final TextWatcher mFormulaTextWatcher = new TextWatcher() {
//         @Override
//         public void beforeTextChanged(CharSequence charSequence, int start, int count, int after) {
//         }

//         @Override
//         public void onTextChanged(CharSequence charSequence, int start, int count, int after) {
//         }

//         @Override
//         public void afterTextChanged(Editable editable) {
//             setState(CalculatorState.INPUT);
//             mEvaluator.evaluate(editable, Calculator.this);
//         }
//     };

//     private final OnKeyListener mFormulaOnKeyListener = new OnKeyListener() {
//         @Override
//         public boolean onKey(View view, int keyCode, KeyEvent keyEvent) {
//             switch (keyCode) {
//                 case KeyEvent.KEYCODE_NUMPAD_ENTER:
//                 case KeyEvent.KEYCODE_ENTER:
//                     if (keyEvent.getAction() == KeyEvent.ACTION_UP) {
//                         mCurrentButton = mEqualButton;
//                         onEquals();
//                     }
//                     // ignore all other actions
//                     return true;
//             }
//             return false;
//         }
//     };

//     private final Editable.Factory mFormulaEditableFactory = new Editable.Factory() {
//         @Override
//         public Editable newEditable(CharSequence source) {
//             final boolean isEdited = mCurrentState == CalculatorState.INPUT
//                     || mCurrentState == CalculatorState.ERROR;
//             return new CalculatorExpressionBuilder(source, mTokenizer, isEdited);
//         }
//     };

//     private CalculatorState mCurrentState;
//     private CalculatorExpressionTokenizer mTokenizer;
//     private CalculatorExpressionEvaluator mEvaluator;

//     private View mDisplayView;
//     private CalculatorEditText mFormulaEditText;
//     private CalculatorEditText mResultEditText;
//     private ViewPager mPadViewPager;
//     private View mDeleteButton;
//     private View mEqualButton;
//     private View mClearButton;

//     private View mCurrentButton;
//     private Animator mCurrentAnimator;

//     @Override
//     protected void onCreate(Bundle savedInstanceState) {
//         super.onCreate(savedInstanceState);
//         setContentView(R.layout.activity_calculator);

//         mDisplayView = findViewById(R.id.display);
//         mFormulaEditText = (CalculatorEditText) findViewById(R.id.formula);
//         mResultEditText = (CalculatorEditText) findViewById(R.id.result);
//         mPadViewPager = (ViewPager) findViewById(R.id.pad_pager);
//         mDeleteButton = findViewById(R.id.del);
//         mClearButton = findViewById(R.id.clr);

//         mEqualButton = findViewById(R.id.pad_numeric).findViewById(R.id.eq);
//         if (mEqualButton == null || mEqualButton.getVisibility() != View.VISIBLE) {
//             mEqualButton = findViewById(R.id.pad_operator).findViewById(R.id.eq);
//         }

//         mTokenizer = new CalculatorExpressionTokenizer(this);
//         mEvaluator = new CalculatorExpressionEvaluator(mTokenizer);

//         savedInstanceState = savedInstanceState == null ? Bundle.EMPTY : savedInstanceState;
//         setState(CalculatorState.values()[
//                 savedInstanceState.getInt(KEY_CURRENT_STATE, CalculatorState.INPUT.ordinal())]);
//         mFormulaEditText.setText(mTokenizer.getLocalizedExpression(
//                 savedInstanceState.getString(KEY_CURRENT_EXPRESSION, "")));
//         mEvaluator.evaluate(mFormulaEditText.getText(), this);

//         mFormulaEditText.setEditableFactory(mFormulaEditableFactory);
//         mFormulaEditText.addTextChangedListener(mFormulaTextWatcher);
//         mFormulaEditText.setOnKeyListener(mFormulaOnKeyListener);
//         mFormulaEditText.setOnTextSizeChangeListener(this);
//         mDeleteButton.setOnLongClickListener(this);
//     }

//     @Override
//     protected void onSaveInstanceState(@NonNull Bundle outState) {
//         // If there's an animation in progress, cancel it first to ensure our state is up-to-date.
//         if (mCurrentAnimator != null) {
//             mCurrentAnimator.cancel();
//         }

//         super.onSaveInstanceState(outState);

//         outState.putInt(KEY_CURRENT_STATE, mCurrentState.ordinal());
//         outState.putString(KEY_CURRENT_EXPRESSION,
//                 mTokenizer.getNormalizedExpression(mFormulaEditText.getText().toString()));
//     }

//     private void setState(CalculatorState state) {
//         if (mCurrentState != state) {
//             mCurrentState = state;

//             if (state == CalculatorState.RESULT || state == CalculatorState.ERROR) {
//                 mDeleteButton.setVisibility(View.GONE);
//                 mClearButton.setVisibility(View.VISIBLE);
//             } else {
//                 mDeleteButton.setVisibility(View.VISIBLE);
//                 mClearButton.setVisibility(View.GONE);
//             }

//             if (state == CalculatorState.ERROR) {
//                 final int errorColor = getResources().getColor(R.color.calculator_error_color);
//                 mFormulaEditText.setTextColor(errorColor);
//                 mResultEditText.setTextColor(errorColor);
//                 getWindow().setStatusBarColor(errorColor);
//             } else {
//                 mFormulaEditText.setTextColor(
//                         getResources().getColor(R.color.display_formula_text_color));
//                 mResultEditText.setTextColor(
//                         getResources().getColor(R.color.display_result_text_color));
//                 getWindow().setStatusBarColor(
//                         getResources().getColor(R.color.calculator_accent_color));
//             }
//         }
//     }

//     @Override
//     public void onBackPressed() {
//         if (mPadViewPager == null || mPadViewPager.getCurrentItem() == 0) {
//             // If the user is currently looking at the first pad (or the pad is not paged),
//             // allow the system to handle the Back button.
//             super.onBackPressed();
//         } else {
//             // Otherwise, select the previous pad.
//             mPadViewPager.setCurrentItem(mPadViewPager.getCurrentItem() - 1);
//         }
//     }

//     @Override
//     public void onUserInteraction() {
//         super.onUserInteraction();

//         // If there's an animation in progress, cancel it so the user interaction can be handled
//         // immediately.
//         if (mCurrentAnimator != null) {
//             mCurrentAnimator.cancel();
//         }
//     }

//     public void onButtonClick(View view) {
//         mCurrentButton = view;

//         switch (view.getId()) {
//             case R.id.eq:
//                 onEquals();
//                 break;
//             case R.id.del:
//                 onDelete();
//                 break;
//             case R.id.clr:
//                 onClear();
//                 break;
//             case R.id.fun_cos:
//             case R.id.fun_ln:
//             case R.id.fun_log:
//             case R.id.fun_sin:
//             case R.id.fun_tan:
//                 // Add left parenthesis after functions.
//                 mFormulaEditText.append(((Button) view).getText() + "(");
//                 break;
//             default:
//                 mFormulaEditText.append(((Button) view).getText());
//                 break;
//         }
//     }

//     @Override
//     public boolean onLongClick(View view) {
//         mCurrentButton = view;

//         if (view.getId() == R.id.del) {
//             onClear();
//             return true;
//         }
//         return false;
//     }

//     @Override
//     public void onEvaluate(String expr, String result, int errorResourceId) {
//         if (mCurrentState == CalculatorState.INPUT) {
//             mResultEditText.setText(result);
//         } else if (errorResourceId != INVALID_RES_ID) {
//             onError(errorResourceId);
//         } else if (!TextUtils.isEmpty(result)) {
//             onResult(result);
//         } else if (mCurrentState == CalculatorState.EVALUATE) {
//             // The current expression cannot be evaluated -> return to the input state.
//             setState(CalculatorState.INPUT);
//         }

//         mFormulaEditText.requestFocus();
//     }

//     @Override
//     public void onTextSizeChanged(final TextView textView, float oldSize) {
//         if (mCurrentState != CalculatorState.INPUT) {
//             // Only animate text changes that occur from user input.
//             return;
//         }

//         // Calculate the values needed to perform the scale and translation animations,
//         // maintaining the same apparent baseline for the displayed text.
//         final float textScale = oldSize / textView.getTextSize();
//         final float translationX = (1.0f - textScale) *
//                 (textView.getWidth() / 2.0f - textView.getPaddingEnd());
//         final float translationY = (1.0f - textScale) *
//                 (textView.getHeight() / 2.0f - textView.getPaddingBottom());

//         final AnimatorSet animatorSet = new AnimatorSet();
//         animatorSet.playTogether(
//                 ObjectAnimator.ofFloat(textView, View.SCALE_X, textScale, 1.0f),
//                 ObjectAnimator.ofFloat(textView, View.SCALE_Y, textScale, 1.0f),
//                 ObjectAnimator.ofFloat(textView, View.TRANSLATION_X, translationX, 0.0f),
//                 ObjectAnimator.ofFloat(textView, View.TRANSLATION_Y, translationY, 0.0f));
//         animatorSet.setDuration(getResources().getInteger(android.R.integer.config_mediumAnimTime));
//         animatorSet.setInterpolator(new AccelerateDecelerateInterpolator());
//         animatorSet.start();
//     }

//     private void onEquals() {
//         if (mCurrentState == CalculatorState.INPUT) {
//             setState(CalculatorState.EVALUATE);
//             mEvaluator.evaluate(mFormulaEditText.getText(), this);
//         }
//     }

//     private void onDelete() {
//         // Delete works like backspace; remove the last character from the expression.
//         final Editable formulaText = mFormulaEditText.getEditableText();
//         final int formulaLength = formulaText.length();
//         if (formulaLength > 0) {
//             formulaText.delete(formulaLength - 1, formulaLength);
//         }
//     }

//     private void reveal(View sourceView, int colorRes, AnimatorListener listener) {
//         final ViewGroupOverlay groupOverlay =
//                 (ViewGroupOverlay) getWindow().getDecorView().getOverlay();

//         final Rect displayRect = new Rect();
//         mDisplayView.getGlobalVisibleRect(displayRect);

//         // Make reveal cover the display and status bar.
//         final View revealView = new View(this);
//         revealView.setBottom(displayRect.bottom);
//         revealView.setLeft(displayRect.left);
//         revealView.setRight(displayRect.right);
//         revealView.setBackgroundColor(getResources().getColor(colorRes));
//         groupOverlay.add(revealView);

//         final int[] clearLocation = new int[2];
//         sourceView.getLocationInWindow(clearLocation);
//         clearLocation[0] += sourceView.getWidth() / 2;
//         clearLocation[1] += sourceView.getHeight() / 2;

//         final int revealCenterX = clearLocation[0] - revealView.getLeft();
//         final int revealCenterY = clearLocation[1] - revealView.getTop();

//         final double x1_2 = Math.pow(revealView.getLeft() - revealCenterX, 2);
//         final double x2_2 = Math.pow(revealView.getRight() - revealCenterX, 2);
//         final double y_2 = Math.pow(revealView.getTop() - revealCenterY, 2);
//         final float revealRadius = (float) Math.max(Math.sqrt(x1_2 + y_2), Math.sqrt(x2_2 + y_2));

//         final Animator revealAnimator =
//                 ViewAnimationUtils.createCircularReveal(revealView,
//                         revealCenterX, revealCenterY, 0.0f, revealRadius);
//         revealAnimator.setDuration(
//                 getResources().getInteger(android.R.integer.config_longAnimTime));
//         revealAnimator.addListener(listener);

//         final Animator alphaAnimator = ObjectAnimator.ofFloat(revealView, View.ALPHA, 0.0f);
//         alphaAnimator.setDuration(
//                 getResources().getInteger(android.R.integer.config_mediumAnimTime));

//         final AnimatorSet animatorSet = new AnimatorSet();
//         animatorSet.play(revealAnimator).before(alphaAnimator);
//         animatorSet.setInterpolator(new AccelerateDecelerateInterpolator());
//         animatorSet.addListener(new AnimatorListenerAdapter() {
//             @Override
//             public void onAnimationEnd(Animator animator) {
//                 groupOverlay.remove(revealView);
//                 mCurrentAnimator = null;
//             }
//         });

//         mCurrentAnimator = animatorSet;
//         animatorSet.start();
//     }

//     private void onClear() {
//         if (TextUtils.isEmpty(mFormulaEditText.getText())) {
//             return;
//         }

//         reveal(mCurrentButton, R.color.calculator_accent_color, new AnimatorListenerAdapter() {
//             @Override
//             public void onAnimationEnd(Animator animation) {
//                 mFormulaEditText.getEditableText().clear();
//             }
//         });
//     }

//     private void onError(final int errorResourceId) {
//         if (mCurrentState != CalculatorState.EVALUATE) {
//             // Only animate error on evaluate.
//             mResultEditText.setText(errorResourceId);
//             return;
//         }

//         reveal(mCurrentButton, R.color.calculator_error_color, new AnimatorListenerAdapter() {
//             @Override
//             public void onAnimationEnd(Animator animation) {
//                 setState(CalculatorState.ERROR);
//                 mResultEditText.setText(errorResourceId);
//             }
//         });
//     }

//     private void onResult(final String result) {
//         // Calculate the values needed to perform the scale and translation animations,
//         // accounting for how the scale will affect the final position of the text.
//         final float resultScale =
//                 mFormulaEditText.getVariableTextSize(result) / mResultEditText.getTextSize();
//         final float resultTranslationX = (1.0f - resultScale) *
//                 (mResultEditText.getWidth() / 2.0f - mResultEditText.getPaddingEnd());
//         final float resultTranslationY = (1.0f - resultScale) *
//                 (mResultEditText.getHeight() / 2.0f - mResultEditText.getPaddingBottom()) +
//                 (mFormulaEditText.getBottom() - mResultEditText.getBottom()) +
//                 (mResultEditText.getPaddingBottom() - mFormulaEditText.getPaddingBottom());
//         final float formulaTranslationY = -mFormulaEditText.getBottom();

//         // Use a value animator to fade to the final text color over the course of the animation.
//         final int resultTextColor = mResultEditText.getCurrentTextColor();
//         final int formulaTextColor = mFormulaEditText.getCurrentTextColor();
//         final ValueAnimator textColorAnimator =
//                 ValueAnimator.ofObject(new ArgbEvaluator(), resultTextColor, formulaTextColor);
//         textColorAnimator.addUpdateListener(new AnimatorUpdateListener() {
//             @Override
//             public void onAnimationUpdate(ValueAnimator valueAnimator) {
//                 mResultEditText.setTextColor((int) valueAnimator.getAnimatedValue());
//             }
//         });

//         final AnimatorSet animatorSet = new AnimatorSet();
//         animatorSet.playTogether(
//                 textColorAnimator,
//                 ObjectAnimator.ofFloat(mResultEditText, View.SCALE_X, resultScale),
//                 ObjectAnimator.ofFloat(mResultEditText, View.SCALE_Y, resultScale),
//                 ObjectAnimator.ofFloat(mResultEditText, View.TRANSLATION_X, resultTranslationX),
//                 ObjectAnimator.ofFloat(mResultEditText, View.TRANSLATION_Y, resultTranslationY),
//                 ObjectAnimator.ofFloat(mFormulaEditText, View.TRANSLATION_Y, formulaTranslationY));
//         animatorSet.setDuration(getResources().getInteger(android.R.integer.config_longAnimTime));
//         animatorSet.setInterpolator(new AccelerateDecelerateInterpolator());
//         animatorSet.addListener(new AnimatorListenerAdapter() {
//             @Override
//             public void onAnimationStart(Animator animation) {
//                 mResultEditText.setText(result);
//             }

//             @Override
//             public void onAnimationEnd(Animator animation) {
//                 // Reset all of the values modified during the animation.
//                 mResultEditText.setTextColor(resultTextColor);
//                 mResultEditText.setScaleX(1.0f);
//                 mResultEditText.setScaleY(1.0f);
//                 mResultEditText.setTranslationX(0.0f);
//                 mResultEditText.setTranslationY(0.0f);
//                 mFormulaEditText.setTranslationY(0.0f);

//                 // Finally update the formula to use the current result.
//                 mFormulaEditText.setText(result);
//                 setState(CalculatorState.RESULT);

//                 mCurrentAnimator = null;
//             }
//         });

//         mCurrentAnimator = animatorSet;
//         animatorSet.start();
//     }
// }

//--------.java----end----

//--------Listener----default begin----
//
    // _apt.OnCreate = function(context, savedInstanceState){
    //     elog('====jso_activity_cb====OnCreate.begin====');
    // }  //OnCreate
    _apt.OnStart = function(context){
        elog('====jso_activity_cb====OnStart.begin====');
    }
    _apt.OnResume = function(context){
        elog('====jso_activity_cb====OnResume.begin====');
    }
    _apt.OnPause = function(context){
        elog('====jso_activity_cb====OnPause.begin====');
    }
    _apt.OnStop = function(context){
        elog('====jso_activity_cb====OnStop.begin====');
    }
    _apt.OnDestroy = function(context){
        elog('====jso_activity_cb====OnDestroy.begin====');
    }
    _apt.OnActivityResult = function(context, aiRequestCode, aiResultCode, aoIntentData){
        elog('====jso_activity_cb====OnActivityResult.begin====');
    }
    _apt.OnCreateDialog = function(context, aiId, out_aoDialog){
        elog('====jso_activity_cb====OnCreateDialog.begin====');
    }   //OnCreateDialog
    _apt.OnCreateContextMenu = function(context, aoMenu, aoV, aoMenuInfo){
        elog('====jso_activity_cb====OnCreateContextMenu.begin====');
    }
    _apt.CreateNavigationBar = function(context){
        elog('====jso_activity_cb====CreateNavigationBar.begin====');
    }
    _apt.OnExchangeData = function(aiTimes ,aoInObject, aoOutObject) {
        elog('====jso_activity_cb====OnExchangeData.begin====');
    }
    _apt.OnHandleMessage = function(context, aoMessage){
        elog('====jso_activity_cb====OnHandleMessage.begin====');
    }

//--------Listener----default end----

    return new CActivityListener(oActivity);
};  //module.exports