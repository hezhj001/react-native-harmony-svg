/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SvgMask.h"
#include "drawing/Brush.h"
#include <native_drawing/drawing_color_filter.h>
#include <native_drawing/drawing_filter.h>
#include <native_drawing/drawing_canvas.h>
#include <native_drawing/drawing_color.h>

namespace rnoh {
namespace svg {

void SvgMask::OnDrawTraversedBefore(OH_Drawing_Canvas *canvas) {
    DLOG(INFO) << "[RNSVGMask] OnDrawTraversedBefore";
    // TODO implement proper support for units
    auto nodeBounds = AsBounds();
    DLOG(INFO) << "[RNSVGMask] Left0: " << nodeBounds.Left();
    DLOG(INFO) << "[RNSVGMask] Top0: " << nodeBounds.Top();
    DLOG(INFO) << "[RNSVGMask] Width0: " << nodeBounds.Width();
    DLOG(INFO) << "[RNSVGMask] Height0: " << nodeBounds.Height();
    auto left = static_cast<float>(nodeBounds.Left() + ParseUnitsAttr(maskAttribute_.x, nodeBounds.Width()));
    auto top = static_cast<float>(nodeBounds.Top() + ParseUnitsAttr(maskAttribute_.y, nodeBounds.Height()));
    auto width = static_cast<float>(ParseUnitsAttr(maskAttribute_.width, nodeBounds.Width()));
    auto height = static_cast<float>(ParseUnitsAttr(maskAttribute_.height, nodeBounds.Height()));
    drawing::Rect maskBounds(left, top, width + left, height + top);
    maskBounds_ = maskBounds;

    // create mask layer
    OH_Drawing_CanvasSaveLayer(canvas, maskBounds_.get(), nullptr);
    // ready to render mask content
    canvasLayerCount_ = OH_Drawing_CanvasGetSaveCount(canvas);

    // set mask
    auto outerFilter = OH_Drawing_ColorFilterCreateLuma();
    auto innerFilter = OH_Drawing_ColorFilterCreateSrgbGammaToLinear();
    auto colorFilter = OH_Drawing_ColorFilterCreateCompose(outerFilter, innerFilter);
    auto *filter = OH_Drawing_FilterCreate();
    OH_Drawing_FilterSetColorFilter(filter, colorFilter);

    drawing::Brush maskFilter;
    OH_Drawing_BrushSetFilter(maskFilter.get(), filter);
    OH_Drawing_CanvasSaveLayer(canvas, nullptr, maskFilter.get());
}

void SvgMask::OnDrawTraversedAfter(OH_Drawing_Canvas *canvas) {
    DLOG(INFO) << "[RNSVGMask] OnDrawTraversedAfter";
    OH_Drawing_CanvasRestoreToCount(canvas, canvasLayerCount_);
    // create content layer and render content
    drawing::Brush maskBrush;
    OH_Drawing_BrushSetBlendMode(maskBrush.get(), BLEND_MODE_SRC_IN);

    OH_Drawing_CanvasSaveLayer(canvas, maskBounds_.get(), maskBrush.get());
    OH_Drawing_CanvasClipRect(canvas, maskBounds_.get(), INTERSECT, true);
}


void SvgMask::OnInitStyle() { DLOG(INFO) << "[RNSVGMask] OnInitStyle"; }

double SvgMask::ParseUnitsAttr(const Dimension &attr, double value) {
    if (isDefaultMaskUnits_) {
        // only support decimal or percent
        if (attr.Unit() == DimensionUnit::PERCENT) {
            return value * attr.Value();
        }
        return attr.Value() * value;
    }
    // percent and px
    if (attr.Unit() == DimensionUnit::PERCENT) {
        return value * attr.Value();
    }
    return attr.Value();
}

} // namespace svg
} // namespace rnoh
