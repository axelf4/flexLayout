#include "rmgui.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int isUndefined(const float value) {
	return isnan(value);
}

static enum FlexDirection getPerpendicularAxis(const enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? DIRECTION_COLUMN : DIRECTION_ROW;
}

static float getLeadingMargin(const struct FlexParams *params, const enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? params->marginLeft : params->marginTop;
}

static float getTrailingMargin(const struct FlexParams *params, const enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? params->marginRight : params->marginBottom;
}

static float getMargin(const struct FlexParams *params, const enum FlexDirection axis) {
	return getLeadingMargin(params, axis) + getTrailingMargin(params, axis);
}

static float getStyleSize(const struct FlexParams *params, const enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? params->width : params->height;
}

static float getLayoutSize(const struct LayoutContext *context, const void *widget, enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? context->getWidth(widget) : context->getHeight(widget);
}

static Align getAlign(const struct FlexParams *params) {
	return params->align;
}

static float getFlex(const struct FlexParams *params) {
	return params->flex;
}

static int isFlex(const struct FlexParams *params) {
	return getFlex(params) != 0;
}

static int isFlexBasisAuto(const struct FlexParams *params) {
	return getFlex(params) <= 0;
}

static float getFlexGrowFactor(const struct FlexParams *params) {
	float flex = getFlex(params);
	if (flex > 0) return flex;
	return 0;
}

static int getFlexShrinkFactor(const struct FlexParams *params) {
	if (getFlex(params) < 0) return 1;
	return 0;
}

