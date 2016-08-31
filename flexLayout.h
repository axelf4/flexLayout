/**
 * Routines for performing layout on flex containers with variable numbers of children.
 * @file
 */
#ifndef RMGUI_H
#define RMGUI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <math.h>

/** The value undefined. */
#define UNDEFINED NAN

/** Alignments. */
typedef enum Align {
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
} Align;

/** Layout requirements from the parent. */
typedef enum MeasureMode {
	/** The parent has not imposed any constraint on the child. */
	MEASURE_UNSPECIFIED,
	/** The parent has determined an exact size for the child. */
	MEASURE_EXACTLY,
	/** The child can be as large as it wants up to the specified size. */
	MEASURE_AT_MOST
} MeasureMode;

/**
 * Returns whether \c value is undefined.
 *
 * @param value the value to check.
 * @return whether \c value is undefined.
 */
int isUndefined(const float value);

/** A layout context that specifies how to query and update properties on widgets. */
typedef struct LayoutContext {
	// int (*getX)(void *widget);
	/**
	 * Sets the x-coordinate of the specified widget.
	 *
	 * @param widget the widget.
	 * @param x the new x-coordinate.
	 */
	void (*setX)(const void *widget, float x);
	// int (*getY)(void *widget);
	/**
	 * Sets the x-coordinate of the specified widget.
	 *
	 * @param widget the widget.
	 * @param x the new x-coordinate.
	 */
	void (*setY)(const void *widget, float y);
	/**
	 * Returns the width set by #setWidth or a layout pass (whichever happened more recently) of the specified widget.
	 *
	 * @param widget the widget.
	 * @return the width of the widget.
	 */
	float (*getWidth)(const void *widget);
	/**
	 * Sets the width of the widget to the specified value.
	 *
	 * @param widget the widget.
	 * @param width the new width of the widget.
	 */
	void (*setWidth)(const void *widget, float width);
	/**
	 * Returns the height set by #setHeight or a layout pass (whichever happened more recently) of the specified widget.
	 *
	 * @param widget the widget.
	 * @return the height of the widget.
	 */
	float (*getHeight)(const void *widget);
	/**
	 * Sets the height of the widget to the specified value.
	 *
	 * @param widget the widget.
	 * @param height the new height of the widget.
	 */
	void (*setHeight)(const void *widget, float height);
	/**
	 * Lays out the specified widget.
	 *
	 * @param widget the widget to lay out.
	 * @param width the available width.
	 * @param widthMode the width requirement.
	 * @param height the available height.
	 * @param heightMode the height requirement.
	 */
	void (*layout)(const void *widget, float width, MeasureMode widthMode, float height, MeasureMode heightMode);
	/**
	 * Returns the number of children of the container.
	 *
	 * @param widget the container.
	 * @return the number of children.
	 */
	int (*getChildCount)(const void *widget);
	/**
	 * Returns the child of the container at the specified index.
	 *
	 * @param widget the container.
	 * @param index the index.
	 * @return the child at the index.
	 */
	void *(*getChildAt)(const void *widget, int index);
	/**
	 * Returns the layout parameters of the specified widget.
	 *
	 * @param widget the widget.
	 * @return the layout parameters.
	 */
	void *(*getLayoutParams)(const void *widget);
} LayoutContext;

/** Directions in which ::layoutFlex places items. */
typedef enum FlexDirection {
	/** The items are layed out horizontally. */
	DIRECTION_ROW,
	/** The items are layed out vertically. */
	DIRECTION_COLUMN
} FlexDirection;

/** Options that control how each individual item is layed out. */
typedef struct FlexParams {
	/** The alignment in the container's cross axis. */
	Align align;
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
	/** The margin space required on the left of the item. */
	float marginLeft;
	/** The margin space required on the top of the item. */
	float marginTop;
	/** The margin space required on the right of the item. */
	float marginRight;
	/** The margin space required on the bottom of the item. */
	float marginBottom;
} FlexParams;

/**
 * Lays out the specified flex container.
 * <p>
 * LayoutContext#getLayoutParams when passed a child must return a valid
 * pointer to FlexParams.
 *
 * @param layoutContext the used context.
 * @param widget the flex container.
 * @param width the available width.
 * @param widthMode the width requirement.
 * @param height the available height.
 * @param heightMode the height requirement.
 * @param direction the direction the items are placed in.
 * @param justify the alignment of the content.
 */
void layoutFlex(const struct LayoutContext *layoutContext, const void *widget, float width, enum MeasureMode widthMode, float height, enum MeasureMode heightMode, const FlexDirection direction, const Align justify);

#ifdef __cplusplus
}
#endif

#endif
