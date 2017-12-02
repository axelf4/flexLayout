/**
 * Routines for performing layout on flex containers with variable numbers of children.
 * @file
 */
#ifndef FLEX_LAYOUT_H
#define FLEX_LAYOUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

/** The value undefined. */
#define UNDEFINED NAN

/** Alignments. */
enum Align {
	/** The start margin edge of the item is flushed with the start edge of the line. */
	ALIGN_START,
	/** The end margin edge of the item is flushed with the end edge of the line. */
	ALIGN_END,
	/** The margin box of the item is centered. */
	ALIGN_CENTER,
	/** Space is allocated equally between the items. */
	ALIGN_SPACE_BETWEEN,
	/** Space is allocated equally around the items. */
	ALIGN_SPACE_AROUND,
	/** The item fills the line. */
	ALIGN_STRETCH
};

/** Layout requirements from the parent. */
enum MeasureMode {
	/** The parent has not imposed any constraint on the child. */
	MEASURE_UNSPECIFIED,
	/** The parent has determined an exact size for the child. */
	MEASURE_EXACTLY,
	/** The child can be as large as it wants up to the specified size. */
	MEASURE_AT_MOST
};

/**
 * Returns whether \c value is undefined.
 *
 * @param value The value to check.
 * @return Whether \c value is undefined.
 */
int isUndefined(const float value);

/** A context specifying an interface to the widgets. */
struct FlexContext {
	/**
	 * Sets the x-coordinate of the specified widget.
	 *
	 * @param widget The widget.
	 * @param x The new x-coordinate.
	 */
	void (*setX)(const void *widget, float x);
	/**
	 * Sets the x-coordinate of the specified widget.
	 *
	 * @param widget The widget.
	 * @param x The new x-coordinate.
	 */
	void (*setY)(const void *widget, float y);
	/**
	 * Returns the width set by #setWidth or a layout pass (whichever happened more recently) of the specified widget.
	 *
	 * @param widget The widget.
	 * @return The width of the widget.
	 */
	float (*getWidth)(const void *widget);
	/**
	 * Sets the width of the widget to the specified value.
	 *
	 * @param widget The widget.
	 * @param width The new width of the widget.
	 */
	void (*setWidth)(const void *widget, float width);
	/**
	 * Returns the height set by #setHeight or a layout pass (whichever happened more recently) of the specified widget.
	 *
	 * @param widget The widget.
	 * @return The height of the widget.
	 */
	float (*getHeight)(const void *widget);
	/**
	 * Sets the height of the widget to the specified value.
	 *
	 * @param widget The widget.
	 * @param height The new height of the widget.
	 */
	void (*setHeight)(const void *widget, float height);
	/**
	 * Lays out the specified widget.
	 *
	 * @param widget The widget to lay out.
	 * @param width The available width.
	 * @param widthMode The width requirement.
	 * @param height The available height.
	 * @param heightMode The height requirement.
	 */
	void (*layout)(const void *widget, float width, enum MeasureMode widthMode, float height, enum MeasureMode heightMode);
	/**
	 * Returns the number of children of the container.
	 *
	 * @param widget The container.
	 * @return The number of children.
	 */
	int (*getChildCount)(const void *widget);
	/**
	 * Returns the child of the container at the specified index.
	 *
	 * @param widget The container.
	 * @param index The zero-based index of the child.
	 * @return The child at the index.
	 */
	void *(*getChildAt)(const void *widget, int index);
	/**
	 * Returns the layout parameters of the specified widget.
	 *
	 * @param widget The widget.
	 * @return The layout parameters.
	 */
	void *(*getLayoutParams)(const void *widget);
};

/** Directions in which to place items. */
enum FlexDirection {
	/** The items are layed out horizontally. */
	DIRECTION_ROW,
	/** The items are layed out vertically. */
	DIRECTION_COLUMN
};

/** Options that control how each individual item is layed out. */
struct FlexParams {
	/** The alignment in the container's cross axis. */
	enum Align align;
	/**
	 * The flex value of the item.
	 *
	 * A value greater than \c 0 means that the item will receive a fraction of
	 * the available space in the main axis equal to the value divided by the
	 * sum of all positive flex values. A negative value means that the item
	 * will shrink in the event of an overflow in the main axis. A value of \c
	 * 0 makes the item non-flexible.
	 */
	float flex;
	/** The width of the item or #UNDEFINED. */
	float width;
	/** The height of the item or #UNDEFINED. */
	float height;
	/** The margin space required on the top of the item. */
	float marginTop;
	/** The margin space required on the right of the item. */
	float marginRight;
	/** The margin space required on the bottom of the item. */
	float marginBottom;
	/** The margin space required on the left of the item. */
	float marginLeft;
};

/**
 * Lays out the specified flex container.
 *
 * @param context The context to use.
 * @param widget The flex container.
 * @param width The available width.
 * @param widthMode The width requirement.
 * @param height The available height.
 * @param heightMode The height requirement.
 * @param direction The direction the items are placed in.
 * @param justify The alignment of the content.
 */
void layoutFlex(const struct FlexContext *context, void *widget, float width, enum MeasureMode widthMode, float height, enum MeasureMode heightMode, enum FlexDirection direction, enum Align justify);

#ifdef __cplusplus
}
#endif

#endif