void layoutFlex(const struct LayoutContext *layoutContext, const void *widget, float width, MeasureMode widthMode, float height, MeasureMode heightMode, const FlexDirection direction, const Align justify) {
	const enum FlexDirection mainAxis = direction, crossAxis = getPerpendicularAxis(mainAxis);
	const int childCount = layoutContext->getChildCount(widget);
	const MeasureMode mainMeasureMode = mainAxis == DIRECTION_ROW ? widthMode : heightMode,
		  crossMeasureMode = crossAxis == DIRECTION_ROW ? widthMode : heightMode;
	const float availableMain = mainAxis == DIRECTION_ROW ? width : height,
		  availableCross = crossAxis == DIRECTION_ROW ? width : height;

	// Determine basis for each child
	float sizeConsumed = 0; // Dimensions of the content in the main axis
	float totalFlexGrowFactors = 0, totalFlexShrinkScaledFactors = 0;
	for (int i = 0; i < childCount; ++i) {
		const void *child = layoutContext->getChildAt(widget, i);
		const struct FlexParams *params = layoutContext->getLayoutParams(child);
		const float styleSize = getStyleSize(params, mainAxis);
		float basis;

		if (!isUndefined(styleSize)) {
			basis = styleSize;
		} else if (!isFlexBasisAuto(params) && availableMain) {
			basis = 0;
		} else {
			// Determine the base size by performing layout
			const Align align = getAlign(params);
			float childWidth, childHeight;
			MeasureMode childWidthMode, childHeightMode;

			if (!isUndefined(getStyleSize(params, DIRECTION_ROW))) {
				childWidth = params->width;
				childWidthMode = MEASURE_EXACTLY;
			} else if (crossAxis == DIRECTION_ROW && widthMode == MEASURE_EXACTLY && align == ALIGN_STRETCH) {
				childWidth = width;
				childWidthMode = MEASURE_EXACTLY;
			} else {
				childWidth = width;
				childWidthMode = widthMode == MEASURE_UNSPECIFIED ? MEASURE_UNSPECIFIED : MEASURE_AT_MOST;
			}

			if (!isUndefined(getStyleSize(params, DIRECTION_COLUMN))) {
				childHeight = params->height;
				childHeightMode = MEASURE_EXACTLY;
			} else if (crossAxis == DIRECTION_COLUMN && heightMode == MEASURE_EXACTLY && align == ALIGN_STRETCH) {
				childHeight = height;
				childHeightMode = MEASURE_EXACTLY;
			} else {
				childHeight = height;
				childHeightMode = heightMode == MEASURE_UNSPECIFIED ? MEASURE_UNSPECIFIED : MEASURE_AT_MOST;
			}

			layoutContext->layout(child, childWidth, childWidthMode, childHeight, childHeightMode);
			basis = getLayoutSize(layoutContext, child, mainAxis);
		}

		// Store the basis in the child's dimension on the main axis
		(mainAxis == DIRECTION_ROW ? layoutContext->setWidth : layoutContext->setHeight)(child, basis);
		sizeConsumed += basis + getMargin(params, mainAxis);
		if (isFlex(child)) {
			totalFlexGrowFactors += getFlexGrowFactor(params);
			totalFlexShrinkScaledFactors += getFlexShrinkFactor(params) * basis;
		}
	}

	// Layout flexible children and allocate empty space
	float leadingMainSize = 0, betweenMain = 0;
	const float remainingSpace = availableMain ? availableMain - sizeConsumed : 0; // The remaining available space in the main axis
	for (int i = 0; i < childCount; ++i) {
		const void *child = layoutContext->getChildAt(widget, i);
		const struct FlexParams *params = layoutContext->getLayoutParams(child);
		const float childCrossStyleSize = getStyleSize(params, crossAxis);
		float childBasis = getLayoutSize(layoutContext, child, mainAxis);

		if (remainingSpace < 0) {
			float flexShrinkScaledFactor = getFlexShrinkFactor(params) * childBasis;
			if (flexShrinkScaledFactor != 0) childBasis += remainingSpace / totalFlexShrinkScaledFactors * flexShrinkScaledFactor;
		} else if (remainingSpace > 0) {
			float flexGrowFactor = getFlexGrowFactor(params);
			if (flexGrowFactor != 0) childBasis += remainingSpace / totalFlexGrowFactors * flexGrowFactor;
		}

		float childWidth, childHeight;
		MeasureMode childWidthMode, childHeightMode;
		if (mainAxis == DIRECTION_ROW) {
			childWidth = childBasis;
			childWidthMode = MEASURE_EXACTLY;

			if (!isUndefined(childCrossStyleSize)) {
				childHeight = childCrossStyleSize;
				childHeightMode = MEASURE_EXACTLY;
			} else {
				childHeight = height;
				if (heightMode == MEASURE_EXACTLY && getAlign(params) == ALIGN_STRETCH) {
					childHeightMode = MEASURE_EXACTLY;
				} else {
					childHeightMode = heightMode == MEASURE_UNSPECIFIED ? MEASURE_UNSPECIFIED : MEASURE_AT_MOST;
				}
			}
		} else {
			childHeight = childBasis;
			childHeightMode = MEASURE_EXACTLY;

			if (childCrossStyleSize) {
				childWidth = childCrossStyleSize;
				childWidthMode = MEASURE_EXACTLY;
			} else {
				childWidth = width;
				if (widthMode == MEASURE_EXACTLY && getAlign(params) == ALIGN_STRETCH) {
					childWidthMode = MEASURE_EXACTLY;
				} else {
					childWidthMode = widthMode == MEASURE_UNSPECIFIED ? MEASURE_UNSPECIFIED : MEASURE_AT_MOST;
				}
			}
		}
		layoutContext->layout(child, childWidth, childWidthMode, childHeight, childHeightMode);
	}
	if (totalFlexGrowFactors == 0 && remainingSpace > 0 && mainMeasureMode == MEASURE_EXACTLY) {
		// Allocate remaining space according to justify.
		switch (justify) {
			case ALIGN_START:
				break;
			case ALIGN_CENTER:
				leadingMainSize = remainingSpace / 2;
				break;
			case ALIGN_END:
				leadingMainSize = remainingSpace;
				break;
			case ALIGN_SPACE_BETWEEN:
				if (childCount > 1) betweenMain = remainingSpace / (childCount - 1);
				break;
			case ALIGN_SPACE_AROUND:
				leadingMainSize = (betweenMain = remainingSpace / childCount) / 2;
				break;
		}
	}

	int mainSize = leadingMainSize, crossSize = 0;
	for (int i = 0; i < childCount; ++i) {
		const void *child = layoutContext->getChildAt(widget, i);
		const struct FlexParams *params = layoutContext->getLayoutParams(child);
		(mainAxis == DIRECTION_ROW ? layoutContext->setX : layoutContext->setY)(child, mainSize + getLeadingMargin(params, crossAxis));
		mainSize += betweenMain + getLayoutSize(layoutContext, child, mainAxis) + getMargin(params, mainAxis);
		crossSize = MAX(crossSize, getLayoutSize(layoutContext, child, crossAxis) + getMargin(params, mainAxis));
	}

	// If the dimensions are definite: use them
	if (mainMeasureMode == MEASURE_EXACTLY) mainSize = availableMain;
	if (crossMeasureMode == MEASURE_EXACTLY) crossSize = availableCross;

	// Position elements in the cross axis
	for (int i = 0; i < childCount; ++i) {
		const void *child = layoutContext->getChildAt(widget, i);
		const struct FlexParams *params = layoutContext->getLayoutParams(child);
		const Align align = getAlign(params);
		int leadingCrossDim = 0;
		if (align == ALIGN_STRETCH) {
			int isCrossSizeDefinite = 0;
			float childWidth = layoutContext->getWidth(child), childHeight = layoutContext->getHeight(child);
			if (mainAxis == DIRECTION_ROW) {
				isCrossSizeDefinite = getStyleSize(params, DIRECTION_COLUMN);
				childHeight = crossSize - getMargin(params, DIRECTION_COLUMN);
			} else {
				isCrossSizeDefinite = getStyleSize(params, DIRECTION_ROW);
				childWidth = crossSize - getMargin(params, DIRECTION_ROW);
			}
			// If the cross size of the child was not already definite
			if (!isCrossSizeDefinite) {
				layoutContext->layout(child, childWidth, MEASURE_EXACTLY, childHeight, MEASURE_EXACTLY);
			}
		} else if (align != ALIGN_START) {
			float remainingCross = crossSize - getLayoutSize(layoutContext, child, crossAxis) + getMargin(params, crossAxis);
			if (align == ALIGN_CENTER) leadingCrossDim += remainingCross / 2;
			else if (align == ALIGN_END) leadingCrossDim += remainingCross;
		}
		(crossAxis == DIRECTION_ROW ? layoutContext->setX : layoutContext->setY)(child, leadingCrossDim + getLeadingMargin(params, crossAxis));
	}

	// Set the implicit width and height
	layoutContext->setWidth(widget, mainAxis == DIRECTION_ROW ? mainSize : crossSize);
	layoutContext->setHeight(widget, mainAxis == DIRECTION_ROW ? crossSize : mainSize);
}
