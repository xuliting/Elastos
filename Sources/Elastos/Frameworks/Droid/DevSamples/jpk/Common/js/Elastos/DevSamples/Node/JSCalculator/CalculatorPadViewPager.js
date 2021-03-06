elog("====CalculatorPadViewPager.js====begin====");

module.exports = function(aoElastos, aoActivity){
//--------common definition----begin----
    var CObject = aoElastos.CObject;

    var CString = aoElastos.Core.CString;
    var Droid_New = aoElastos.Droid.New;
    var Core_New = aoElastos.Core.New;

    var R = aoElastos.Application.R;

    var oActivity = aoActivity.ActivityInstance;
    var oHandler = aoActivity.ActivityHandler;

    var IView__VISIBLE = 0x00000000;

//--------common definition----end----

//--------.java----begin----

// /*
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

// import android.content.Context;
// import android.support.v4.view.PagerAdapter;
    var PagerAdapter = require("./support/v4/view/PagerAdapter.js")(aoElastos, aoActivity);
// import android.support.v4.view.ViewPager;
    var ViewPager = require("./support/v4/view/ViewPager.js")(aoElastos, aoActivity);
    //var ViewPager = class _ViewPager {};

// import android.util.AttributeSet;
// import android.view.View;
// import android.view.ViewGroup;

// public class CalculatorPadViewPager extends ViewPager {
class _CalculatorPadViewPager extends ViewPager {
    constructor() {
        super();
    }

//     private final PagerAdapter mStaticPagerAdapter = new PagerAdapter() {
//         @Override
//         public int getCount() {
//             return getChildCount();
//         }

//         @Override
//         public Object instantiateItem(ViewGroup container, int position) {
//             return getChildAt(position);
//         }

//         @Override
//         public void destroyItem(ViewGroup container, int position, Object object) {
//             removeViewAt(position);
//         }

//         @Override
//         public boolean isViewFromObject(View view, Object object) {
//             return view == object;
//         }

//         @Override
//         public float getPageWidth(int position) {
//             return position == 1 ? 7.0f / 9.0f : 1.0f;
//         }
//     }
    get mStaticPagerAdapter() {
        if (this._mStaticPagerAdapter) return this._mStaticPagerAdapter;

        var _this = this._obj;

        class _StaticPagerAdapter extends PagerAdapter {
            constructor() {
                super();
            }

            // @Override
            GetCount() {
                return _this.GetChildCount();
            }

            // @Override
            InstantiateItem(container, position) {
                return _this.GetChildAt(position);
            }

            // @Override
            DestroyItem(container, position, object) {
                _this.RemoveViewAt(position);
            }

            // @Override
            IsViewFromObject(view, object) {
                //return (view == object);
                //TODO:common method JSWrapper.equal/has/commonInterface/valid/getClass
                var n1 = view.getClass().GetName();
                var n2 = object.getClass().GetName();
                elog("====CalculatorPadViewPager::mStaticPagerAdapter::IsViewFromObject====n1:"+n1+"====n2:"+n2+"====equal:"+(n1==n2));
                return n1==n2;
            }

            // @Override
            GetPageWidth(position, result) {
                return (position == 1) ? 7.0 / 9.0 : 1.0;
            }

        }   //class _StaticPagerAdapter

        this._mStaticPagerAdapter = new _StaticPagerAdapter();

        return this._mStaticPagerAdapter;
    }   //get mStaticPagerAdapter

//     private final OnPageChangeListener mOnPageChangeListener = new SimpleOnPageChangeListener() {
//         private void recursivelySetEnabled(View view, boolean enabled) {
//             if (view instanceof ViewGroup) {
//                 final ViewGroup viewGroup = (ViewGroup) view;
//                 for (int childIndex = 0; childIndex < viewGroup.getChildCount(); ++childIndex) {
//                     recursivelySetEnabled(viewGroup.getChildAt(childIndex), enabled);
//                 }
//             } else {
//                 view.setEnabled(enabled);
//             }
//         }

//         @Override
//         public void onPageSelected(int position) {
//             if (getAdapter() == mStaticPagerAdapter) {
//                 for (int childIndex = 0; childIndex < getChildCount(); ++childIndex) {
//                     // Only enable subviews of the current page.
//                     recursivelySetEnabled(getChildAt(childIndex), childIndex == position);
//                 }
//             }
//         }
//     }
    get mOnPageChangeListener() {
        if (this._mOnPageChangeListener_) return this._mOnPageChangeListener;
        this._mOnPageChangeListener_ = true;
        this._mOnPageChangeListener = new class _tmp extends ViewPager.SimpleOnPageChangeListener {
            RecursivelySetEnabled(view, enabled) {
                if (view instanceof ViewGroup) {
                    var viewGroup = view;
                    for (var childIndex = 0; childIndex < viewGroup.GetChildCount(); ++childIndex) {
                        recursivelySetEnabled(viewGroup.GetChildAt(childIndex), enabled);
                    }
                } else {
                    view.setEnabled(enabled);
                }
            }

            // @Override
            OnPageSelected(position) {
                if (getAdapter() == mStaticPagerAdapter) {
                    for (var childIndex = 0; childIndex < getChildCount(); ++childIndex) {
                        // Only enable subviews of the current page.
                        recursivelySetEnabled(getChildAt(childIndex), childIndex == position);
                    }
                }
            }
        }();
        return this._mOnPageChangeListener;
    }   //get mOnPageChangeListener
    set mOnPageChangeListener(v) {this._mOnPageChangeListener = v;}

//     private final PageTransformer mPageTransformer = new PageTransformer() {
//         @Override
//         public void transformPage(View view, float position) {
//             if (position < 0.0f) {
//                 // Pin the left page to the left side.
//                 view.setTranslationX(getWidth() * -position);
//                 view.setAlpha(Math.max(1.0f + position, 0.0f));
//             } else {
//                 // Use the default slide transition when moving to the next page.
//                 view.setTranslationX(0.0f);
//                 view.setAlpha(1.0f);
//             }
//         }
//     }
    get mPageTransformer() {
        if (this._mPageTransformer_) return this._mPageTransformerr;
        this._mPageTransformer_ = true;
        this._mPageTransformer = new class _tmp extends this.PageTransformer {
            // @Override
            TransformPage(view, position) {
                if (position < 0.0) {
                    // Pin the left page to the left side.
                    view.SetTranslationX(getWidth() * -position);
                    view.SetAlpha(Math.max(1.0 + position, 0.0));
                } else {
                    // Use the default slide transition when moving to the next page.
                    view.SetTranslationX(0.0);
                    view.SetAlpha(1.0);
                }
            }
        }();
        return this._mOnPageChangeListener;
    }   //get mPageTransformer
    set mPageTransformer(v) {this._mPageTransformer = v;}

//     public CalculatorPadViewPager(Context context) {
//         this(context, null);
//     }

//     public CalculatorPadViewPager(Context context, AttributeSet attrs) {
//         super(context, attrs);

//         setAdapter(mStaticPagerAdapter);
//         setBackgroundColor(getResources().getColor(android.R.color.black));
//         setOnPageChangeListener(mOnPageChangeListener);
//         setPageMargin(getResources().getDimensionPixelSize(R.dimen.pad_page_margin));
//         setPageTransformer(false, mPageTransformer);
//     }
    OnCreate(_this, context, attrs) {
        elog("====CalculatorPadViewPager====OnCreate====begin====");

        attrs = attrs || null;

        this._obj = _this;

        super.OnCreate(_this, context, attrs);

        this.SetAdapter(this.mStaticPagerAdapter);
        var Elastos__Droid__R__color__black = 0x0106000c;
        _this.SetBackgroundColor(_this.GetResources().GetColor(Elastos__Droid__R__color__black));
        this.SetOnPageChangeListener(this.mOnPageChangeListener);
        this.SetPageMargin(_this.GetResources().GetDimensionPixelSize(R.dimen.pad_page_margin));
        this.SetPageTransformer(false, this.mPageTransformer);

        elog("====CalculatorPadViewPager====OnCreate====end====");
    }

//     @Override
//     protected void onFinishInflate() {
//         super.onFinishInflate();

//         // Invalidate the adapter's data set since children may have been added during inflation.
//         if (getAdapter() == mStaticPagerAdapter) {
//             mStaticPagerAdapter.notifyDataSetChanged();
//         }
//     }
    OnFinishInflate(_this) {
        elog("====OnFinishInflate====begin====");

        _this._OnFinishInflate();

        if (this.GetAdapter() == this.mStaticPagerAdapter) {
            this.mStaticPagerAdapter.NotifyDataSetChanged();
        }
        elog("====OnFinishInflate====end====");
    }

}   //class _CalculatorPadViewPager

//--------.java----end----

class CalculatorPadViewPager extends _CalculatorPadViewPager {
    constructor() {
        super();
    }

    //--------
    // OnCreate(_this, context, attrs) {
    //     Assert(0);

    //     elog("====CalculatorPadViewPager.js====::OnCreate====begin");
    //     _this._constructor(context, attrs);
    //     elog("====CalculatorPadViewPager.js====::OnCreate====end");
    // }

    //--------
    // GenerateLayoutParams(_this, attrs, result){
    //     Assert(0);

    //     elog("====CalculatorPadViewPager.js====::GenerateLayoutParams====begin");
    //     result.data = _this._GenerateLayoutParams(attrs);
    //     elog("====CalculatorPadViewPager.js====::GenerateLayoutParams====end");
    // }

    GetChildDrawingOrder(_this, childCount, i, result) {
        Assert(0);

        elog("====CalculatorPadViewPager.js====::GetChildDrawingOrder====begin");
        result.data = _this._GetChildDrawingOrder(childCount, i);
        elog("====CalculatorPadViewPager.js====::GetChildDrawingOrder====end");
    }

    //--------
    // AddView(_this, child, index, params) {
    //     Assert(0);

    //     elog("====CalculatorPadViewPager.js====::AddView====begin");
    //     _this._AddView(child, index, params);
    //     elog("====CalculatorPadViewPager.js====::AddView====end");
    // }

    //--------
    // CheckLayoutParams(_this, p, result) {
    //     //Assert(0);

    //     elog("====CalculatorPadViewPager.js====::CheckLayoutParams====begin");
    //     result.data = _this._CheckLayoutParams(p);
    //     elog("====CalculatorPadViewPager.js====::CheckLayoutParams====end==result:" + result.data);
    // }

    // GenerateDefaultLayoutParams(_this, result) {
    //     Assert(0);

    //     elog("====CalculatorPadViewPager.js====::GenerateDefaultLayoutParams====begin");
    //     result.data = _this._GenerateDefaultLayoutParams();
    //     elog("====CalculatorPadViewPager.js====::GenerateDefaultLayoutParams====end");
    // }

    // OnFinishInflate(_this) {
    //     Assert(0);

    //     elog("====CalculatorPadViewPager.js====::OnFinishInflate====begin");
    //     _this._OnFinishInflate();
    //     elog("====CalculatorPadViewPager.js====::OnFinishInflate====end");
    // }

    // OnAttachedToWindow(_this) {
    //     Assert(0);

    //     elog("====CalculatorPadViewPager.js====::OnAttachedToWindow====begin");
    //     _this._OnAttachedToWindow();
    //     elog("====CalculatorPadViewPager.js====::OnAttachedToWindow====end");
    // }

    // DrawableStateChanged(_this) {
    //     Assert(0);

    //     elog("====CalculatorPadViewPager.js====::DrawableStateChanged====begin");
    //     _this._DrawableStateChanged();
    //     elog("====CalculatorPadViewPager.js====::DrawableStateChanged====end");
    // }

    // OnMeasure(_this, widthMeasureSpec, heightMeasureSpec) {
    //     Assert(0);

    //     elog('====CalculatorPadViewPager.js====::OnMeasure.begin====');
    //     _this._OnMeasure(widthMeasureSpec, heightMeasureSpec);
    //     elog('====CalculatorPadViewPager.js====::OnMeasure.end====');
    // }

    // OnSizeChanged(_this, w, h, oldw, oldh) {
    //     Assert(0);

    //     elog('====CalculatorPadViewPager.js====::OnSizeChanged.begin====');
    //     _this._OnSizeChanged(w, h, oldw, oldh);
    //     elog('====CalculatorPadViewPager.js====::OnSizeChanged.end====');
    // }

    // OnLayout(_this, changed, left, top, right, bottom) {
    //     Assert(0);

    //     elog('====CalculatorPadViewPager.js====::OnLayout.begin====');
    //     elog('====CalculatorPadViewPager.js====::OnLayout.end====');
    // }

    // ComputeScroll(_this) {
    //     Assert(0);

    //     elog('====CalculatorPadViewPager.js====::ComputeScroll.begin====');
    //     _this._ComputeScroll();
    //     elog('====CalculatorPadViewPager.js====::ComputeScroll.end====');
    // }

}   //class CalculatorPadViewPager

return new CalculatorPadViewPager();

};  //module.exports

elog("====CalculatorPadViewPager.js====end====");