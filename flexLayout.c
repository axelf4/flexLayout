#include "flexLayout.h"

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

int isUndefined(float value) {
	return isnan(value);
}

static enum FlexDirection getPerpendicularAxis(enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? DIRECTION_COLUMN : DIRECTION_ROW;
}

static float getLeadingMargin(struct FlexParams *params, enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? params->marginLeft : params->marginTop;
}

static float getTrailingMargin(struct FlexParams *params, enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? params->marginRight : params->marginBottom;
}

static float getMargin(struct FlexParams *params, enum FlexDirection axis) {
	return getLeadingMargin(params, axis) + getTrailingMargin(params, axis);
}

static float getStyleSize(struct FlexParams *params, enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? params->width : params->height;
}

static float getLayoutSize(const struct FlexContext *context, void *widget, enum FlexDirection axis) {
	return axis == DIRECTION_ROW ? context->getWidth(widget) : context->getHeight(widget);
}

static int isFlexBasisAuto(struct FlexParams *params) {
	return params->flex <= 0;
}

static float getFlexGrowFactor(struct FlexParams *params) {
	float flex = params->flex;
	if (flex > 0) return flex;
	return 0;
}

static int getFlexShrinkFactor(struct FlexParams *params) {
	return params->flex < 0;
}

void layoutFlex(const struct FlexContext *context, void *widget, float width, enum MeasureMode widthMode, float height, enum MeasureMode heightMode, enum FlexDirection direction, enum Align justify) {
	enum FlexDirection mainAxis = direction, crossAxis = getPerpendicularAxis(mainAxis);
	int childCount = context->getChildCount(widget);
	enum MeasureMode mainMeasureMode = mainAxis == DIRECTION_ROW ? widthMode : heightMode,
					 crossMeasureMode = crossAxis == DIRECTION_ROW ? widthMode : heightMode;
	float availableMain = mainAxis == DIRECTION_ROW ? width : height,
		  availableCross = crossAxis == DIRECTION_ROW ? width : height;

	// Determine basis for each child
	float sizeConsumed = 0, // Dimensions of the content in the main axis
		  totalFlexGrowFactors = 0, totalFlexShrinkScaledFactors = 0;
	for (int i = 0; i < childCount; ++i) {
		void *child = context->getChildAt(widget, i);
		struct FlexParams *params = context->getLayoutParams(child);
		float styleSize = getStyleSize(params, mainAxis), basis;

		if (!isUndefined(styleSize)) {
			basis = styleSize;
		} else if (!isFlexBasisAuto(params) && availableMain) {
			basis = 0;
		} else {
			// Determine the base size by performing layout
			float childWidth, childHeight;
			enum MeasureMode childWidthMode, childHeightMode;

			if (!isUndefined(getStyleSize(params, DIRECTION_ROW))) {
				childWidth = params->width;
				childWidthMode = MEASURE_EXACTLY;
			} else if (crossAxis == DIRECTION_ROW && widthMode == MEASURE_EXACTLY && params->align == ALIGN_STRETCH) {
				childWidth = width;
				childWidthMode = MEASURE_EXACTLY;
			} else {
				childWidth = width;
				childWidthMode = widthMode == MEASURE_UNSPECIFIED ? MEASURE_UNSPECIFIED : MEASURE_AT_MOST;
			}

			if (!isUndefined(getStyleSize(params, DIRECTION_COLUMN))) {
				childHeight = params->height;
				childHeightMode = MEASURE_EXACTLY;
			} else if (crossAxis == DIRECTION_COLUMN && heightMode == MEASURE_EXACTLY && params->align == ALIGN_STRETCH) {
				childHeight = height;
				childHeightMode = MEASURE_EXACTLY;
			} else {
				childHeight = height;
				childHeightMode = heightMode == MEASURE_UNSPECIFIED ? MEASURE_UNSPECIFIED : MEASURE_AT_MOST;
			}

			context->layout(child, childWidth, childWidthMode, childHeight, childHeightMode);
			basis = getLayoutSize(context, child, mainAxis);
		}

		context->setWidth(child, basis); // Store the basis in the child's width dimension
		sizeConsumed += basis + getMargin(params, mainAxis);
		totalFlexGrowFactors += getFlexGrowFactor(params);
		totalFlexShrinkScaledFactors += getFlexShrinkFactor(params) * basis;
	}

	// Layout flexible children and allocate empty space
	float remainingSpace = availableMain ? availableMain - sizeConsumed : 0; // The remaining available space in the main axis
	float leadingMainSize = 0, betweenMain = 0;
	if (totalFlexGrowFactors == 0 && remainingSpace > 0 && mainMeasureMode == MEASURE_EXACTLY) {
		// Allocate remaining space according to justify.
		switch (justify) {
			default:
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
		void *child = context->getChildAt(widget, i);
		struct FlexParams *params = context->getLayoutParams(child);
		float childCrossStyleSize = getStyleSize(params, crossAxis);
		float childBasis = context->getWidth(child);

		if (remainingSpace < 0) {
			float flexShrinkScaledFactor = getFlexShrinkFactor(params) * childBasis;
			if (flexShrinkScaledFactor != 0) childBasis += remainingSpace / totalFlexShrinkScaledFactors * flexShrinkScaledFactor;
		} else if (remainingSpace > 0) {
			float flexGrowFactor = getFlexGrowFactor(params);
			if (flexGrowFactor != 0) childBasis += remainingSpace / totalFlexGrowFactors * flexGrowFactor;
		}

		float childCrossSize = isUndefined(childCrossStyleSize) ? availableCross : childCrossStyleSize;
		enum MeasureMode childCrossMode = !isUndefined(childCrossStyleSize) || (crossMeasureMode == MEASURE_EXACTLY && params->align == ALIGN_STRETCH)
			? MEASURE_EXACTLY : crossMeasureMode == MEASURE_UNSPECIFIED ? MEASURE_UNSPECIFIED : MEASURE_AT_MOST;
		if (mainAxis == DIRECTION_ROW) context->layout(child, childBasis, MEASURE_EXACTLY, childCrossSize, childCrossMode);
		else context->layout(child, childCrossSize, childCrossMode, childBasis, MEASURE_EXACTLY);

		// Position element in the main axis
		(mainAxis == DIRECTION_ROW ? context->setX : context->setY)(child, mainSize + getLeadingMargin(params, mainAxis));
		mainSize += betweenMain + getLayoutSize(context, child, mainAxis) + getMargin(params, mainAxis);
		crossSize = MAX(crossSize, getLayoutSize(context, child, crossAxis) + getMargin(params, crossAxis));
	}

	// If the dimensions are definite: set them
	if (mainMeasureMode == MEASURE_EXACTLY) mainSize = availableMain;
	if (crossMeasureMode == MEASURE_EXACTLY) crossSize = availableCross;

	// Position elements in the cross axis
	for (int i = 0; i < childCount; ++i) {
		void *child = context->getChildAt(widget, i);
		struct FlexParams *params = context->getLayoutParams(child);
		int leadingCrossDim = 0;
		switch (params->align) {
			case ALIGN_STRETCH:
				// Layout the child if the cross size wasn't already definite
				if (!getStyleSize(params, crossAxis)) {
					float childWidth = context->getWidth(child), childHeight = context->getHeight(child);
					*(crossAxis == DIRECTION_ROW ? &childWidth : &childHeight) = crossSize - getMargin(params, crossAxis);
					context->layout(child, childWidth, MEASURE_EXACTLY, childHeight, MEASURE_EXACTLY);
				}
				break;
			case ALIGN_CENTER:
			case ALIGN_END:
				leadingCrossDim = (crossSize - getLayoutSize(context, child, crossAxis) - getMargin(params, crossAxis)) / (params->align == ALIGN_CENTER ? 2 : 1);
				break;
			default:
				break;
		}
		(crossAxis == DIRECTION_ROW ? context->setX : context->setY)(child, leadingCrossDim + getLeadingMargin(params, crossAxis));
	}

	// Set the implicit width and height
	context->setWidth(widget, mainAxis == DIRECTION_ROW ? mainSize : crossSize);
	context->setHeight(widget, mainAxis == DIRECTION_ROW ? crossSize : mainSize);
}
