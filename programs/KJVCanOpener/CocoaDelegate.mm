/****************************************************************************
**
** Copyright (C) 2025 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

#include <Cocoa/Cocoa.h>
#include <ApplicationServices/ApplicationServices.h>

// See "~/Qt/6.5.3-src/qtbase/src/plugins/platforms/cocoa/qcocoaapplicationdelegate.mm"
// See "~/Qt/6.5.3-src/qtbase/src/plugins/platforms/cocoa/qcocoaintegration.mm"


@interface KJPBSCocoaAppDelegate : NSObject <NSApplicationDelegate>
+ (instancetype)sharedDelegate;
- (void)setReflectionDelegate:(NSObject<NSApplicationDelegate> *)oldDelegate;
- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app;
@end

@implementation KJPBSCocoaAppDelegate {
	NSObject <NSApplicationDelegate> *reflectionDelegate;
}

+ (instancetype)sharedDelegate
{
	static KJPBSCocoaAppDelegate *shared = nil;
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		shared = [[self alloc] init];
		atexit_b(^{
			[shared release];
			shared = nil;
		});
	});
	return shared;
}

- (instancetype)init
{
	self = [super init];
	return self;
}

- (void)dealloc
{
	if (reflectionDelegate) {
		[[NSApplication sharedApplication] setDelegate:reflectionDelegate];
		[reflectionDelegate release];
	}
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	[super dealloc];
}

- (void)setReflectionDelegate:(NSObject <NSApplicationDelegate> *)oldDelegate
{
	[oldDelegate retain];
	[reflectionDelegate release];
	reflectionDelegate = oldDelegate;
}

- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app
{
	return YES;
}
@end

void setCocoaAppDelegate()
{
	NSApplication *cocoaApplication = [NSApplication sharedApplication];

	// Set app delegate, link to the current delegate (if any)
	KJPBSCocoaAppDelegate *newDelegate = [KJPBSCocoaAppDelegate sharedDelegate];
	[newDelegate setReflectionDelegate:[cocoaApplication delegate]];
	[cocoaApplication setDelegate:newDelegate];
}

void releaseCocoaAppDelegate()
{
	// reset the application delegate
	[[NSApplication sharedApplication] setDelegate:nil];
}

// ============================================================================

/*

// The following would be all that's needed to make this work, but because the
//	Cocoa Platform is a plugin on Qt instead of being in the main code, it causes
//	a linker error since the QCocoaApplicationDelegate isn't part of the main code.
//	So, instead of just adding this one function to the existing delegate, we create
//	our own delegate (above) and load it in main() before creating the Application,
//	which should allow the QCocoaApplicationDelegate to piggyback on top of ours.

@interface QCocoaApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@interface QCocoaApplicationDelegate (MyExtensionCategory)
- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app;
@end

@implementation QCocoaApplicationDelegate (MyExtensionCategory)
- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app
{
	return YES;
}
@end

*/

